#include "../include/lwm2m_bootstrap.h"


int on_bootstrap_object_write(lwm2m_server* server, lwm2m_object* object, char* message) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    deserialize_lwm2m_object(object, message);
}

int on_bootstrap_instance_write(lwm2m_server* server, lwm2m_instance* instance, char* message) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    deserialize_lwm2m_instance(instance, message);
}

int on_bootstrap_resource_write(lwm2m_server* server, lwm2m_resource* resource, char* message) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    deserialize_lwm2m_resource(resource, message);
}

int on_bootstrap_delete(lwm2m_server* server, lwm2m_instance* instance) {
    if (!server->context->is_bootstrap_ready) {
        return OPERATION_NOT_ALLOWED;
    }
    lwm2m_delete_instance(instance);
}

int on_bootstrap_finish(lwm2m_server* server) {
    server->context->is_bootstrap_ready = false;
}