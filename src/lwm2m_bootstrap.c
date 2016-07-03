#include "../include/lwm2m_bootstrap.h"
#include "../include/lwm2m_register.h"

#define BOOTSTRAP_FAILED 1

///////////// CALLBACKS ////////////////////

// TODO insert object into tree
int on_bootstrap_object_write(lwm2m_server *server, lwm2m_object *object, char *message) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    deserialize_lwm2m_object(object, message);
}

int on_bootstrap_instance_write(lwm2m_server *server, lwm2m_instance *instance, char *message) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    deserialize_lwm2m_instance(instance, message);
}

int on_bootstrap_resource_write(lwm2m_server *server, lwm2m_resource *resource, char *message) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    deserialize_lwm2m_resource(resource, message);
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


int lwm2m_bootstrap(lwm2m_context *context) {
    context->bootstrap_state = STARTED;
    context->is_bootstrap_ready = false;

    if (context->has_smartcard && context->smartcard_bootstrap_callback != NULL) {
        if (context->smartcard_bootstrap_callback(context)) {
            context->bootstrap_state = BOOTSTRAPPED_BY_SMARTCARD;
        }
    }
    else if (context->factory_bootstrap_callback != NULL) {
        if (context->factory_bootstrap_callback(context)) {
            context->bootstrap_state = FACTORY_BOOTSTRAPPED;
        }
    }
    if (context->bootstrap_state != BOOTSTRAPPED_BY_SMARTCARD && context->bootstrap_state != FACTORY_BOOTSTRAPPED) {
        return BOOTSTRAP_FAILED;
    }

    if (has_server_instances(context)) {
        int registered_servers = lwm2m_register_on_all_servers(context);
        context->is_bootstrap_ready = true;

        if (registered_servers > 0) {
            context->bootstrap_state = BOOTSTRAPPED;
            return 0;
        }
        else {
            if (lwm2m_boostrap_client_initiated(context)) {
                context->bootstrap_state = BOOTSTRAPPED;
                return 0;
            }
            return BOOTSTRAP_FAILED;
        }
    }
    else {
        context->is_bootstrap_ready = true;
        context->bootstrap_state = WAITING_FOR_SERVER_BOOTSTRAP;
        if (lwm2m_wait_for_server_bootstrap(context)) {
            context->bootstrap_state = BOOTSTRAPPED;
            return 0;
        }
        else {
            if (lwm2m_boostrap_client_initiated(context)) {
                context->bootstrap_state = BOOTSTRAPPED;
                return 0;
            }
            return BOOTSTRAP_FAILED;
        }
    }
}