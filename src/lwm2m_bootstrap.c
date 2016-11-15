#include <lwm2m_transport.h>
#include "lwm2m.h"
#include "lwm2m_bootstrap.h"
#include "lwm2m_parser.h"

static bool has_server_instances(lwm2m_context *context) {
    lwm2m_object *server_object = lwm2m_map_get_object(context->object_tree, SERVER_OBJECT_ID);
    return server_object->instances->size > 0;
}

// TODO move

static void free_value(lwm2m_value value, lwm2m_type type) {
//    if (type == OPAQUE) {
//        free(value.opaque_value);
//    }
//    if (type == STRING) {
//        free(value.string_value);
//    }
}

static void lwm2m_attribute_free(lwm2m_attribute *attribute) {
    free_value(attribute->numeric_value, attribute->type);
    free(attribute);
}

static void free_attributes(lwm2m_map *attributes) {
    char **keys = (char **) malloc(sizeof(char *) * attributes->size);
    lwm2m_map_get_keys_string(attributes, keys);

    for (int i = 0; i < attributes->size; ++i) {
        lwm2m_attribute *attribute = lwm2m_map_get_string(attributes, keys[i]);
        lwm2m_attribute_free(attribute);
    }
    lwm2m_map_free(attributes);
}

static void delete_resource(lwm2m_context *context, lwm2m_resource *resource) {
    if (!resource->multiple) {
        free_value(resource->resource.single.value, resource->type);
    }
    if (resource->multiple) {
        lwm2m_map *instances = resource->resource.multiple.instances;
        int keys[instances->size];

        for (int i = 0; i < instances->size; ++i) {
            lwm2m_value *value = lwm2m_map_get(instances, keys[i]);
            free_value(*value, resource->type);
            free(value);
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

static void override_instance(lwm2m_instance *old_instance, lwm2m_instance *new_instance) {
    int resource_keys[new_instance->resources->size];
    lwm2m_map_get_keys(new_instance->resources, resource_keys);

    // MERGE RESOURCES TODO free old resource values / multiple resource instances?
    for (int i = 0; i < new_instance->resources->size; ++i) {
        lwm2m_resource *new_resource = lwm2m_map_get_resource(new_instance->resources, resource_keys[i]);
        lwm2m_resource *old_resource = lwm2m_map_get_resource(old_instance->resources, resource_keys[i]);

        if (old_resource == NULL) {
            lwm2m_map_put(old_instance->resources, resource_keys[i], new_resource);
        } else {
            old_resource->resource = new_resource->resource;
        }
    }
}

///////////// CALLBACKS ////////////////////

// TODO insert object into tree
int on_bootstrap_object_write(lwm2m_context *context, lwm2m_object *object, char *message, int message_len) {
    lwm2m_map *new_instances = parse_object(context, object->id, message, message_len);
    int keys[new_instances->size];
    lwm2m_map_get_keys(new_instances, keys);

    // MERGE INSTANCES TODO should I execute write callbacks?
    for (int i = 0; i < new_instances->size; ++i) {
        lwm2m_instance *new_instance = lwm2m_map_get_instance(new_instances, keys[i]);
        lwm2m_instance *old_instance = lwm2m_map_get_instance(object->instances, keys[i]);
        
        if (old_instance == NULL) {
            lwm2m_map_put(object->instances, keys[i], new_instance);
        } else {
            override_instance(old_instance, new_instance);
        }
    }
    return 0;
}

int on_bootstrap_instance_write(lwm2m_context *context, lwm2m_object *object, int instance_id, char *message, int message_len) {
    lwm2m_instance *old_instance = lwm2m_map_get_instance(object->instances, instance_id);
    lwm2m_instance *new_instance = (lwm2m_instance *) malloc(sizeof(lwm2m_instance));
    new_instance->id = instance_id;
    new_instance->object = object;
    new_instance->resources = parse_instance(context, object->id, message, message_len);

    // Set instance in parsed resources (only needed in bootstrap) TODO (probably)
    int keys[new_instance->resources->size];
    lwm2m_map_get_keys(new_instance->resources, keys);
    for (int i = 0; i < new_instance->resources->size; ++i) {
        lwm2m_map_get_resource(new_instance->resources, keys[i])->instance = new_instance;
    }

    if (old_instance == NULL) {
        lwm2m_map_put(object->instances, instance_id, new_instance);
    } else {
        override_instance(old_instance, new_instance);
    };
    return 0;
}

int on_bootstrap_resource_write(lwm2m_context *context, lwm2m_resource *resource, char *message, int message_len) {
    int object_id = resource->instance->object->id;

    if (resource->multiple) {
        lwm2m_map *resource_instances = parse_multiple_resource(context, object_id, resource->id, message, message_len);
        resource->resource.multiple.instances = resource_instances;
        // TODO free old instances
    } else {
        lwm2m_resource *parsed_resource = parse_resource(context, resource->instance->object->id, resource->id, message, message_len);
        resource->resource.single.value = parsed_resource->resource.single.value;
        if (resource->type == STRING || resource->type == OPAQUE) {
            resource->resource.single.length = parsed_resource->resource.single.length;
            // TODO free old value?
        }
    }
    return 0;
}

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