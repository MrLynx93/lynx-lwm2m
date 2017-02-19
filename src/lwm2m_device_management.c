#include "lwm2m.h"
#include "lwm2m_device_management.h"
#include "lwm2m_access_control.h"
#include "lwm2m_parser.h"

#define PAD_LEN 30
#define STR_LEN 30

#define EXECUTE_RESPONSE {CONTENT_TYPE_NO_FORMAT, RESPONSE_CODE_CHANGED, 1, NULL, 0};


static void __free_value(lwm2m_value *value, lwm2m_type type) {
    if (type == STRING || type == OPAQUE) {
        free(value->string_value);
    }
    free(value);
}

static void __free_parsed_resource(lwm2m_resource *resource) {
    if (resource->multiple) {
        if (resource->instances != NULL) {
            for (list_elem *elem = resource->instances->first; elem != NULL; elem = elem->next) {
                lwm2m_resource *resource_instance = elem->value;
                if (resource_instance->value != NULL) {
                    __free_value(resource_instance->value, resource->type);
                }
                free(resource_instance);
            }
            list_free(resource->instances);
        }
    }
    else if (resource->value != NULL) {
        __free_value(resource->value, resource->type);
    }
    free(resource);
}

static void __free_parsed_resources(list *resources) {
    for (list_elem *elem = resources->first; elem != NULL; elem = elem->next) {
        __free_parsed_resource(elem->value);
    }
    list_free(resources);
}

static void __free_instances_to_parse(list *instances) {
    for (list_elem *elem = instances->first; elem != NULL; elem = elem->next) {
        lwm2m_instance *instance = elem->value;
        list_free(instance->resources);
        free(instance);
    }
    list_free(instances);
}




/***
     * [1]
     * If a new Object Instance is created through that operation and
     * the LWM2M Client has more than one LWM2M Server Account, then
     * the LWM2M Client creates an Access Control Object Instance for
     * the created Object Instance
     */

execute_param *param_new() {
    execute_param *param = (execute_param*) malloc(sizeof(execute_param));
    param->key = -1;
    param->string_value = NULL;
    return param;
}

int on_resource_write(lwm2m_server *server, lwm2m_resource *resource, char *message, int message_len) {
    sem_wait(&server->context->object_tree_lock);

    if (!lwm2m_check_instance_access_control(server, resource->instance, WRITE)) {
        sem_post(&server->context->object_tree_lock);
        return RESPONSE_CODE_UNAUTHORIZED;
    }
    if (!lwm2m_check_resource_operation_supported(resource, WRITE)) {
        sem_post(&server->context->object_tree_lock);
        return RESPONSE_CODE_METHOD_NOT_ALLOWED;
    }

    lwm2m_resource *parsed_resource = parse_resource( resource->instance->object, resource->id, message, message_len);

    /**** Copy value and call WRITE callback *****/
    list *servers_to_notify = merge_resource(resource, parsed_resource, true, true); // todo notify here and free list
    notify_instance_object(server->context, resource->instance, servers_to_notify);
    list_free(servers_to_notify);
    __free_parsed_resource(parsed_resource);

    sem_post(&server->context->object_tree_lock);
    return RESPONSE_CODE_CHANGED;
}

int on_instance_write(lwm2m_server *server, lwm2m_instance *instance, char *message, int message_len) {
    sem_wait(&server->context->object_tree_lock);

    if (!lwm2m_check_instance_access_control(server, instance, WRITE)) {
        return RESPONSE_CODE_UNAUTHORIZED;
    }
    list *parsed_resources = parse_instance(instance->object, message, message_len);
    lwm2m_instance parsed_instance = {.resources = parsed_resources};

    /**** Error if any of resources is not writeable ****/
    for (list_elem *elem = parsed_resources->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *resource = elem->value;
        if (!lwm2m_check_resource_operation_supported(resource, WRITE)) {
            __free_parsed_resources(parsed_resources);
            return RESPONSE_CODE_METHOD_NOT_ALLOWED;
        };
    }

    /**** Copy all resources and call WRITE callback ****/
    merge_resources(instance, &parsed_instance, true, true);
    __free_parsed_resources(parsed_resources);

    sem_post(&server->context->object_tree_lock);
    return RESPONSE_CODE_CHANGED;
}

