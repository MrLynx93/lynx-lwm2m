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
// TODO
bool lwm2m_check_instance_access_control(lwm2m_server *server, lwm2m_instance *instance, int operation) {
    if (server->context->servers->size == 1) {
        return true; // TODO why dont check operation ot supported????
    }

    lwm2m_resource *acl_resource = lwm2m_get_instance_acl_resource(instance);
    lwm2m_resource *acl_resource_instance = lwm2m_get_acl_resource_instance(acl_resource, server);

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

bool check_create_object_access_control(lwm2m_server *server, lwm2m_object *object) {
    return true; // TODO find ACO associated with object and check CREATE permission for this server
}

bool lwm2m_check_resource_operation_supported(lwm2m_resource *resource, int operation) {
    return (resource->operations & operation) != 0;
}

lwm2m_object *lwm2m_get_aco_object(lwm2m_context* context) {
    return (lwm2m_object*) lwm2m_map_get(context->object_tree, ACCESS_CONTROL_OBJECT_ID);
}

lwm2m_resource* lwm2m_get_object_acl_resource(lwm2m_object* object) {
    return lwm2m_map_get_resource(object->aco_instance->resources, ACL_RESOURCE_ID);
}

lwm2m_resource *lwm2m_get_instance_acl_resource(lwm2m_instance *instance) {
    return lwm2m_map_get_resource(instance->aco_instance->resources, ACL_RESOURCE_ID);
}

lwm2m_resource *lwm2m_get_acl_resource_instance(lwm2m_resource *acl_resource, lwm2m_server *server) {
    return lwm2m_map_get_resource(acl_resource->instances, server->short_server_id);
}

lwm2m_resource *get_default_acl_resource_instance(lwm2m_context *context, lwm2m_instance* instance) {
    lwm2m_resource* acl_multiple_resource = lwm2m_get_instance_acl_resource(instance);
    return lwm2m_map_get_resource(acl_multiple_resource->instances, SECURITY_OBJECT_ID);
}


/////////////// OTHERS ///////////////////

int lwm2m_get_access_control_owner(lwm2m_instance *instance) {
    lwm2m_resource* resource = (lwm2m_resource*) lwm2m_map_get(instance->aco_instance->resources, ACO_RESOURCE_ID);
    return resource->value->int_value;
}

lwm2m_instance *lwm2m_object_create_aco_instance(lwm2m_server* server, lwm2m_object* object) {
    lwm2m_instance* aco_instance = lwm2m_instance_new(server->context, lwm2m_get_aco_object(server->context)->id);
    aco_instance->resources = server->context->create_standard_resources_callback(ACCESS_CONTROL_OBJECT_ID);
    __set_value_int(lwm2m_map_get_resource(aco_instance->resources, OBJECT_ID_RESOURCE_ID), object->id);
    __set_value_int(lwm2m_map_get_resource(aco_instance->resources, INSTANCE_ID_RESOURCE_ID), 0);
    __set_value_int(lwm2m_map_get_resource(aco_instance->resources, ACO_RESOURCE_ID), server->short_server_id);
    // TODO setting default ACL resource instances
    return aco_instance;
}

lwm2m_instance *lwm2m_instance_create_aco_instance(lwm2m_server* server, lwm2m_instance* instance) {
    lwm2m_instance* aco_instance = lwm2m_instance_new(server->context, lwm2m_get_aco_object(server->context)->id);
    aco_instance->resources = server->context->create_standard_resources_callback(ACCESS_CONTROL_OBJECT_ID);
    __set_value_int(lwm2m_map_get_resource(aco_instance->resources, OBJECT_ID_RESOURCE_ID), instance->object->id);
    __set_value_int(lwm2m_map_get_resource(aco_instance->resources, INSTANCE_ID_RESOURCE_ID), instance->id);
    __set_value_int(lwm2m_map_get_resource(aco_instance->resources, ACO_RESOURCE_ID), server->short_server_id);

    /***** Create ACL multiple resource *****/
    lwm2m_map *acl = lwm2m_map_get_resource(aco_instance->resources, ACL_RESOURCE_ID)->instances;

    /***** Add ACL resource instance with all previliges *****/
    lwm2m_resource *server_resource_instance = lwm2m_resource_new(false);
    __set_value_int(server_resource_instance, READ | WRITE | EXECUTE | DELETE);
    lwm2m_map_put(acl, server->short_server_id, server_resource_instance);

    return aco_instance;
}
