#ifndef LYNX_LWM2M_INFORMATION_REPORTING_H
#define LYNX_LWM2M_INFORMATION_REPORTING_H

#include "lwm2m_context.h"
#include "lwm2m_object.h"

typedef struct lwm2m_observe_session lwm2m_observe_session;


/////////////// OBSERVE //////////////////////

/* Checks access control and start observing object */
int on_lwm2m_object_observe(lwm2m_server *server, lwm2m_object *object);

/* Checks access control and start observing instance */
int on_lwm2m_instance_observe(lwm2m_server *server, lwm2m_instance *instance);

/* Checks access control and start observing resource */
int on_lwm2m_resource_observe(lwm2m_server *server, lwm2m_resource *resource);


////////////// CANCEL OBSERVATION /////////////

/* Cancels observation of object */
int on_cancel_lwm2m_object_observation(lwm2m_server *server, lwm2m_object *object);

/* Cancels observation of instance */
int on_cancel_lwm2m_instance_observation(lwm2m_server *server, lwm2m_instance *instance);

/* Cancels observation of resource */
int on_cancel_lwm2m_resource_observation(lwm2m_server *server, lwm2m_resource *resource);


///////////////// START OBSERVING //////////////


struct observe_session {
    lwm2m_server *server;
    lwm2m_node_type node_type;
    lwm2m_node *node;
};


#endif //LYNX_LWM2M_INFORMATION_REPORTING_H
