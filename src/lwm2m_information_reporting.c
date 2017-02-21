#include <lwm2m.h>
#include <lwm2m_information_reporting.h>
#include <lwm2m_access_control.h>
#include <lwm2m_parser.h>

static bool __fulfill_GT(float gt, lwm2m_value *new_value, lwm2m_type type) {
    if (type == INTEGER) {
        return gt < new_value->int_value;
    }
    if (type == DOUBLE) {
        return gt < new_value->double_value;
    }
    return false;
}

static bool __fulfill_LT(float lt, lwm2m_value *new_value, lwm2m_type type) {
    if (type == INTEGER) {
        return lt > new_value->int_value;
    }
    if (type == DOUBLE) {
        return lt > new_value->double_value;
    }
    return false;
}

static bool __fulfill_STP(float stp, lwm2m_value *old_value, lwm2m_value *new_value, lwm2m_type type) {
    if (type == INTEGER) {
        float step = abs(old_value->int_value - new_value->int_value);
        return stp < step;
    }
    if (type == DOUBLE) {
        float step = (float) (old_value->double_value - new_value->double_value);
        step = step < 0 ? -step : step;
        return stp < step;
    }
    return false;
}


list *should_notify(lwm2m_resource *resource, lwm2m_value *old_value, lwm2m_value *new_value) {
    lwm2m_context *context = resource->instance->object->context;
    list *servers = list_new();

    /**** For each server check if should notify ****/
    for (list_elem *elem = context->servers->first; elem != NULL; elem = elem->next) {
        lwm2m_server *server = elem->value;

        /**** If value changed from NULL or to NULL, then notify ****/
        if (new_value == NULL && old_value != NULL) {
            ladd(servers, server->short_server_id, (void *)server);
            continue;
        }
        if (new_value != NULL && old_value == NULL) {
            ladd(servers, server->short_server_id, (void *)server);
            continue;
        }

        /**** For numeric resource check numeric parameters ****/
        if (resource->type == INTEGER || resource->type == DOUBLE) {
            float *gt = get_resource_gt(server, resource);
            float *lt = get_resource_lt(server, resource);
            float *stp = get_resource_stp(server, resource);

            /**** Check if "gt" is fulfilled ****/
            if (gt != NULL && !__fulfill_GT(*gt, new_value, resource->type)) {
                continue;
            }

            /**** Check if "lt" is fulfilled ****/
            if (lt != NULL && !__fulfill_LT(*lt, new_value, resource->type)) {
                continue;
            }

            /**** Check if "stp" is fulfilled ****/
            if (stp != NULL && !__fulfill_STP(*stp, old_value, new_value, resource->type)) {
                continue;
            }
        }

        /**** If fulfilled all conditions, notify this server ****/
        ladd(servers, server->short_server_id, (void *)server);
    }
    return servers;
}


/**
 * Notify functions - these are called periodically
 *
 *
 *
 */
static void notify_instance_on_server(scheduler_task *task, lwm2m_server *server, lwm2m_instance* instance, char *token) {
    lwm2m_response response = {
            .content_type = CONTENT_TYPE_TLV,
            .response_code = RESPONSE_CODE_CONTENT,
            .payload = malloc(sizeof(char) * 5000),
    };

    lwm2m_topic topic = {
            .operation   = LWM2M_OPERATION_OBSERVE,
            .type        = "res",
            .client_id   = server->context->client_id,
            .server_id   = copy_str(server->name),  // TODO convert ID to name
            .token       = token,
            .object_id   = instance->object->id,
            .instance_id = instance->id,
            .resource_id = -1
    };

    /**** Parse only resources that are readable ****/
    list *resources_to_parse = list_new();
    for (list_elem *elem = instance->resources->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *resource = elem->value;
        if (lwm2m_check_resource_operation_supported(resource, READ)) {
            ladd(resources_to_parse, resource->id, resource);
        }
    }
    serialize_instance(resources_to_parse, response.payload, &response.payload_len);
    perform_notify_response(server->context, topic, response);
    task->last_waking_time = time(0);
}