lwm2m_response on_resource_read(lwm2m_server *server, lwm2m_resource *resource) {
    sem_wait(&server->context->object_tree_lock);

    lwm2m_response response = {
            .payload = (char *) malloc(sizeof(char) * 5000),
    };

    if (!lwm2m_check_instance_access_control(server, resource->instance, READ)) {
        response.content_type = CONTENT_TYPE_NO_FORMAT;
        response.response_code = RESPONSE_CODE_UNAUTHORIZED;
        return response;
    }
    if (!lwm2m_check_resource_operation_supported(resource, READ)) {
        response.content_type = CONTENT_TYPE_NO_FORMAT;
        response.response_code = RESPONSE_CODE_METHOD_NOT_ALLOWED;
        return response;
    }

    /** Call read callback to update value of resource **/
    if (resource->read_callback != NULL) {
        resource->read_callback(resource);
    }

    /** Serialize resource into response **/
    if (resource->multiple) {
        serialize_multiple_resource(resource->instances, response.payload, &response.payload_len);
        response.content_type = CONTENT_TYPE_TLV;
    } else {
        serialize_resource_text(resource, response.payload, &response.payload_len);
        response.content_type = resource->type == OPAQUE ? CONTENT_TYPE_OPAQUE : CONTENT_TYPE_TEXT;
    }
    response.response_code = RESPONSE_CODE_CHANGED;

    sem_post(&server->context->object_tree_lock);
    return response;
}

lwm2m_response on_instance_read(lwm2m_server *server, lwm2m_instance *instance) {
    sem_wait(&server->context->object_tree_lock);

    lwm2m_response response = {
            .payload = (char *) malloc(sizeof(char) * 5000),
    };

    if (!lwm2m_check_instance_access_control(server, instance, READ)) {
        response.content_type = CONTENT_TYPE_NO_FORMAT;
        response.response_code = RESPONSE_CODE_UNAUTHORIZED;
        return response;
    }

    list *resources_to_parse = list_new();

    /**** Serialize only resources that are readable****/
    for (list_elem *elem = instance->resources->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *resource = elem->value;
        if (lwm2m_check_resource_operation_supported(resource, READ)) {
            /** Call read callback to update value of resource **/
            if (resource->read_callback != NULL) {
                resource->read_callback(resource);
            }
            /** Serialize only resources that are not NULL **/
            if (resource->value != NULL) {
                ladd(resources_to_parse, resource->id, resource);
            }
        }
    }
    serialize_instance(resources_to_parse, response.payload, &response.payload_len);
    list_free(resources_to_parse);

    response.content_type = CONTENT_TYPE_TLV;
    response.response_code = RESPONSE_CODE_CHANGED;
    sem_post(&server->context->object_tree_lock);
    return response;
}

lwm2m_response on_object_read(lwm2m_server *server, lwm2m_object *object) {
    sem_wait(&server->context->object_tree_lock);

    lwm2m_response response = {
            .payload = (char *) malloc(sizeof(char) * 5000),
            .content_type = CONTENT_TYPE_TLV,
            .response_code = RESPONSE_CODE_CHANGED,
    };

    list *instances_to_parse = list_new();
    for (list_elem *elem_i = object->instances->first; elem_i != NULL; elem_i = elem_i->next) {
        lwm2m_instance *instance = elem_i->value;

        /**** Serialize only instances, that have granted READ access ****/
        if (lwm2m_check_instance_access_control(server, instance, READ)) {
            lwm2m_instance *instance_to_parse = (lwm2m_instance *) malloc(sizeof(lwm2m_instance));
            instance_to_parse->resources = list_new();
            instance_to_parse->id = instance->id;

            for (list_elem *elem_r = instance->resources->first; elem_r != NULL; elem_r = elem_r->next) {
                lwm2m_resource *resource = elem_r->value;

                /**** Serialize only resources that are readable ****/
                if (lwm2m_check_resource_operation_supported(resource, READ)) {
                    /** Call read callback to update value of resource **/
                    if (resource->read_callback != NULL) {
                        resource->read_callback(resource);
                    }
                    /** Serialize only resources that are not NULL **/
                    if (resource->value != NULL) {
                        ladd(instance_to_parse->resources, resource->id, resource);
                    }
                }
            }
            /** Serialize only instances, that have resources **/
            if (instance_to_parse->resources->size > 0) {
                ladd(instances_to_parse, instance_to_parse->id, instance_to_parse);
            }
        }
    }
    serialize_object(instances_to_parse, response.payload, &response.payload_len);

    /**** Free parsed instances ****/
    __free_instances_to_parse(instances_to_parse);

    sem_post(&server->context->object_tree_lock);
    return response;
}

