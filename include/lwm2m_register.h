#ifndef LYNX_LWM2M_LWM2M_REGISTER_H
#define LYNX_LWM2M_LWM2M_REGISTER_H

#include "lwm2m.h"

/* Tries to register on all servers that has corresponding Server Object instance. Returns number of servers registered */
int lwm2m_register_on_all_servers(lwm2m_context* context);

/* Wait until all registration requests have a response. Returns -1 when timeout */
int lwm2m_wait_for_registration(lwm2m_context *context);

/* Checks if client registered successfully to any of lwm2m_servers */
bool registered_any_server(lwm2m_context *context);

#endif //LYNX_LWM2M_LWM2M_REGISTER_H
