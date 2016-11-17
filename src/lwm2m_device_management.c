#include "lwm2m.h"
#include "lwm2m_device_management.h"
#include "lwm2m_access_control.h"
#include "lwm2m_parser.h"

#define PAD_LEN 30
#define STR_LEN 30

/***
     * [1]
     * If a new Object Instance is created through that operation and
     * the LWM2M Client has more than one LWM2M Server Account, then
     * the LWM2M Client creates an Access Control Object Instance for
     * the created Object Instance
     */

static lwm2m_map* __create_resources(lwm2m_context *context, int object_id) {
    if (object_id == SECURITY_OBJECT_ID || object_id == SERVER_OBJECT_ID || object_id == ACCESS_CONTROL_OBJECT_ID) {
        return context->create_standard_resources_callback(object_id);
    } else {
        return context->create_resources_callback(object_id);
    }
}

int on_resource_write(lwm2m_server *server, lwm2m_resource *resource, char *message, int message_len) {
    if (!lwm2m_check_instance_access_control(server, resource->instance)) {
        return RESPONSE_CODE_UNAUTHORIZED;
    }
    if (!lwm2m_check_resource_operation_supported(resource, READ)) {
        return RESPONSE_CODE_METHOD_NOT_ALLOWED;
    }

    lwm2m_resource *parsed_resource = parse_resource(
            server->context,
            resource->instance->object->id,
            resource->id,
            message,
            message_len
    );

    /**** Copy value and call WRITE callback *****/
    merge_resource(resource, parsed_resource, true);

    return RESPONSE_CODE_CHANGED;
}

int on_instance_write(lwm2m_server *server, lwm2m_instance *instance, char *message, int message_len) {
    if (!lwm2m_check_instance_access_control(server, instance)) {
        return RESPONSE_CODE_UNAUTHORIZED;
    }
    lwm2m_map *parsed_resources = parse_instance(server->context, instance->object->id, message, message_len);

    /**** Error if any of resources is not writeable ****/
    int keys[parsed_resources->size];
    lwm2m_map_get_keys(parsed_resources, keys);
    for (int i = 0; i < parsed_resources->size; ++i) {
        lwm2m_resource *resource = lwm2m_map_get_resource(parsed_resources, keys[i]);
        if (!lwm2m_check_resource_operation_supported(resource, WRITE)) {
            return RESPONSE_CODE_METHOD_NOT_ALLOWED;
        };
    }

    /**** Copy all resources and call WRITE callback ****/
    merge_resources(instance->resources, parsed_resources, true);

    return RESPONSE_CODE_CHANGED;
}

lwm2m_response on_resource_read(lwm2m_server *server, lwm2m_resource *resource) {
    lwm2m_response response = {
            .payload = (char *) malloc(sizeof(char) * 100),
    };

    if (!lwm2m_check_instance_access_control(server, resource->instance)) {
        response.content_type = CONTENT_TYPE_NO_FORMAT;
        response.response_code = RESPONSE_CODE_UNAUTHORIZED;
        return response;
    }
    if (!lwm2m_check_resource_operation_supported(resource, READ)) {
        response.content_type = CONTENT_TYPE_NO_FORMAT;
        response.response_code = RESPONSE_CODE_METHOD_NOT_ALLOWED;
        return response;
    }

    if (resource->multiple) {
        serialize_multiple_resource(resource->instances, response.payload, &response.payload_len);
        response.content_type = CONTENT_TYPE_TLV;
    } else {
        serialize_resource_text(resource, response.payload, &response.payload_len);
        response.content_type = resource->type == OPAQUE ? CONTENT_TYPE_OPAQUE : CONTENT_TYPE_TEXT;
    }
    response.response_code = RESPONSE_CODE_CHANGED;
    return response;
}

lwm2m_response on_instance_read(lwm2m_server *server, lwm2m_instance *instance) {
    lwm2m_response response = {
            .payload = (char *) malloc(sizeof(char) * 100),
    };

    if (!lwm2m_check_instance_access_control(server, instance)) {
        response.content_type = CONTENT_TYPE_NO_FORMAT;
        response.response_code = RESPONSE_CODE_UNAUTHORIZED;
        return response;
    }

    lwm2m_map *resources_to_parse = lwm2m_map_new();

    /**** Parse only resources that are readable ****/
    int keys[instance->resources->size];
    lwm2m_map_get_keys(instance->resources, keys);
    for (int i = 0; i < instance->resources->size; ++i) {
        lwm2m_resource *resource = lwm2m_map_get_resource(instance->resources, keys[i]);
        if (lwm2m_check_resource_operation_supported(resource, READ)) {
            lwm2m_map_put(resources_to_parse, resource->id, resource);
        }
    }
    serialize_instance(resources_to_parse, response.payload, &response.payload_len);

    response.content_type = CONTENT_TYPE_TLV;
    response.response_code = RESPONSE_CODE_CHANGED;
    return response;
}

