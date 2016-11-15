#include "lwm2m.h"
#include "lwm2m_object.h"
#include "lwm2m_access_control.h"


///////////// CHECKING ACCESS CONTROL ///////////////////

//int lwm2m_check_object_access_control(lwm2m_server *server, lwm2m_object *object) { // TODO what to do here
//    TODO
//
//    lwm2m_resource_multiple *acl_resource = lwm2m_get_instance_acl_resource(instance); // TODO check if instance or object
//    lwm2m_resource_single *acl_resource_instance = lwm2m_get_acl_resource_instance(acl_resource, server);
//    if (!(acl_resource_instance->value.int_value & CREATE)) {
//        return ACCESS_RIGHT_PERMISSION_DENIED;
//    }
//    return 0;
//}

/////////// GETTING ACCESS CONTROL OBJECT AND RESOURCES ////////////

// W/R/E/D ACCESS JEST SPRAWDZANY:
// 0. JEŻELI OWNER, GRANTED
// 1. PER INSTANCE (SPRAWDZANE PO INSTANCE -> ACCESS CONTROL INSTANCE -> ACL RESOURCE[SERVER_ID]
// 2. PER RESOURCE - CZY RESOURCE JEST NP. READONLY/EXECUTABLE ETC
// todo PROBABLY OPERATION PARAMETER WILL COME BACK HERE -
bool lwm2m_check_instance_access_control(lwm2m_server *server, lwm2m_instance *instance) {
    if (server->context->servers->size == 1) {
        return true; // TODO why dont check operation ot supported????
    }

    lwm2m_resource_multiple *acl_resource = lwm2m_get_instance_acl_resource(instance);
    lwm2m_resource_single *acl_resource_instance = lwm2m_get_acl_resource_instance(acl_resource, server);

    if (lwm2m_get_access_control_owner(instance) == server->short_server_id && acl_resource_instance == NULL) {
        return true;
    }

    if (acl_resource_instance == NULL) {
        acl_resource_instance = get_default_acl_resource_instance(server->context, instance);
    }

    if (acl_resource_instance == NULL) {
        return false;
    } else {
        // TODO get access from acl_resource_instance
    }
    return true;
}

bool lwm2m_check_resource_operation_supported(lwm2m_resource *resource, int operation) {
    return (resource->operations & operation) != 0;
}

lwm2m_object *lwm2m_get_aco_object(lwm2m_context* context) {
    return (lwm2m_object*) lwm2m_map_get(context->object_tree, ACCESS_CONTROL_OBJECT_ID);
}

lwm2m_resource_multiple* lwm2m_get_object_acl_resource(lwm2m_object* object) {
    lwm2m_resource* resource = (lwm2m_resource*) lwm2m_map_get(object->aco_instance->resources, ACL_RESOURCE_ID);
    return &(resource->resource.multiple);
}

lwm2m_resource_multiple *lwm2m_get_instance_acl_resource(lwm2m_instance *instance) {
    lwm2m_resource* resource = (lwm2m_resource*) lwm2m_map_get(instance->aco_instance->resources, ACL_RESOURCE_ID);
    return &(resource->resource.multiple);
}

lwm2m_resource_single *lwm2m_get_acl_resource_instance(lwm2m_resource_multiple *acl_resource, lwm2m_server *server) {
    lwm2m_resource* resource = (lwm2m_resource*) lwm2m_map_get(acl_resource->instances, server->short_server_id);
    return &(resource->resource.single);
}

lwm2m_resource_single *get_default_acl_resource_instance(lwm2m_context *context, lwm2m_instance* instance) {
    lwm2m_resource_multiple* acl_multiple_resource = lwm2m_get_instance_acl_resource(instance);
    lwm2m_resource* resource_instance = (lwm2m_resource*) lwm2m_map_get(acl_multiple_resource->instances, SECURITY_OBJECT_ID);
    return &(resource_instance->resource.single);
}


/////////////// OTHERS ///////////////////

int lwm2m_get_access_control_owner(lwm2m_instance *instance) {
    lwm2m_resource* resource = (lwm2m_resource*) lwm2m_map_get(instance->aco_instance->resources, ACO_RESOURCE_ID);
    return resource->resource.single.value.int_value;
}

lwm2m_instance *lwm2m_object_create_aco_instance(lwm2m_server* server, lwm2m_object* object) {
    lwm2m_instance* aco_instance = lwm2m_instance_new(lwm2m_get_aco_object(server->context));
    aco_instance->resources = server->context->create_standard_resources_callback(ACCESS_CONTROL_OBJECT_ID);
    ((lwm2m_resource*)lwm2m_map_get(aco_instance->resources, OBJECT_ID_RESOURCE_ID))->resource.single.value.int_value = object->id;
    ((lwm2m_resource*)lwm2m_map_get(aco_instance->resources, INSTANCE_ID_RESOURCE_ID))->resource.single.value.int_value = 0;
    ((lwm2m_resource*)lwm2m_map_get(aco_instance->resources, ACO_RESOURCE_ID))->resource.single.value.int_value = server->short_server_id;
    // TODO setting default ACL resource instances
    return aco_instance;
}

lwm2m_instance *lwm2m_instance_create_aco_instance(lwm2m_server* server, lwm2m_instance* instance) {
    lwm2m_instance* aco_instance = lwm2m_instance_new(lwm2m_get_aco_object(server->context));
    aco_instance->resources = server->context->create_standard_resources_callback(ACCESS_CONTROL_OBJECT_ID);
    ((lwm2m_resource*)lwm2m_map_get(aco_instance->resources, OBJECT_ID_RESOURCE_ID))->resource.single.value.int_value = instance->object->id;
    ((lwm2m_resource*)lwm2m_map_get(aco_instance->resources, INSTANCE_ID_RESOURCE_ID))->resource.single.value.int_value = instance->id;
    ((lwm2m_resource*)lwm2m_map_get(aco_instance->resources, ACO_RESOURCE_ID))->resource.single.value.int_value = server->short_server_id;
    // TODO setting default ACL resource instances
    return aco_instance;
}
