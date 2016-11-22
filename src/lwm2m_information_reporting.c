#include <lwm2m.h>
#include <lwm2m_information_reporting.h>
#include <lwm2m_access_control.h>
#include <lwm2m_parser.h>

/**
 * Getting attribute according to 5.1.1
 *
 *
 *
 */

static int get_resource_pmax(lwm2m_server *server, lwm2m_resource *resource) {
    /**** Take R-Attribute ****/
    int *pmax = lwm2m_map_get_string(resource->attributes, "pmax");

    /**** Take OI-Attribute ****/
    if (pmax == NULL) {
        pmax = lwm2m_map_get_string(resource->instance->attributes, "pmax");
    }
    /**** Take O-Attribute ****/
    if (pmax == NULL) {
        pmax = lwm2m_map_get_string(resource->instance->object->attributes, "pmax");
    }
    /**** Take default attribute ****/
    if (pmax == NULL) {
        lwm2m_resource *pmax_resource = lwm2m_map_get_resource(server->server_instance->resources, 2);
        return pmax_resource->value->int_value;
    } else {
        return *pmax;
    }
}

static int get_instance_pmax(lwm2m_server *server, lwm2m_instance *instance) {
    /**** Take OI-Attribute ****/
    int *pmax = lwm2m_map_get_string(instance->attributes, "pmax");

    /**** Take O-Attribute ****/
    if (pmax == NULL) {
        pmax = lwm2m_map_get_string(instance->object->attributes, "pmax");
    }
    /**** Take default attribute ****/
    if (pmax == NULL) {
        lwm2m_resource *pmax_resource = lwm2m_map_get_resource(server->server_instance->resources, 2);
        return pmax_resource->value->int_value;
    } else {
        return *pmax;
    }
}

static int get_object_pmax(lwm2m_server *server, lwm2m_object *object) {
    /**** Take O-Attribute ****/
    int *pmax = lwm2m_map_get_string(object->attributes, "pmax");

    /**** Take default attribute ****/
    if (pmax == NULL) {
        lwm2m_resource *pmax_resource = lwm2m_map_get_resource(server->server_instance->resources, 2);
        return pmax_resource->value->int_value;
    } else {
        return *pmax;
    }
}

/**
 * Notify functions - these are called periodically
 *
 *
 *
 */