#define CREATE_RESPONSE {CONTENT_TYPE_NO_FORMAT, RESPONSE_CODE_CREATED, 1, NULL, 0}

lwm2m_response on_instance_create(lwm2m_server *server, lwm2m_object *object, int *instance_id, char *message, int message_len) {
    sem_wait(&server->context->object_tree_lock);

    lwm2m_context *context = server->context;
    lwm2m_response response = CREATE_RESPONSE;

    /***** Check access in ACO Instance assigned to LWM2M Object *****/
    if (!check_create_object_access_control(server, object)) {
        response.response_code = RESPONSE_CODE_UNAUTHORIZED;
        return response;
    }

    /***** If one instance is allowed, then instance's ID=0 *****/
    if (object->multiple == false) {
        instance_id = 0;
    }

    /***** Error if ID already exists *****/
    lwm2m_instance *old_instance = lfind(object->instances, *instance_id);
    if (old_instance != NULL) {
        response.response_code = RESPONSE_CODE_METHOD_NOT_ALLOWED;
        return response;
    }

    /***** Create instance and add to object *******/
    lwm2m_instance *instance = *instance_id == -1
                               ? lwm2m_instance_new(object)
                               : lwm2m_instance_new_with_id(object, *instance_id);

    *instance_id = instance->id;
    list *all_parsed_resources = parse_instance(object, message, message_len);
    lwm2m_instance parsed_instance = {
            .resources = list_new()
    };

    /**** Filter parsed resources to merge to instance ****/
    for (list_elem *elem = instance->resources->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *template_resource = elem->value;
        lwm2m_resource *parsed_resource = lfind(all_parsed_resources, elem->key);

        /***** Error id not all mandatory resources are provided  ******/
        if (template_resource->mandatory && parsed_resource == NULL) {
            response.response_code = RESPONSE_CODE_METHOD_NOT_ALLOWED; // TODO check
            return response; // TODO free payload?
        }

        /***** DONT Ignore read-only resource - its create op. *******/
        if (parsed_resource != NULL && template_resource->type != NONE) {
            ladd(parsed_instance.resources, parsed_resource->id, parsed_resource);
        }
        // TODO Can resource be mandatory and read-only?
    }


    /**** [1] Create ACO for new instance *****/
    if (context->servers->size > 1) {
        lwm2m_instance_create_aco_instance(server, instance);
    }

    /**** Possibly object-level notify ****/
    // TODO check if it works and in documentation if it should work
    merge_resources(instance, &parsed_instance, true, true);
    __free_parsed_resources(all_parsed_resources);
    list_free(parsed_instance.resources);

    sem_post(&server->context->object_tree_lock);
    return response;
}

lwm2m_response on_instance_delete(lwm2m_server *server, lwm2m_instance *instance) {
    sem_wait(&server->context->object_tree_lock);

    lwm2m_response response = {
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload = NULL,
            .payload_len = 0
    };

    /**** Check access for DELETE operation ****/
    if (!lwm2m_check_instance_access_control(server, instance, DELETE)) {
        response.response_code = RESPONSE_CODE_UNAUTHORIZED;
        sem_post(&server->context->object_tree_lock);
        return response;
    }

    lwm2m_delete_instance(instance);
    response.response_code = RESPONSE_CODE_DELETED;
    sem_post(&server->context->object_tree_lock);
    return response;
}

lwm2m_response on_resource_execute(lwm2m_server *server, lwm2m_resource *resource, list *args) {
    lwm2m_response response = EXECUTE_RESPONSE;

    /**** Check access for EXECUTE operation ****/
    if (!lwm2m_check_instance_access_control(server, resource->instance, EXECUTE)) {
        response.response_code = RESPONSE_CODE_UNAUTHORIZED;
        return response;
    }

    /**** Check if resource is executable ****/
    if (!lwm2m_check_resource_operation_supported(resource, EXECUTE)) {
        response.response_code = RESPONSE_CODE_METHOD_NOT_ALLOWED;
        return response;
    }

    resource->execute_callback(resource, args);
    return response;
}

