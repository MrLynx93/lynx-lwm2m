#include "../include/lwm2m_access_control.h"
#include "../include/lwm2m_device_management.h"

int l2m2m_check_object_access_control(lwm2m_server *server, lwm2m_instance *instance) {
    lwm2m_resource_multiple *acl_resource = lwm2m_get_object_acl_resource(instance);
    lwm2m_resource_single *acl_resource_instance = lwm2m_get_acl_resource_instance(acl_resource, server);
    if (!(acl_resource_instance->value.int_value & CREATE)) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }
    return 0;
}

int lwm2m_check_instance_access_control(lwm2m_server *server, lwm2m_instance *instance, int operation) {
    if (lwm2m_get_number_of_servers(server->context) == 1) {
        return 0;
    }

    lwm2m_resource_multiple *acl_resource = lwm2m_get_instance_acl_resource(instance);
    lwm2m_resource_single *acl_resource_instance = lwm2m_get_acl_resource_instance(acl_resource, server);

    if (!lwm2m_get_access_control_owner(instance) == server->short_server_id && acl_resource_instance == NULL) {
        return 0;
    }

    acl_resource_instance = acl_resource_instance != NULL ? get_default_acl_resource_instance(server->context);
    if (acl_resource_instance == null) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }

    if (!(acl_resource_instance->value.int_value & operation)) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }

    return 0;
}

int lwm2m_check_resource_operation_supported(lwm2m_resource *resource, int operation) {
    return resource->operations & operation;
}

lwm2m_resource_multiple* lwm2m_get_object_acl_resource(lwm2m_object* object) {
    // TODO use object->aco_instance;
}

lwm2m_resource_multiple *lwm2m_get_instance_acl_resource(lwm2m_instance *instance) {
    // TODO use instance->aco_instance;
}

l2m2m_resource_single *lwm2m_get_acl_resource_instance(lwm2m_resource_multiple *acl_resource, lwm2m_server *server) {
    // TODO
}

int lwm2m_get_access_control_owner(lwm2m_instance *instance) {
    // TODO
}

lwm2m_resource_single *get_default_acl_resource_instance(lwm2m_context *context) {
    // TODO
}