static void notify_resource_on_server(scheduler_task *task, lwm2m_server *server, lwm2m_resource *resource, char* token) {
    lwm2m_response response = {
            .response_code = RESPONSE_CODE_CONTENT,
            .payload = malloc(sizeof(char) * 5000),
    };

    lwm2m_topic topic = {
            .operation   = LWM2M_OPERATION_OBSERVE,
            .type        = "res",
            .client_id   = server->context->client_id,
            .server_id   = copy_str(server->name),
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
    task->last_waking_time = time(0);
}



static void notify_object_func(void *task, void *server, void *object, void *token, void *instance) {
    if (instance == NULL) {
        /**** Notify all instances ****/
        lwm2m_object* o = (lwm2m_object*) object;
        for (list_elem *elem = o->instances->first; elem != NULL; elem = elem->next) {
            lwm2m_instance *inst = elem->value;
            notify_instance_on_server((scheduler_task*) task, server, inst, token);
        }
    } else {
        /**** Notify on instance-level ****/
        notify_instance_on_server((scheduler_task*) task, (lwm2m_server *)server, (lwm2m_instance *)instance, (char*) token);
    }
    ((scheduler_task*) task)->last_waking_time = time(0);
}

static void notify_instance_func(void *task, void *server, void *instance, void *token, void *nothing) {
    notify_instance_on_server((scheduler_task*) task, (lwm2m_server *)server, (lwm2m_instance *)instance, (char*) token);
}

static void notify_resource_func(void *task, void *server, void *resource, void *token, void *nothing) {
    notify_resource_on_server((scheduler_task*) task, (lwm2m_server *)server, (lwm2m_resource *)resource, (char*) token);
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
    char *token_copy = malloc(strlen(token) + 1);
    strcpy(token_copy, token);

    scheduler_task *notify_task = (scheduler_task *) malloc(sizeof(scheduler_task));
    notify_task->id = generate_task_id();
    notify_task->short_server_id = -1;
    notify_task->period = *get_resource_pmax(server, resource, 2);
    notify_task->function = notify_resource_func;
    notify_task->last_waking_time = 0;
    notify_task->arg0 = notify_task;
    notify_task->arg1 = server;
    notify_task->arg2 = resource;
    notify_task->arg3 = token_copy;

    // TODO create map of observe tasks? should be useful in write attributes
    schedule(server->context->scheduler, notify_task);
    ladd(resource->observers, server->short_server_id, notify_task);
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
    char *token_copy = malloc(strlen(token) + 1);
    strcpy(token_copy, token);

    scheduler_task *notify_task = (scheduler_task *) malloc(sizeof(scheduler_task));
    notify_task->id = generate_task_id();
    notify_task->short_server_id = -1;
    notify_task->period = *get_instance_pmax(server, instance, 2);
    notify_task->function = notify_instance_func;
    notify_task->last_waking_time = 0;
    notify_task->arg0 = notify_task;
    notify_task->arg1 = server;
    notify_task->arg2 = instance;
    notify_task->arg3 = token_copy;

    // TODO create map of observe tasks? should be useful in write attributes
    schedule(server->context->scheduler, notify_task);
    ladd(instance->observers, server->short_server_id, notify_task);
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
    char *token_copy = malloc(strlen(token) + 1);
    strcpy(token_copy, token);

    scheduler_task *notify_task = (scheduler_task *) malloc(sizeof(scheduler_task));
    notify_task->id = generate_task_id();
    notify_task->short_server_id = -1;
    notify_task->period = *get_object_pmax(server, object, 2);
    notify_task->function = notify_object_func;
    notify_task->last_waking_time = 0;
    notify_task->arg0 = notify_task;
    notify_task->arg1 = server;
    notify_task->arg2 = object; // TODO IT WILL BE SET ON EACH NOTIFY
    notify_task->arg3 = token_copy;

    schedule(server->context->scheduler, notify_task);
    ladd(object->observers, server->short_server_id, notify_task);
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
    scheduler_task *notify_task = (scheduler_task *) lfind(object->observers, server->short_server_id);
    cancel(scheduler, notify_task);
    lremove(object->observers, server->short_server_id);
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
    scheduler_task *notify_task = (scheduler_task *) lfind(instance->observers, server->short_server_id);
    cancel(scheduler, notify_task);
    lremove(instance->observers, server->short_server_id);
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
    scheduler_task *notify_task = (scheduler_task *) lfind(resource->observers, server->short_server_id);
    cancel(scheduler, notify_task);
    lremove(resource->observers, server->short_server_id);
    free(notify_task);
    return response;
}
