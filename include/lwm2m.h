#ifndef LYNX_LWM2M_LWM2M_H
#define LYNX_LWM2M_LWM2M_H

#include "lwm2m_context.h"
#include "lwm2m_object.h"
#include "lwm2m_attributes.h"
#include "lwm2m_device_management.h"
#include <stdbool.h>
#include <stdlib.h>

//// todo fix this
//typedef enum lwm2m_bootstrap_state {
//    s
//} lwm2m_bootstrap_state;

// TODO zrobic cos, zeby byly 2 sesje na bootstrapa i reszte

///////////////// MAIN ENDPOINT FUNCTIONS ///////////////////

/* Creates a new LWM2M client context */
lwm2m_context *lwm2m_create_context();

/* Creates LWM2M objects, performs bootstrap sequence and starts transport layer */
int lwm2m_start_client(lwm2m_context *context);

lwm2m_server *lwm2m_server_new();

/* Checks if any Server Object instance is created in context */
bool has_server_instances(lwm2m_context* context);


#endif //LYNX_LWM2M_LWM2M_H