lwm2m_response on_object_read(lwm2m_server *server, lwm2m_object *object) {
    lwm2m_response response = {
            .payload = (char *) malloc(sizeof(char) * 1000),
            .content_type = CONTENT_TYPE_TLV,
            .response_code = RESPONSE_CODE_CHANGED,
    };
    lwm2m_map *instances_to_parse = lwm2m_map_new();

    int keys[object->instances->size];
    lwm2m_map_get_keys(object->instances, keys);
    for (int i = 0; i < object->instances->size; ++i) {
        lwm2m_instance *instance = lwm2m_map_get_instance(object->instances, keys[i]);

        /**** Parse only instances, that have granted READ access ****/
        if (lwm2m_check_instance_access_control(server, instance)) {
            lwm2m_instance *instance_to_parse = (lwm2m_instance *) malloc(sizeof(lwm2m_instance));
            instance_to_parse->resources = lwm2m_map_new();
            instance_to_parse->id = instance->id;

            int res_keys[instance->resources->size];
            lwm2m_map_get_keys(instance->resources, res_keys);
            for (int j = 0; j < instance->resources->size; ++j) {
                lwm2m_resource *resource = lwm2m_map_get_resource(instance->resources, res_keys[j]);

                /**** Parse only resources that are readable ****/
                if (lwm2m_check_resource_operation_supported(resource, READ)) {
                    lwm2m_map_put(instance_to_parse->resources, resource->id, resource);
                }
            }
            lwm2m_map_put(instances_to_parse, instance_to_parse->id, instance_to_parse);
        }
    }
    serialize_object(instances_to_parse, response.payload, &response.payload_len);

    /**** Free parsed instances ****/
    int keys2[instances_to_parse->size];
    lwm2m_map_get_keys(instances_to_parse, keys2);
    for (int i = 0; i < instances_to_parse->size; ++i) {
        lwm2m_instance *parsed_instance = lwm2m_map_get_instance(instances_to_parse, keys2[i]);
        lwm2m_map_free(parsed_instance->resources);
        free(parsed_instance);
    }
    free(instances_to_parse);

    return response;
}

lwm2m_response on_instance_create(lwm2m_server *server, lwm2m_object *object, int instance_id, char *message, int message_len) {
    lwm2m_context *context = server->context;
    lwm2m_response response = {
            .payload = NULL,
            .payload_len = 0
    };

    /***** Check access in ACO Instance assigned to LWM2M Object *****/
    if (!check_create_object_access_control(server, object)) {
        response.content_type = CONTENT_TYPE_NO_FORMAT;
        response.response_code = RESPONSE_CODE_UNAUTHORIZED;
        return response;
    }

    /***** If one instance is allowed, then instance's ID=0 *****/
    if (object->multiple == false) {
        instance_id = 0;
    }

    /***** Error if ID already exists *****/
    lwm2m_instance *old_instance = lwm2m_map_get_instance(object->instances, instance_id);
    if (old_instance != NULL) {
        response.content_type = CONTENT_TYPE_NO_FORMAT;
        response.response_code = RESPONSE_CODE_METHOD_NOT_ALLOWED;
        return response;
    }

    lwm2m_map *resources = lwm2m_map_new();
    lwm2m_map *template_resources = __create_resources(context, object->id);
    lwm2m_map *parsed_resources = parse_instance(context, object->id, message, message_len);

    int keys[template_resources->size];
    lwm2m_map_get_keys(template_resources, keys);
    for (int i = 0; i < template_resources->size; ++i) {
        lwm2m_resource *template_resource = lwm2m_map_get_resource(template_resources, keys[i]);
        lwm2m_resource *parsed_resource = lwm2m_map_get_resource(parsed_resources, keys[i]);

        /***** Error id not all mandatory resources are provided  ******/
        if (template_resource->mandatory && parsed_resource == NULL) {
            response.content_type = CONTENT_TYPE_NO_FORMAT;
            response.response_code = RESPONSE_CODE_METHOD_NOT_ALLOWED; // TODO check
            return response; // TODO free payload?
        }

        /***** Ignore read-only resource *******/
        if (parsed_resource != NULL && template_resource->operations & WRITE) {
            lwm2m_map_put(resources, parsed_resource->id, parsed_resource);
        }

        // TODO Can resource be mandatory and read-only?
    }

    /***** Create instance and add to object *******/
    lwm2m_instance *instance = instance_id == -1
                               ? lwm2m_instance_new(context, object->id)
                               : lwm2m_instance_new_with_id(context, object->id, instance_id);
    instance->object = object;
    lwm2m_map_put(object->instances, instance->id, instance);

    /**** [1] Create ACO for new instance *****/
    if (context->servers->size > 1) {
        lwm2m_instance_create_aco_instance(server, instance);
    }
    merge_resources(instance->resources, resources, true);

    response.content_type = CONTENT_TYPE_NO_FORMAT;
    response.response_code = RESPONSE_CODE_CREATED;
    return response;
}




















