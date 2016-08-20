#ifndef LYNX_LWM2M_LWM2M_REGISTER_H
#define LYNX_LWM2M_LWM2M_REGISTER_H

#include "../include/lwm2m.h"
#include "../include/lwm2m_object.h"

/* Tries to register on all servers that has corresponding Server Object instance. Returns number of servers registered */
int lwm2m_register_on_all_servers(lwm2m_context* context);

#endif //LYNX_LWM2M_LWM2M_REGISTER_H
