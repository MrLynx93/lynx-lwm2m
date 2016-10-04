//#ifndef LYNX_LWM2M_INFORMATION_REPORTING_H
//#define LYNX_LWM2M_INFORMATION_REPORTING_H
//
//#include "lwm2m.h"
//#include "lwm2m_object.h"
//
//typedef struct observe_session {
//    lwm2m_server *server;
//    lwm2m_node_type node_type;
//    lwm2m_node *node;
//} lwm2m_observe_session;
//
///////////////// OBSERVE //////////////////////
//
///* Checks access control and start observing object */
//int on_lwm2m_object_observe(lwm2m_server *server, lwm2m_object *object);
//
///* Checks access control and start observing instance */
//int on_lwm2m_instance_observe(lwm2m_server *server, lwm2m_instance *instance);
//
///* Checks access control and start observing resource */
//int on_lwm2m_resource_observe(lwm2m_server *server, lwm2m_resource *resource);
//
//
//////////////// CANCEL OBSERVATION /////////////
//
///* Cancels observation of object */
//int on_cancel_lwm2m_object_observation(lwm2m_server *server, lwm2m_object *object);
//
///* Cancels observation of instance */
//int on_cancel_lwm2m_instance_observation(lwm2m_server *server, lwm2m_instance *instance);
//
///* Cancels observation of resource */
//int on_cancel_lwm2m_resource_observation(lwm2m_server *server, lwm2m_resource *resource);
//
//
/////////////////// START OBSERVING //////////////
//
///* Checks numeric conditions of resource to notify (used in LWM2M write operation */
//int should_notify(lwm2m_resource *resource, lwm2m_value previous_value);
//
//
//#endif //LYNX_LWM2M_INFORMATION_REPORTING_H
