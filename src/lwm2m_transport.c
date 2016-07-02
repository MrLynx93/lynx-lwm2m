#include "../include/lwm2m_transport.h"
#include "../include/lwm2m_object.h"
#include "../include/lwm2m.h"


typedef struct lwm2m_server_address lwm2m_server_address;

int receive_read(lwm2m_context* context, lwm2m_server_address* address, char* endpoint);
int receive_write(lwm2m_context* context, lwm2m_server_address* address, char* endpoint, char* message);
int receive_observe(lwm2m_context* context, lwm2m_server_address* address, char* endpoint);
int receive_cancel_observation(lwm2m_context* context, lwm2m_server_address* address, char* endpoint);

int receive_discover(lwm2m_context* context, lwm2m_server_address* address, char* endpoint);
int receive_write_attributes(lwm2m_context* context, lwm2m_server_address* address, char* endpoint, char* message);

int receive_register(lwm2m_context* context, lwm2m_server_address* address, char* message);
int receive_deregister(lwm2m_context* context, lwm2m_server_address* address);

int receive_bootstrap_write(lwm2m_context* context, lwm2m_server_address* address, char* endpoint, char* message);
int receive_bootstrap_delete(lwm2m_context* context, lwm2m_server_address* address, char* endpoint, char* message);
int receive_bootstrap_finish(lwm2m_context* context, lwm2m_server_address* address, char* endpoint);

