#include <lwm2m_transport.h>
#include "lwm2m.h"
#include "lwm2m_bootstrap.h"
#include "lwm2m_parser.h"

static bool has_server_instances(lwm2m_context *context) {
    lwm2m_object *server_object = lwm2m_map_get_object(context->object_tree, SERVER_OBJECT_ID);
    return server_object->instances->size > 0;
}

///////////// CALLBACKS ////////////////////

// TODO insert object into tree
int on_bootstrap_object_write(lwm2m_server *server, lwm2m_object *object, char *message) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    deserialize_lwm2m_object(object, message, strlen(message));
}

int on_bootstrap_instance_write(lwm2m_server *server, lwm2m_instance *instance, char *message) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    deserialize_lwm2m_instance(instance, message, strlen(message)); // TODO check
}

int on_bootstrap_resource_write(lwm2m_server *server, lwm2m_resource *resource, char *message) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    deserialize_lwm2m_resource(resource, message, strlen(message), TLV_FORMAT); // TODO check format
}

int on_bootstrap_delete(lwm2m_server *server, lwm2m_instance *instance) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    lwm2m_delete_instance(instance);
}

int on_bootstrap_finish(lwm2m_server *server) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    server->context->is_bootstrap_ready = false;
}

///////////// OTHER ///////////////////////

int lwm2m_wait_for_server_bootstrap(lwm2m_context *context) {
    pthread_mutex_lock(&context->bootstrap_mutex);
    while (context->state == WAITING_FOR_BOOTSTRAP || context->state == STARTED) {
        pthread_cond_wait(&context->bootstrap_finished_condition, &context->bootstrap_mutex);
    }
    pthread_mutex_unlock(&context->bootstrap_mutex);

    return context->state == BOOTSTRAPPED;
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
        int res = lwm2m_wait_for_server_bootstrap(context);
        if (res != 0) {
            // Client initiated bootstrap TODO check response of bootstrap request
            initiate_bootstrap(context);
            context->state = WAITING_FOR_BOOTSTRAP;
            res = lwm2m_wait_for_server_bootstrap(context);
            if (res != 0) {
                return -1;
            }
        }
    }
    return 0;
}