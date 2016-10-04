#ifndef LYNX_LWM2M_TRANSPORT_H
#define LYNX_LWM2M_TRANSPORT_H

#include "lwm2m.h"

///////////////// SERVER ADDRESS /////////////////////

typedef struct lwm2m_server_address {
    char *address;
    int port;
} lwm2m_server_address;

lwm2m_server_address *lwm2m_server_address_new();

///////////////// REQUEST /////////////////////

typedef struct lwm2m_request {
    char *endpoint;
    char *endpoint_client_name;
    char *payload;
    int reset; // for observe cancel
} lwm2m_request;

lwm2m_request *lwm2m_request_new();

///////////////// RESPONSE /////////////////////

typedef struct lwm2m_response {
    char *endpoint;
    char *payload;
    int response_code;
    int reset; // for observe cancel
    int created_instance_id;
} lwm2m_response;

lwm2m_response *lwm2m_response_new();

/*
 * Structures lwm2m_request/lwm2m_response can be used both for uplink and downlink request/response,
 *
 * 1. Downlink receive_request functions are used to resolve object/instance/resource on which operation
 * is performed. Also for example for "create" operation it resolves requested instance id from request
 *
 * 2. Uplink receive_response functions are used to implement reaction for server responses for uplink requests.
 * For example receive_notify_response is used for cancelling observation by "reset" message in response to notify.
 *
 * 3. Uplink send_request functions are used to perform uplink requests from LWM2M client to LWM2M server.
 * For example "register" operation is sent from client to server.
 *
 */


///////////////////// DEVICE MANAGEMENT //////////////////////

/* Receives a read request from LWM2M server and calls proper functions */
lwm2m_response receive_read_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Receives a write request from LWM2M server and calls proper functions */
lwm2m_response receive_write_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Receives a create request from LWM2M server and calls proper functions */
lwm2m_response receive_create_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Receives a delete request from LWM2M server and calls proper functions */
lwm2m_response receive_delete_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Receives a discover request from LWM2M server and calls proper functions */
lwm2m_response receive_discover_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Receives a write attributes request from LWM2M server and calls proper functions */
lwm2m_response receive_write_attributes_request(lwm2m_context *context, lwm2m_server_address *address,
                                                lwm2m_request *request);

//////////////////// INFORMATION REPORTING ///////////////////

/* Receives an observe request from LWM2M server and calls proper functions */
lwm2m_response receive_observe_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Receives a cancel observation request from LWM2M server and calls proper functions */
lwm2m_response receive_cancel_observation_request(lwm2m_context *context, lwm2m_server_address *address,
                                                  lwm2m_request *request);

/* Sends an uplink request to LWM2M server with notification */
int send_notify_request(lwm2m_context *context, lwm2m_server_address *address);

/* Receives a response for the uplink request. This is used for cancelling observation by "reset" message in response to notify */
lwm2m_response receive_notify_response(lwm2m_context *context, lwm2m_server_address *address, lwm2m_response *response);


/////////////////// REGISTRATION //////////////////////////////

/* Sends an uplink request to register to LWM2M server */
int send_register_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Sends an uplink request to send update to LWM2M server */
int send_update_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Sends an uplink request to deregister from LWM2M server */
int send_deregister_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);


/////////////////// BOOTSTRAP /////////////////////////////////

/* Receives a bootstrap write request from LWM2M server and calls proper functions */
int receive_bootstrap_write_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Receives a bootstrap delete request from LWM2M server and calls proper functions */
int receive_bootstrap_delete_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Receives a bootstrap finish request from LWM2M server and calls proper functions */
int receive_bootstrap_finish_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);

/* Sends an uplink request to request bootstrapping from LWM2M server */
int send_bootstrap_request_request(lwm2m_context *context, lwm2m_server* server);


#endif //LYNX_LWM2M_TRANSPORT_H
