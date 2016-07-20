#ifndef LYNX_LWM2M_ACCESS_CONTROL_H
#define LYNX_LWM2M_ACCESS_CONTROL_H

#include "context.h"
#include "lwm2m_object.h"
#include "lwm2m.h

///////////// CHECKING ACCESS CONTROL ///////////////////

/* Checks if object have an access control for a given LWM2M server (used only for create operation) */
int lwm2m_check_object_access_control(lwm2m_server *server, lwm2m_instance *instance);

/* Checks if instance have an access control for a given LWM2M server and operation */
int lwm2m_check_instance_access_control(lwm2m_server *server, lwm2m_instance *instance, int operation);

/* Checks if resource have an operation supported */
int lwm2m_check_resource_operation_supported(lwm2m_resource *resource, int operation);

/////////// GETTING ACCESS CONTROL OBJECT AND RESOURCES ////////////

/* Returns ACL object for context */
lwm2m_object *lwm2m_get_aco_object(lwm2m_context* context);

/* Returns ACL multiple resource from a given object */
lwm2m_resource_multiple* lwm2m_get_object_acl_resource(lwm2m_object* object);

/* Returns ACL multiple resource from a given instance */
lwm2m_resource_multiple* lwm2m_get_instance_acl_resource(lwm2m_instance* instance);

/* Returns ACL resource instance from ACL multiple resource for a given server */
l2m2m_resource_single *lwm2m_get_acl_resource_instance(lwm2m_resource_multiple *acl_resource, lwm2m_server *server);

/* Returns a default ACL resource instance which is defined at global level (context) */
lwm2m_resource_single *get_default_acl_resource_instance(lwm2m_context *context);

//////////// OTHERS ////////////////////////////////////

/* Returns a Short Server ID of LWM2M server which is Access Control Owner of a given instance */
int lwm2m_get_access_control_owner(lwm2m_instance *instance);

/* Creates new ACO instance for a lwm2m object */
lwm2m_instance *lwm2m_object_create_aco_instance(lwm2m_server* server, lwm2m_object* object);

/* Creates new ACO instance for a lwm2m instance */
lwm2m_instance *lwm2m_instance_create_aco_instance(lwm2m_server* server, lwm2m_instance* instance);



#endif //LYNX_LWM2M_ACCESS_CONTROL_H