void subst(char *s, char from, char to) {
    while (*s == from)
        *s++ = to;
}

void DUMP_MULTIPLE_RESOURCE(lwm2m_resource *resource) {
    char buffer[100], header[20], value[100];
    int value_len;

    sprintf(header, "/%d/%d/%d", resource->instance->object->id, resource->instance->id, resource->id);
    sprintf(buffer, "%0*d", (int) (PAD_LEN + STR_LEN + 2 - strlen(header)), 0);
    subst(buffer, '0', '.');
    printf("%s%s\n", header, buffer);

    lwm2m_map *instances = resource->instances;
    int keys[instances->size];
    lwm2m_map_get_keys(instances, keys);
    
    for (int i = 0; i < instances->size; ++i) {
        lwm2m_resource *instance = lwm2m_map_get_resource(instances, keys[i]);
        serialize_resource_text(instance, value, &value_len);

        sprintf(header, "/%d/%d/%d/%d", resource->instance->object->id, resource->instance->id, resource->id, instance->id);
        sprintf(buffer, "%0*d", (int) (PAD_LEN - strlen(header)), 0);
        subst(buffer, '0', '.');

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
    subst(buffer, '0', '.');
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
    subst(buffer, '0', '.');
    printf("%s%s\n", header, buffer);

    int keys[instance->resources->size];
    lwm2m_map_get_keys(instance->resources, keys);
    for (int i = 0; i < instance->resources->size; ++i) {
        lwm2m_resource *resource = lwm2m_map_get_resource(instance->resources, keys[i]);
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
    subst(buffer, '0', '.');
    printf("%s%s\n", header, buffer);

    int keys[object->instances->size];
    lwm2m_map_get_keys(object->instances, keys);
    for (int i = 0; i < object->instances->size; ++i) {
        lwm2m_instance *instance = lwm2m_map_get_instance(object->instances, keys[i]);
        DUMP_INSTANCE(instance);
    }
}

void DUMP_ALL(lwm2m_context *context) {
    int keys[context->object_tree->size];
    lwm2m_map_get_keys(context->object_tree, keys);

    for (int i = 0; i < context->object_tree->size; ++i) {
        lwm2m_object *object = lwm2m_map_get_object(context->object_tree, keys[i]);
        DUMP_OBJECT(object);
    }
}

//static int check_object_access(lwm2m_server *server, lwm2m_object *object) {
//    if (!lwm2m_check_object_access_control(server, object)) {
//        return ACCESS_RIGHT_PERMISSION_DENIED;
//    }
//    return 0;
//}
//
//static int check_instance_access(lwm2m_server *server, lwm2m_instance *instance, int operation) {
//    if (!lwm2m_check_instance_access_control(server, instance, READ)) {
//        return ACCESS_RIGHT_PERMISSION_DENIED;
//    }
//    return 0;
//}
//

//
//static int on_lwm2m_node_write_attributes(lwm2m_server *server, lwm2m_node *node, lwm2m_type type, char *message) {
//    lwm2m_map *parsed_attributes = deserialize_lwm2m_attributes(message);
//    lwm2m_map *attributes = get_node_attributes(node, type);
//
//    char **attribute_names = (char **) malloc(sizeof(char) * 10 * parsed_attributes->size);
//    lwm2m_map_get_keys_string(parsed_attributes, attribute_names);
//    for (int i = 0; i < parsed_attributes->size; i++) {
//        lwm2m_attribute *parsed_attribute = (lwm2m_attribute *) lwm2m_map_get_string(parsed_attributes,
//                                                                                     attribute_names[i]);
//        lwm2m_attribute *attribute = (lwm2m_attribute *) lwm2m_map_get_string(attributes, attribute_names[i]);
//
//        if (!is_notify_attribute(parsed_attribute->name)) {
//            return STRANGE_ERROR;
//        }
//        int error = lwm2m_check_attribute_access(parsed_attribute, READ);
//        if (error) {
//            return error;
//        }
//
//        if (attribute == NULL) {
//            lwm2m_map_put_string(attributes, attribute_names[i], parsed_attribute);
//        }
//        else {
//            attribute->numeric_value = parsed_attribute->numeric_value;
//        }
//    }
//    return 0;
//}
//
//static lwm2m_map *get_node_attributes(lwm2m_node *node, lwm2m_type type) {
//    if (type == OBJECT) {
//        return node->object.attributes;
//    }
//    if (type == INSTANCE) {
//        return node->instance.attributes;
//    }
//    if (type == RESOURCE) {
//        return node->resource.attributes;
//    }
//    return NULL;
//}
//
/////////////////////// READ /////////////////////////
//
//int on_lwm2m_object_read(lwm2m_server *server, lwm2m_object *object, char **message) {
//    serialize_lwm2m_object(server, object, message, NULL); // todo what here
//    return 0;
//}
//
//int on_lwm2m_instance_read(lwm2m_server *server, lwm2m_instance *instance, char **message) {
//    int access_error = check_instance_access(server, instance, READ);
//    if (access_error) {
//        return access_error;
//    }
//    serialize_lwm2m_instance(server, instance, message, NULL); // todo what here
//    return 0;
//}
//
//
//int on_lwm2m_resource_read(lwm2m_server *server, lwm2m_resource *resource, char **message) {
//    int access_error = check_resource_access(server, resource, READ);
//    if (access_error) {
//        return access_error;
//    }
//    resource->read_callback(resource);
//    serialize_lwm2m_resource(server, resource, message, NULL, TLV_FORMAT); // todo what here
//    return 0;
//}
//
/////////////////////// WRITE ////////////////////////
//// TODO also sending notification
//
//int on_lwm2m_instance_write(lwm2m_server *server, lwm2m_instance *instance, char *message) {
//    int access_error = check_instance_access(server, instance, WRITE);
//    if (access_error) {
//        return access_error;
//    }
//    deserialize_lwm2m_instance(instance, message, strlen(message));
//    return 0;
//}
//
//int on_lwm2m_resource_write(lwm2m_server *server, lwm2m_resource *resource, char *message) {
//    int access_error = check_resource_access(server, resource, WRITE);
//    if (access_error) {
//        return access_error;
//    }
//    deserialize_lwm2m_resource(resource, message, strlen(message), TEXT_FORMAT); // TODO what format ? based on multiple/single?
//    return 0;
//}
//
/////////////////////////// DELETE //////////////////////
//
//int on_lwm2m_instance_delete(lwm2m_server *server, lwm2m_instance *instance) {
//    int access_error = check_instance_access(server, instance, DELETE);
//    if (access_error) {
//        return access_error;
//    }
//    lwm2m_delete_instance(instance);
//}
//
/////////////////////////// CREATE //////////////////////
//
//// TODO move
//int get_next_id(lwm2m_object *object) {
//    return 5;
//}
//
//int on_lwm2m_instance_create(lwm2m_server *server, lwm2m_object *object, char *message, int *created_instance_id) {
//    // todo server-chosen id is in payload
//    int access_error = check_object_access(server, object); // todo create aco instance
//    if (access_error) {
//        return access_error;
//    }
//    lwm2m_instance *instance = lwm2m_instance_new(object);
//    deserialize_lwm2m_instance(instance, message, strlen(message));
//    instance->aco_instance = lwm2m_instance_create_aco_instance(server, instance);
//    instance->id = get_next_id(object); // TODO if message contains id - use it
//    *created_instance_id = instance->id;
//}
//
//
/////////////////////////// DISCOVER //////////////////////
//
//
//int on_lwm2m_object_discover(lwm2m_server *server, lwm2m_object *object, char **message) {
//    serialize_lwm2m_object_discover(object, message);
//    return 0;
//}
//
//int on_lwm2m_instance_discover(lwm2m_server *server, lwm2m_instance *instance, char **message) {
//    serialize_lwm2m_instance_discover(instance, message);
//    return 0;
//}
//
//int on_lwm2m_resource_discover(lwm2m_server *server, lwm2m_resource *resource, char **message) {
//    serialize_lwm2m_resource_discover(resource, message);
//    return 0;
//}
//
//
////////////////////// WRITE ATTRIBUTES ////////////////////
//
//
//int on_lwm2m_object_write_attributes(lwm2m_server *server, lwm2m_object *object, char *message) {
//    return on_lwm2m_node_write_attributes(server, (lwm2m_node*) object, OBJECT, message);
//}
//
//int on_lwm2m_instance_write_attributes(lwm2m_server *server, lwm2m_instance *instance, char *message) {
//    return on_lwm2m_node_write_attributes(server, (lwm2m_node*) instance, INSTANCE, message);
//}
//
//int on_lwm2m_resource_write_attributes(lwm2m_server *server, lwm2m_resource *resource, char *message) {
//    return on_lwm2m_node_write_attributes(server, (lwm2m_node*) resource, RESOURCE, message);
//}