#ifndef LYNX_LWM2M_INFORMATION_REPORTING_H
#define LYNX_LWM2M_INFORMATION_REPORTING_H

#include "lwm2m_object.h"
#include "lwm2m.h"

typedef struct lwm2m_observe_session lwm2m_observe_session;

int start_observing(lwm2m_server* server, lwm2m_resource* resource);
int on_observe(lwm2m_server* server, lwm2m_resource* resource);
int on_cancel_observation(lwm2m_server* server, lwm2m_resource *resource);


struct observe_session {
    lwm2m_node_type node_type;
    lwm2m_node node;
};

#endif //LYNX_LWM2M_INFORMATION_REPORTING_H
