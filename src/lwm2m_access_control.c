#include "lwm2m.h"
#include "lwm2m_access_control.h"

#define MAX_ID 66535

static lwm2m_object *__aco_object(lwm2m_context *context) {
    return (lwm2m_object *) lfind(context->object_tree, ACCESS_CONTROL_OBJECT_ID);
}

static int __get_access_control_owner(lwm2m_instance *instance) {
    lwm2m_resource *resource = (lwm2m_resource *) lfind(aco_for_instance(instance)->resources, ACO_RESOURCE_ID);
    return resource->value->int_value;
}

bool lwm2m_check_instance_access_control(lwm2m_server *server, lwm2m_instance *instance, int operation) {
    /** If there is only one server registered, grant access **/
    if (server->context->servers->size == 1) {
        return true; // TODO why dont check operation ot supported????
    }
    /** Get ACL resource instance for (instance, server) pair **/
    lwm2m_resource *acl_resource = lfind(aco_for_instance(instance)->resources, ACL_RESOURCE_ID);
    lwm2m_resource *acl_resource_instance = lfind(acl_resource->instances, server->short_server_id);

    /** If there is no ACL resource instance for server and server is ACO, then access is given **/
    if (acl_resource_instance == NULL) {
        if (__get_access_control_owner(instance) == server->short_server_id) {
            return true;
        }
        /** If there is no ACL resource instance for server and server is not ACO, then use default ACL instance (id=0) **/
        acl_resource_instance = lfind(acl_resource->instances, 0);

        /** If there is no default ACL resource instance, then revoke access **/
        if (acl_resource_instance == NULL) {
            return false;
        }
    }
    /** Check access nomally from ACL resource instance **/
    if (acl_resource_instance->value->int_value & operation) {
        return true;
    }
    return false;
}

bool check_create_object_access_control(lwm2m_server *server, lwm2m_object *object) {
    /** Find ACO instance associated with LWM2M object **/
    lwm2m_instance *aco_instance = aco_for_object(object);

    /** If there is no associated ACO instance, then revoke **/
    if (aco_instance == NULL) {
        return false;
    }

    /** Get ACL resource instance for (object, server) pair **/
    lwm2m_resource *acl_resource = lfind(aco_instance->resources, ACL_RESOURCE_ID);
    lwm2m_resource *acl_resource_instance = lfind(acl_resource->instances, server->short_server_id);

    /** If there is no ACL resource instance for server, get default **/
    if (acl_resource_instance == NULL) {
        acl_resource_instance = lfind(acl_resource->instances, 0);

        /** If there is no default ACL resource instance, then revoke access **/
        if (acl_resource_instance == NULL) {
            return false;
        }
    }
    /** Check access nomally from ACL resource instance **/
    if (acl_resource_instance->value->int_value & CREATE) {
        return true;
    }
    return false;
}

bool lwm2m_check_resource_operation_supported(lwm2m_resource *resource, int operation) {
    return (resource->operations & operation) != 0;
}

/////////////// OTHERS ///////////////////

// TODO is it necessary?
lwm2m_instance *lwm2m_object_create_aco_instance(lwm2m_server *server, lwm2m_object *object) {
    lwm2m_instance *aco_instance = lwm2m_instance_new(__aco_object(server->context));
    __set_value_int(lfind(aco_instance->resources, OBJECT_ID_RESOURCE_ID), object->id);
    __set_value_int(lfind(aco_instance->resources, INSTANCE_ID_RESOURCE_ID), 0);
    __set_value_int(lfind(aco_instance->resources, ACO_RESOURCE_ID), server->short_server_id);
    // TODO setting default ACL resource instances
    return aco_instance;
}

lwm2m_instance *lwm2m_instance_create_aco_instance(lwm2m_server *server, lwm2m_instance *instance) {
    lwm2m_instance *aco_instance = lwm2m_instance_new(__aco_object(server->context));
    __set_value_int(lfind(aco_instance->resources, OBJECT_ID_RESOURCE_ID), instance->object->id);
    __set_value_int(lfind(aco_instance->resources, INSTANCE_ID_RESOURCE_ID), instance->id);
    __set_value_int(lfind(aco_instance->resources, ACO_RESOURCE_ID), server->short_server_id);

    /***** Create ACL multiple resource *****/
    list *acl = ((lwm2m_resource *) lfind(aco_instance->resources, ACL_RESOURCE_ID))->instances;

    /***** Add ACL resource instance with all previliges (only for ACO) *****/
    lwm2m_resource *server_resource_instance = lwm2m_resource_new(false);
    __set_value_int(server_resource_instance, READ | WRITE | EXECUTE | DELETE);
    ladd(acl, server->short_server_id, server_resource_instance);

    return aco_instance;
}

lwm2m_instance *aco_for_instance(lwm2m_instance *instance) {
    lwm2m_context *context = instance->object->context;
    for (list_elem *elem = __aco_object(context)->instances->first; elem != NULL; elem = elem->next) {
        lwm2m_instance *aco_instance = elem->value;
        int object_id = ((lwm2m_resource *) lfind(aco_instance->resources, OBJECT_ID_RESOURCE_ID))->value->int_value;
        int instance_id = ((lwm2m_resource *) lfind(aco_instance->resources, INSTANCE_ID_RESOURCE_ID))->value->int_value;
        if (object_id == instance->object->id && instance_id == instance->id) {
            return aco_instance;
        }
    }
    return NULL;
}

lwm2m_instance *aco_for_object(lwm2m_object *object) {
    lwm2m_context *context = object->context;
    for (list_elem *elem = __aco_object(context)->instances->first; elem != NULL; elem = elem->next) {
        lwm2m_instance *aco_instance = elem->value;
        int object_id = ((lwm2m_resource *)lfind(aco_instance->resources, OBJECT_ID_RESOURCE_ID))->value->int_value;
        int instance_id = ((lwm2m_resource *)lfind(aco_instance->resources, INSTANCE_ID_RESOURCE_ID))->value->int_value;
        if (object_id == object->id && instance_id == MAX_ID) {
            return aco_instance;
        }
    }
    return NULL;
}