#ifndef LYNX_LWM2M_LWM2M_H
#define LYNX_LWM2M_LWM2M_H

#include "lwm2m.h"
#include "lwm2m_object.h"
#include "map.h"


///////////////// MAIN ENDPOINT FUNCTIONS ///////////////////

/* Creates a new LWM2M client context */
lwm2m_context *lwm2m_create_context();

/* Creates LWM2M objects, performs bootstrap sequence and starts transport layer */
int lwm2m_start_client(lwm2m_context *context);

void deregister(lwm2m_server *server);

#endif //LYNX_LWM2M_LWM2M_H