static void notify_object_on_server(lwm2m_server *server, lwm2m_object* object, char *token) {
    lwm2m_response response = {
            .content_type = CONTENT_TYPE_TLV,
            .response_code = RESPONSE_CODE_CONTENT,
            .payload = malloc(sizeof(char) * 1000),
    };

    lwm2m_topic topic = {
            .operation   = LWM2M_OPERATION_OBSERVE,
            .type        = "res",
            .client_id   = server->context->client_id,
            .server_id   = itoa(server->short_server_id),
            .token       = token,
            .object_id   = object->id,
            .instance_id = -1,
            .resource_id = -1
    };

    lwm2m_map *instances_to_parse = lwm2m_map_new();
    int keys[object->instances->size];
    lwm2m_map_get_keys(object->instances, keys);
    for (int i = 0; i < object->instances->size; ++i) {
        lwm2m_instance *instance = lwm2m_map_get_instance(object->instances, keys[i]);

        /**** Parse only instances, that have granted READ access ****/
        if (lwm2m_check_instance_access_control(server, instance, READ)) {
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
    perform_notify_response(server->context, topic, response);
}

static void notify_instance_on_server(lwm2m_server *server, lwm2m_instance* instance, char *token) {
    lwm2m_response response = {
            .content_type = CONTENT_TYPE_TLV,
            .response_code = RESPONSE_CODE_CONTENT,
            .payload = malloc(sizeof(char) * 1000),
    };

    lwm2m_topic topic = {
            .operation   = LWM2M_OPERATION_OBSERVE,
            .type        = "res",
            .client_id   = server->context->client_id,
            .server_id   = itoa(server->short_server_id),
            .token       = token,
            .object_id   = instance->object->id,
            .instance_id = instance->id,
            .resource_id = -1
    };

    /**** Parse only resources that are readable ****/
    lwm2m_map *resources_to_parse = lwm2m_map_new();
    int keys[instance->resources->size];
    lwm2m_map_get_keys(instance->resources, keys);
    for (int i = 0; i < instance->resources->size; ++i) {
        lwm2m_resource *resource = lwm2m_map_get_resource(instance->resources, keys[i]);
        if (lwm2m_check_resource_operation_supported(resource, READ)) {
            lwm2m_map_put(resources_to_parse, resource->id, resource);
        }
    }
    serialize_instance(resources_to_parse, response.payload, &response.payload_len);
    perform_notify_response(server->context, topic, response);
}

static void notify_resource_on_server(lwm2m_server *server, lwm2m_resource *resource, char* token) {
    lwm2m_response response = {
            .response_code = RESPONSE_CODE_CONTENT,
            .payload = malloc(sizeof(char) * 100),
    };

    lwm2m_topic topic = {
            .operation   = LWM2M_OPERATION_OBSERVE,
            .type        = "res",
            .client_id   = server->context->client_id,
            .server_id   = itoa(server->short_server_id),
            .token       = token,
            .object_id   = resource->instance->object->id,
            .instance_id = resource->instance->id,
            .resource_id = resource->id
    };

    // TODO should I check access every time?
    if (resource->multiple) {
        response.content_type = CONTENT_TYPE_TLV;
        serialize_multiple_resource(resource->instances, response.payload, &response.payload_len);
    } else {
        response.content_type = CONTENT_TYPE_TEXT;
        serialize_resource_text(resource, response.payload, &response.payload_len);
    }
    perform_notify_response(server->context, topic, response);
}

static void notify_object_func(void *server, void *object, void *token) {
    notify_object_on_server((lwm2m_server *)server, (lwm2m_object *)object, (char*) token);
}

static void notify_instance_func(void *server, void *instance, void *token) {
    notify_instance_on_server((lwm2m_server *)server, (lwm2m_instance *)instance, (char*) token);
}

static void notify_resource_func(void *server, void *resource, void *token) {
    notify_resource_on_server((lwm2m_server *)server, (lwm2m_resource *)resource, (char*) token);
}

/**
 * Observe functions - these are called once - when start observing
 *
 *
 *
 */
lwm2m_response on_resource_observe(lwm2m_server *server, lwm2m_resource *resource, char *token) {
    lwm2m_response response = {
            .response_code = RESPONSE_CODE_CONTENT,
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload = NULL,
            .payload_len = 0
    };

    /**** Check access for READ operation ****/
    if (!lwm2m_check_instance_access_control(server, resource->instance, READ)) {
        response.response_code = RESPONSE_CODE_UNAUTHORIZED;
        return response;
    }

    /**** Check if resource is readable ****/
    if (!lwm2m_check_resource_operation_supported(resource, READ)) {
        response.response_code = RESPONSE_CODE_METHOD_NOT_ALLOWED;
        return response;
    }

    /**** Scheule notify task ****/
    scheduler_task *notify_task = (scheduler_task *) malloc(sizeof(scheduler_task));
    notify_task->id = generate_task_id();
    notify_task->period = get_resource_pmax(server, resource);
    notify_task->function = notify_resource_func;
    notify_task->arg1 = server;
    notify_task->arg2 = resource;
    notify_task->arg3 = token;

    // TODO create map of observe tasks? should be useful in write attributes
    schedule(server->context->scheduler, notify_task);
    lwm2m_map_put(resource->observers, server->short_server_id, notify_task);
    return response;
}

lwm2m_response on_instance_observe(lwm2m_server *server, lwm2m_instance *instance, char *token) {
    lwm2m_response response = {
            .response_code = RESPONSE_CODE_CONTENT,
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload = NULL,
            .payload_len = 0
    };

    /**** Check access for READ operation ****/
    if (!lwm2m_check_instance_access_control(server, instance, READ)) {
        response.response_code = RESPONSE_CODE_UNAUTHORIZED;
        return response;
    }

    /**** Scheule notify task ****/
    scheduler_task *notify_task = (scheduler_task *) malloc(sizeof(scheduler_task));
    notify_task->id = generate_task_id();
    notify_task->period = get_instance_pmax(server, instance);
    notify_task->function = notify_instance_func;
    notify_task->arg1 = server;
    notify_task->arg2 = instance;
    notify_task->arg3 = token;

    // TODO create map of observe tasks? should be useful in write attributes
    schedule(server->context->scheduler, notify_task);
    lwm2m_map_put(instance->observers, server->short_server_id, notify_task);
    return response;
}

lwm2m_response on_object_observe(lwm2m_server *server, lwm2m_object *object, char *token) {
    lwm2m_response response = {
            .response_code = RESPONSE_CODE_CONTENT,
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload = NULL,
            .payload_len = 0
    };

    /**** Scheule notify task ****/
    scheduler_task *notify_task = (scheduler_task *) malloc(sizeof(scheduler_task));
    notify_task->id = generate_task_id();
    notify_task->period = get_object_pmax(server, object);
    notify_task->function = notify_object_func;
    notify_task->arg1 = server;
    notify_task->arg2 = object;
    notify_task->arg3 = token;

    schedule(server->context->scheduler, notify_task);
    lwm2m_map_put(object->observers, server->short_server_id, notify_task);
    return response;
}

lwm2m_response on_object_cancel_observe(lwm2m_server *server, lwm2m_object *object) {
    lwm2m_response response = {
            .response_code = RESPONSE_CODE_CONTENT,
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload = NULL,
            .payload_len = 0
    };

    lwm2m_scheduler *scheduler = server->context->scheduler;
    scheduler_task *notify_task = (scheduler_task *) lwm2m_map_get(object->observers, server->short_server_id);
    cancel(scheduler, notify_task);
    lwm2m_map_remove(object->observers, server->short_server_id);
    free(notify_task);
    return response;
}

lwm2m_response on_instance_cancel_observe(lwm2m_server *server, lwm2m_instance *instance) {
    lwm2m_response response = {
            .response_code = RESPONSE_CODE_CONTENT,
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload = NULL,
            .payload_len = 0
    };

    lwm2m_scheduler *scheduler = server->context->scheduler;
    scheduler_task *notify_task = (scheduler_task *) lwm2m_map_get(instance->observers, server->short_server_id);
    cancel(scheduler, notify_task);
    lwm2m_map_remove(instance->observers, server->short_server_id);
    free(notify_task);
    return response;
}

lwm2m_response on_resource_cancel_observe(lwm2m_server *server, lwm2m_resource *resource) {
    lwm2m_response response = {
            .response_code = RESPONSE_CODE_CONTENT,
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload = NULL,
            .payload_len = 0
    };

    lwm2m_scheduler *scheduler = server->context->scheduler;
    scheduler_task *notify_task = (scheduler_task *) lwm2m_map_get(resource->observers, server->short_server_id);
    cancel(scheduler, notify_task);
    lwm2m_map_remove(resource->observers, server->short_server_id);
    free(notify_task);
    return response;
}
