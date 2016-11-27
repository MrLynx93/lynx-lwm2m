#include <lwm2m_transport.h>
#include <lwm2m_access_control.h>
#include "lwm2m.h"
#include "lwm2m_bootstrap.h"
#include "lwm2m_parser.h"

// TODO sdsssssssssssssssssssssssssssssssssssssssssssssssss
/****
 *
 * Only in Bootstrap Interface, Delete operation MAY target to “/” URI to delete all

the existing Object Instances - except LWM2M Bootstrap Server Account - in the LWM2M Client, for initialization purpose

before LWM2M Bootstrap Server sends Write operation(s) to the LWM2M Client. Different from „Write“ operation in

Device Management and Service Enablement interface, the LWM2M Client MUST write the value included in the payload

regardless of an existence of the targeting Object Instance(s) or Resource and access rights.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * @param context
 * @param object_id
 * @return
 */
static lwm2m_map* __create_resources(lwm2m_context *context, int object_id) {
    if (object_id == SECURITY_OBJECT_ID || object_id == SERVER_OBJECT_ID || object_id == ACCESS_CONTROL_OBJECT_ID) {
        return context->create_standard_resources_callback(object_id);
    } else {
        return context->create_resources_callback(object_id);
    }
}

static bool has_server_instances(lwm2m_context *context) {
    lwm2m_object *server_object = lwm2m_map_get_object(context->object_tree, SERVER_OBJECT_ID);
    return server_object->instances->size > 0;
}

// TODO move

static void free_value(lwm2m_value *value, lwm2m_type type) {
//    if (type == OPAQUE) {
//        free(value.opaque_value);
//    }
//    if (type == STRING) {
//        free(value.string_value);
//    }
}

//static void lwm2m_attribute_free(lwm2m_attribute *attribute) {
//    free_value(attribute->numeric_value, attribute->type);
//    free(attribute);
//}

//static void free_attributes(lwm2m_map *attributes) {
//    char **keys = (char **) malloc(sizeof(char *) * attributes->size);
//    lwm2m_map_get_keys_string(attributes, keys);
//
//    for (int i = 0; i < attributes->size; ++i) {
//        lwm2m_attribute *attribute = lwm2m_map_get_string(attributes, keys[i]);
//        lwm2m_attribute_free(attribute);
//    }
//    lwm2m_map_free(attributes);
//}

static void delete_resource(lwm2m_context *context, lwm2m_resource *resource) {
    if (!resource->multiple) {
        free_value(resource->value, resource->type);
    }
    if (resource->multiple) {
        lwm2m_map *instances = resource->instances;
        int keys[instances->size];

        for (int i = 0; i < instances->size; ++i) {
            free_value(lwm2m_map_get(instances, keys[i]), resource->type);
        }
    }
//    if (resource->attributes != NULL) {
//        free_attributes(resource->attributes);
//    }
}

static void delete_instance(lwm2m_context *context, lwm2m_instance *instance) {
    int keys[instance->resources->size];
    lwm2m_map_get_keys(instance->resources, keys);

    for (int i = 0; i < instance->resources->size; ++i) {
        lwm2m_resource *resource = lwm2m_map_get_resource(instance->resources, keys[i]);
        delete_resource(context, resource);
    }
//    if (instance->attributes != NULL) {
//        free_attributes(instance->attributes);
//    }
}

static void delete_object(lwm2m_context *context, lwm2m_object *object) {
    int keys[object->instances->size];
    lwm2m_map_get_keys(object->instances, keys);

    for (int i = 0; i < object->instances->size; ++i) {
        lwm2m_instance *instance = lwm2m_map_get_instance(object->instances, keys[i]);
        delete_instance(context, instance);
    }
}


static int __bootstrap_create_instance(lwm2m_context *context, lwm2m_object *object, lwm2m_map *parsed_resources, int instance_id) {
    lwm2m_map *resources = lwm2m_map_new();
    lwm2m_instance parsed_instance = {.resources = resources};
    lwm2m_map *template_resources = __create_resources(context, object->id);

    /***** If one instance is allowed, then instance's ID=0 *****/
    if (object->multiple == false) {
        instance_id = 0;
    }

    int keys[template_resources->size];
    lwm2m_map_get_keys(template_resources, keys);
    for (int i = 0; i < template_resources->size; ++i) {
        lwm2m_resource *template_resource = lwm2m_map_get_resource(template_resources, keys[i]);
        lwm2m_resource *parsed_resource = lwm2m_map_get_resource(parsed_resources, keys[i]);

        /***** Error if not all mandatory resources are provided  ******/
        if (template_resource->mandatory && template_resource->type != NONE && parsed_resource == NULL) {
            return RESPONSE_CODE_METHOD_NOT_ALLOWED;
        }

        /**** Don't check if resource is writeable ****/
        if (parsed_resource != NULL) {
            lwm2m_map_put(resources, parsed_resource->id, parsed_resource);
        }
    }
    lwm2m_instance *new_instance = lwm2m_instance_new_with_id(context, object->id, instance_id);
    new_instance->object = object;
    lwm2m_map_put(object->instances, new_instance->id, new_instance);

    merge_resources(new_instance, &parsed_instance, true, false);
    return RESPONSE_CODE_CREATED;
}

