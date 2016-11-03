#ifndef LYNX_LWM2M_LWM2M_REGISTER_H
#define LYNX_LWM2M_LWM2M_REGISTER_H

#include "lwm2m.h"

/* Assuming that bootstrap has been finished and transport is started, register on all servers */
int lwm2m_register(lwm2m_context* context);

void deregister_on_server(lwm2m_context *context, lwm2m_server *server);
void register_on_server(lwm2m_context *context, lwm2m_instance *server_instance);
void update_on_server(lwm2m_context *context, lwm2m_server *server);

void on_server_deregister(lwm2m_server* server, int response_code);
void on_server_register(lwm2m_server* server, int response_code);
void on_server_update(lwm2m_server* server, int response_code);

/* Checks if client registered successfully to any of lwm2m_servers */
bool registered_any_server(lwm2m_context *context);

#endif //LYNX_LWM2M_LWM2M_REGISTER_H
