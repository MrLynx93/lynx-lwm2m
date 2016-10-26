#include "lwm2m.h"
#include "lwm2m_bootstrap.h"
#include "lwm2m_register.h"
#include "lwm2m_parser.h"

static bool has_server_instances(lwm2m_context *context) {
    return false; // TODO implement
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
    return 0; // todo
}

int lwm2m_bootstrap_client_initiated(lwm2m_context *context) {
    // TODO
    return 0; // todo
}

int lwm2m_bootstrap(lwm2m_context *context) {
    context->state = STARTED;
    context->is_bootstrap_ready = false;

    if (context->has_smartcard && context->smartcard_bootstrap_callback != NULL) {
        if (context->smartcard_bootstrap_callback(context)) {
            context->state = BOOTSTRAPPED;
        }
    } else if (context->factory_bootstrap_callback != NULL) {
        if (context->factory_bootstrap_callback(context)) {
            context->state = BOOTSTRAPPED;
        }
    }

    if (context->state != BOOTSTRAPPED || !has_server_instances(context)) {
        // Server initiated bootstrap
        int res = lwm2m_wait_for_server_bootstrap(context);
        if (res != 0) {
            // Client initiated bootstrap
            res = lwm2m_bootstrap_client_initiated(context);
            if (res != 0) {
                return -1;
            }
            context->state = WAITING_FOR_BOOTSTRAP;
            res = lwm2m_wait_for_server_bootstrap(context);
            if (res != 0) {
                return -1;
            }
        }
    }

    if (has_server_instances(context)) {
        context->state = REGISTERING;
        lwm2m_register_on_all_servers(context);
        lwm2m_wait_for_registration(context);
        if (registered_any_server(context)) {
            context->state = REGISTERED;
            return 0;
        }
    }
    return -1;
}