static void __subst(char *s, char from, char to) {
    while (*s == from)
        *s++ = to;
}

void DUMP_MULTIPLE_RESOURCE(lwm2m_resource *resource) {
    char buffer[100], header[20], value[100];
    int value_len;

    sprintf(header, "/%d/%d/%d", resource->instance->object->id, resource->instance->id, resource->id);
    sprintf(buffer, "%0*d", (int) (PAD_LEN + STR_LEN + 2 - strlen(header)), 0);
    __subst(buffer, '0', '.');
    printf("%s%s\n", header, buffer);

    for (list_elem *elem = resource->instances->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *instance = elem->value;
        serialize_resource_text(instance, value, &value_len);

        sprintf(header, "/%d/%d/%d/%d", resource->instance->object->id, resource->instance->id, resource->id, instance->id);
        sprintf(buffer, "%0*d", (int) (PAD_LEN - strlen(header)), 0);
        __subst(buffer, '0', '.');

        if (resource->type == OPAQUE) {
            printf("%s%s[", header, buffer);
            if (value_len == 0) {
                printf("-------------%s-------------", resource->type == NONE ? "EXEC" : "NULL");
            } else {
                for (int j = 0; j < 10; ++j) {
                    if (j < instance->length) {
                        printf("x%02X", value[j]);
                    } else {
                        printf("   ");
                    }
                }
            }
            printf("]");
        } else {
            printf("%s%s[%-30s]", header, buffer, value);
        }
        printf(".%c.....%s\n", resource->mandatory ? 'M' : '.', resource->name);
    }
}

void DUMP_SINGLE_RESOURCE(lwm2m_resource *resource) {
    char buffer[100], header[20], value[100];
    int value_len;

    serialize_resource_text(resource, value, &value_len);
    if (value_len == 0) {
        strcpy(value, resource->type == NONE ? "-------------EXEC-------------" : "-------------NULL-------------");
    }
    sprintf(header, "/%d/%d/%d", resource->instance->object->id, resource->instance->id, resource->id);

    sprintf(buffer, "%0*d", (int) (PAD_LEN - strlen(header)), 0);
    __subst(buffer, '0', '.');
    if (resource->type == OPAQUE) {
        printf("%s%s[", header, buffer);
        if (value_len == 0) {
            printf("-------------%s-------------", resource->type == NONE ? "EXEC" : "NULL");
        } else {
            for (int i = 0; i < 10; ++i) {
                if (i < value_len) {
                    printf("x%02X", value[i]);
                } else {
                    printf("   ");
                }
            }
        }

        printf("]");
    } else {
        printf("%s%s[%-30s]", header, buffer, value);
    }
    printf(".%c.....%s\n", resource->mandatory ? 'M' : '.', resource->name);
}

void DUMP_INSTANCE(lwm2m_instance *instance) {
    char header[20], buffer[100];
    sprintf(header, "/%d/%d", instance->object->id, instance->id);
    sprintf(buffer, "%0*d", (int) (PAD_LEN - strlen(header) + STR_LEN + 2), 0);
    __subst(buffer, '0', '.');
    printf("%s%s\n", header, buffer);

    for (list_elem *elem = instance->resources->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *resource = elem->value;
        if (resource->multiple) {
            DUMP_MULTIPLE_RESOURCE(resource);
        } else {
            DUMP_SINGLE_RESOURCE(resource);
        }
    }
}

void DUMP_OBJECT(lwm2m_object *object) {
    char header[20], buffer[100];
    sprintf(header, "/%d", object->id);
    sprintf(buffer, "%0*d", (int) (PAD_LEN - strlen(header) + STR_LEN + 2), 0);
    __subst(buffer, '0', '.');
    printf("%s%s\n", header, buffer);

    for (list_elem *elem = object->instances->first; elem != NULL; elem = elem->next) {
        lwm2m_instance *instance = elem->value;
        DUMP_INSTANCE(instance);
    }
}

void DUMP_ALL(lwm2m_context *context) {
    for (list_elem *elem = context->object_tree->first; elem != NULL; elem = elem->next) {
        lwm2m_object *object = elem->value;
        DUMP_OBJECT(object);
    }
}