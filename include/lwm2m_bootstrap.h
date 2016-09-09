#ifndef LYNX_LWM2M_BOOTSTRAP_H
#define LYNX_LWM2M_BOOTSTRAP_H

#include "lwm2m_context.h"
#include "lwm2m_object.h"

/*
 * All functions return 0 if operation was performed correctly otherwise it returns error code
 * List of error codes:
 * - OPERATION_NOT_ALLOWED - when client is not bootstrap-ready
 * - PARSE_ERROR           - when it was not possible to parse object/resource/instance
 */
// todo inserting into tree should be done before this

///////////// CALLBACKS ////////////////////

/* Checks if client is ready for bootstrapping, parses TLV format message and writes values to object */
int on_bootstrap_object_write(lwm2m_server *server, lwm2m_object *object, char *message);

/* Checks if client is ready for bootstrapping, parses TLV format message and writes values to instance */
int on_bootstrap_instance_write(lwm2m_server *server, lwm2m_instance *instance, char *message);

/* Checks if client is ready for bootstrapping, parses message in proper format ad writes values to resource */
int on_bootstrap_resource_write(lwm2m_server *server, lwm2m_resource *resource, char *message);

/* Finishes bootstrapping and sets client not ready for bootstrapping */
int on_bootstrap_finish(lwm2m_server *server);


///////////// OTHER ///////////////////////

/* Performs all bootstrapped sequence at client start */
int lwm2m_bootstrap(lwm2m_context *context);

/* Wait for maximum ClientHoldOffTime for server initiated bootstrap */
int lwm2m_wait_for_server_bootstrap(lwm2m_context *context);

/* Starts client initiated bootstrap */
int lwm2m_bootstrap_client_initiated(lwm2m_context *context);


#endif //LYNX_LWM2M_BOOTSTRAP_H
