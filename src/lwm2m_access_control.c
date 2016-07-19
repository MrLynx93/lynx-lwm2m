#include "../include/lwm2m.h"
#include "../include/lwm2m_access_control.h"
#include "../include/lwm2m_device_management.h"
#include "../include/map.h"

int lwm2m_check_object_access_control(lwm2m_server *server, lwm2m_instance *instance) {
    lwm2m_resource_multiple *acl_resource = lwm2m_get_object_acl_resource(instance);
    lwm2m_resource_single *acl_resource_instance = lwm2m_get_acl_resource_instance(acl_resource, server);
    if (!(acl_resource_instance->value.int_value & CREATE)) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }
    return 0;
}

int lwm2m_check_instance_access_control(lwm2m_server *server, lwm2m_instance *instance, int operation) {
    if (server->context->servers->elements == 1) {
        return 0;
    }

    lwm2m_resource_multiple *acl_resource = lwm2m_get_instance_acl_resource(instance);
    lwm2m_resource_single *acl_resource_instance = lwm2m_get_acl_resource_instance(acl_resource, server);

    if (!lwm2m_get_access_control_owner(instance) == server->short_server_id && acl_resource_instance == NULL) {
        return 0;
    }

    if (acl_resource_instance == NULL) {
        acl_resource_instance = get_default_acl_resource_instance(server->context, instance);
    }
    if (acl_resource_instance == NULL) {
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
    return &(lwm2m_map_get(object->aco_instance->resources, 2)->resource.resource.multiple);
}

lwm2m_resource_multiple *lwm2m_get_instance_acl_resource(lwm2m_instance *instance) {
    return &(lwm2m_map_get(instance->aco_instance->resources, 2)->resource.resource.multiple);
}

l2m2m_resource_single *lwm2m_get_acl_resource_instance(lwm2m_resource_multiple *acl_resource, lwm2m_server *server) {
    return &(lwm2m_map_get(acl_resource->instances, server->short_server_id)->resource.resource.single);
}

int lwm2m_get_access_control_owner(lwm2m_instance *instance) {
    return lwm2m_map_get(instance->aco_instance->resources, 3)->resource.resource.single.value.int_value;
}

lwm2m_resource_single *get_default_acl_resource_instance(lwm2m_context *context, lwm2m_instance* instance) {
    lwm2m_resource_multiple* acl_multiple_resource = lwm2m_get_instance_acl_resource(instance);
    return lwm2m_map_get(acl_multiple_resource->instances, 0)->resource.resource.single;
}
