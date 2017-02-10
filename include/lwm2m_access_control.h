#ifndef LYNX_LWM2M_ACCESS_CONTROL_H
#define LYNX_LWM2M_ACCESS_CONTROL_H

#include "lwm2m_object.h"
#include "lwm2m.h"

///////////// CHECKING ACCESS CONTROL ///////////////////

/* Checks if instance have an access control for a given LWM2M server */
bool lwm2m_check_instance_access_control(lwm2m_server *server, lwm2m_instance *instance, int operation);

/* Checks if object have an access control for a given LWM2M server (used only for create operation) */
bool check_create_object_access_control(lwm2m_server *server, lwm2m_object *object);

/* Checks if resource have an operation supported */
bool lwm2m_check_resource_operation_supported(lwm2m_resource *resource, int operation);

/////////// OTHERS ////////////////////////////////////

/* Creates new ACO instance for a lwm2m object */
lwm2m_instance *lwm2m_object_create_aco_instance(lwm2m_server* server, lwm2m_object* object);

/* Creates new ACO instance for a lwm2m instance */
lwm2m_instance *lwm2m_instance_create_aco_instance(lwm2m_server* server, lwm2m_instance* instance);

lwm2m_instance *aco_for_instance(lwm2m_instance *instance);

lwm2m_instance *aco_for_object(lwm2m_object *object);

#endif //LYNX_LWM2M_ACCESS_CONTROL_H
