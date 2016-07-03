#ifndef LYNX_LWM2M_LWM2M_REGISTER_H
#define LYNX_LWM2M_LWM2M_REGISTER_H

#include "lwm2m.h"
#include "lwm2m_object.h"

typedef struct lwm2m_register_message lwm2m_register_message;

/*
int on_registered(lwm2m_server* server, lwm2m_register_message *register_message); // (response????)
int on_deregistered(lwm2m_server* server);*/

/* Tries to register on all servers that has corresponding Server Object instance. Returns number of servers registered */
int lwm2m_register_on_all_servers(lwm2m_context* context);

struct lwm2m_register_message {
    char* endpoint_client_name;
    int lifetime;
    char* lwm2m_version;
    char* binding_mode;
    int sms_number;
    char* objects_and_instances;
};

#endif //LYNX_LWM2M_LWM2M_REGISTER_H