///////////// CALLBACKS ////////////////////

int on_bootstrap_object_write(lwm2m_context *context, lwm2m_object *object, char *message, int message_len) {
    lwm2m_map *parsed_instances = parse_object(context, object->id, message, message_len);

    int keys[parsed_instances->size];
    lwm2m_map_get_keys(parsed_instances, keys);
    for (int i = 0; i < parsed_instances->size; ++i) {
        lwm2m_instance *old_instance = lwm2m_map_get_instance(object->instances, keys[i]);
        lwm2m_instance *new_instance = lwm2m_map_get_instance(parsed_instances, keys[i]);

        if (old_instance == NULL) {
            __bootstrap_create_instance(context, object, new_instance->resources, new_instance->id);
        } else {
            merge_resources(old_instance, new_instance, true, false);
        }
    }
    return RESPONSE_CODE_CHANGED;
}

int on_bootstrap_instance_write(lwm2m_context *context, lwm2m_object *object, int instance_id, char *message, int message_len) {
    lwm2m_instance *old_instance = lwm2m_map_get_instance(object->instances, instance_id);
    lwm2m_map *parsed_resources = parse_instance(context, object->id, message, message_len);
    lwm2m_instance parsed_instance = {.resources = parsed_resources};

    if (old_instance == NULL) {
        return __bootstrap_create_instance(context, object, parsed_resources, instance_id);
    } else {
        merge_resources(old_instance, &parsed_instance, true, false);
    }
    return RESPONSE_CODE_CHANGED;
}

int on_bootstrap_resource_write(lwm2m_context *context, lwm2m_resource *resource, char *message, int message_len) {
    /**** Parse and copy values ****/
    lwm2m_resource *parsed_resource;
    if (resource->multiple) {
        parsed_resource = (lwm2m_resource *) malloc(sizeof(lwm2m_resource));
        parsed_resource->instances = parse_multiple_resource(
                context,
                resource->instance->object->id,
                resource->id,
                message,
                message_len
        );
    } else {
        parsed_resource = parse_resource(
                context,
                resource->instance->object->id,
                resource->id,
                message,
                message_len
        );
    }
    merge_resource(resource, parsed_resource, true, false);
    return RESPONSE_CODE_CHANGED;
}

// TODO EXCEPT Bootstrap server account
int on_bootstrap_delete_all(lwm2m_context *context) {
    int object_ids[context->object_tree->size];

    lwm2m_map_get_keys(context->object_tree, object_ids);
    for (int i = 0; i < context->object_tree->size; ++i) {
        lwm2m_object *object = lwm2m_map_get_object(context->object_tree, object_ids[i]);
        delete_object(context, object);
    }
    return 0;
}

int on_bootstrap_delete(lwm2m_context *context, lwm2m_instance *instance) {
    delete_instance(context, instance);
    return 0;
}

int on_bootstrap_finish(lwm2m_context *context) {
    pthread_mutex_lock(&context->bootstrap_mutex);
    context->state = BOOTSTRAPPED;
    pthread_cond_signal(&context->bootstrap_finished_condition);
    pthread_mutex_unlock(&context->bootstrap_mutex);
    return 0;
}

///////////// OTHER ///////////////////////

void lwm2m_wait_for_server_bootstrap(lwm2m_context *context) {
    pthread_mutex_lock(&context->bootstrap_mutex);
    while (context->state == WAITING_FOR_BOOTSTRAP || context->state == STARTED) {
        // TODO time wait for clientHolfOffTime
        pthread_cond_wait(&context->bootstrap_finished_condition, &context->bootstrap_mutex);
    }
    pthread_mutex_unlock(&context->bootstrap_mutex);
}

int initiate_bootstrap(lwm2m_context *context) {
    lwm2m_topic topic = {
            .operation = LWM2M_OPERATION_DEREGISTER,
            .type = "req",
            .token = generate_token(),
            .client_id = context->client_id,
//            .server_id = TODO server_id from security object
            .object_id = -1,
            .instance_id = -1,
            .resource_id = -1,
    };

    lwm2m_request request = {
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload = "",
            .payload_len = 0
    };
    perform_bootstrap_request(context, topic, request);
}

int lwm2m_bootstrap(lwm2m_context *context) {
    context->state = STARTED;
    context->is_bootstrap_ready = false;

    if (context->has_smartcard && context->smartcard_bootstrap_callback != NULL) {
        if (context->smartcard_bootstrap_callback(context) == 0 && has_server_instances(context)) {
            context->state = BOOTSTRAPPED;
        }
    } else if (context->factory_bootstrap_callback != NULL) {
        if (context->factory_bootstrap_callback(context) == 0 && has_server_instances(context)) {
            context->state = BOOTSTRAPPED;
        }
    }

    if (context->state != BOOTSTRAPPED) {
        // Server initiated bootstrap
        lwm2m_wait_for_server_bootstrap(context);
        if (context->state != BOOTSTRAPPED) {
            // Client initiated bootstrap TODO check response of bootstrap request
            initiate_bootstrap(context);
            context->state = WAITING_FOR_BOOTSTRAP;
            lwm2m_wait_for_server_bootstrap(context);
            if (context->state != BOOTSTRAPPED) {
                return -1;
            }
        }
    }
    return 0;
}