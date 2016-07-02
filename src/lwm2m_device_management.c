#include "../include/lwm2m_device_management.h"
#include "../include/lwm2m_access_control.h
#include "../include/lwm2m_object.h"
#include "../include/lwm2m.h"

///////////////////// READ /////////////////////////

int on_object_read(lwm2m_server* server, lwm2m_object* object, char** message) {
    serialize_lwm2m_object(object, message);
    return 0;
}

int on_instance_read(lwm2m_server* server, lwm2m_instance* instance, char** message) {
    int access_error = check_instance_access(server, instance, READ);
    if (access_error) {
        return access_error;
    }
    serialize_lwm2m_instance(instance);
    return 0;
}


int on_resource_read(lwm2m_server* server, lwm2m_resource* resource, char** message) {
    int access_error = check_resource_access(server, resource, READ);
    if (access_error) {
        return access_error;
    }
    resource->read_callback(resource);
    serialize_lwm2m_resource(resource, message);
    return 0;
}

///////////////////// WRITE ////////////////////////

int on_instance_write(lwm2m_server* server, lwm2m_instance* instance, char* message) {
    int access_error = check_instance_access(server, instance, WRITE)
    if (access_error) {
        return access_error;
    }
    deserialize_lwm2m_instance(instance, message);
    return 0;
}

int on_resource_write(lwm2m_server* server, lwm2m_resource* resource, char* message) {
    int access_error = check_resource_access(server, resource, WRITE);
    if (access_error) {
        return access_error;
    }
    deserialize_lwm2m_resource(resource, message);
    return 0;
}

///////////////////////// DELETE //////////////////////

int on_instance_delete(lwm2m_server* server, lwm2m_instance* instance) {
    int access_error = check_instance_access(server, resource, DELETE);
    if (access_error) {
        return access_error;
    }
    lwm2m_delete_instance(instance);
}

///////////////////////// CREATE //////////////////////

int on_instance_create(lwm2m_server* server, lwm2m_object* object, char* message, int id) {
    int access_error = check_object_access(server, object);
    if (access_error) {
        return access_error;
    }
    lwm2m_instance *instance = lwm2m_instance_new();
    deserialize_lwm2m_instance(instance, message);
    instance->id = id;
}

///////////////////////// PRIVATE /////////////////////

static int check_object_access(lwm2m_server* server, lwm2m_object* object) {
    if (!lwm2m_check_object_access_control(server, object)) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }
    return 0;
}

static int check_instance_access(lwm2m_server* server, lwm2m_instance* instance, int operation) {
    if (!lwm2m_check_instance_access_control(server, instance, READ)) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }
    return 0;
}

static int check_resource_access(lwm2m_server* server, lwm2m_resource* resource, int operation) {
    if (!lwm2m_check_instance_access_control(server, resource->instance, READ)) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }
    if (!lwm2m_check_resource_operation_supported(resource, operation)) {
        return OPERATION_NOT_SUPPORTED;
    }
    return 0;
}

