#ifndef LYNX_LWM2M_TRANSPORT_H
#define LYNX_LWM2M_TRANSPORT_H

#include "lwm2m.h"

#define RESPONSE_CODE_CREATED 201
#define RESPONSE_CODE_DELETED 202
#define RESPONSE_CODE_CHANGED 204
#define RESPONSE_CODE_UNAUTHORIZED 401
#define RESPONSE_CODE_METHOD_NOT_ALLOWED 405

#define CONTENT_TYPE_NO_FORMAT 0
#define CONTENT_TYPE_TEXT 1
#define CONTENT_TYPE_LINK 2
#define CONTENT_TYPE_OPAQUE 3
#define CONTENT_TYPE_TLV 4

#define LWM2M_OPERATION_BOOTSTRAP_INIT   "br"
#define LWM2M_OPERATION_BOOTSTRAP_DELETE "bd"
#define LWM2M_OPERATION_BOOTSTRAP_WRITE  "bw"
#define LWM2M_OPERATION_BOOTSTRAP_FINISH "bf"
#define LWM2M_OPERATION_REGISTER       "rr"
#define LWM2M_OPERATION_DEREGISTER     "rd"
#define LWM2M_OPERATION_UPDATE         "ru"
#define LWM2M_OPERATION_WRITE "mw"
#define LWM2M_OPERATION_READ "mr"

///////////////// REQUEST /////////////////////

typedef struct lwm2m_request { // TODO observe flag???
    int content_type;
    char *payload;
    size_t payload_len;
} lwm2m_request;

typedef struct lwm2m_register_request {
    int content_type;
    char *header;
    char *payload;
    size_t payload_len;

} lwm2m_register_request;

typedef struct lwm2m_response {
    int content_type;
    int response_code;
    int success;
    char *payload;
    int payload_len;
} lwm2m_response;

typedef struct lwm2m_topic {
    char *operation;
    char *type; // req/res
    char *client_id;
    char *server_id;
    char *token;
    int object_id;      // -1 when undefined
    int instance_id;    // -1 when undefined
    int resource_id;    // -1 when undefined
} lwm2m_topic;


// TODO what about tokens????

char *serialize_topic(lwm2m_topic topic);

char *serialize_response(lwm2m_response response, int *message_len);

char *generate_token();

lwm2m_response handle_bootstrap_delete_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request response);

lwm2m_response handle_bootstrap_write_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_bootstrap_finish_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_write_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_read_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

void perform_bootstrap_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

void perform_deregister_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

void perform_register_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_register_request request);

void perform_update_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_register_request request);

void handle_deregister_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response);

void handle_register_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response);

void handle_update_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response);










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

//
/////////////////////// DEVICE MANAGEMENT //////////////////////
//
///* Receives a read request from LWM2M server and calls proper functions */
//lwm2m_response receive_read_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Receives a write request from LWM2M server and calls proper functions */
//lwm2m_response receive_write_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Receives a create request from LWM2M server and calls proper functions */
//lwm2m_response receive_create_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Receives a delete request from LWM2M server and calls proper functions */
//lwm2m_response receive_delete_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Receives a discover request from LWM2M server and calls proper functions */
//lwm2m_response receive_discover_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Receives a write attributes request from LWM2M server and calls proper functions */
//lwm2m_response receive_write_attributes_request(lwm2m_context *context, lwm2m_server_address *address,
//                                                lwm2m_request *request);
//
////////////////////// INFORMATION REPORTING ///////////////////
//
///* Receives an observe request from LWM2M server and calls proper functions */
//lwm2m_response receive_observe_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Receives a cancel observation request from LWM2M server and calls proper functions */
//lwm2m_response receive_cancel_observation_request(lwm2m_context *context, lwm2m_server_address *address,
//                                                  lwm2m_request *request);
//
///* Sends an uplink request to LWM2M server with notification */
//int send_notify_request(lwm2m_context *context, lwm2m_server_address *address);
//
///* Receives a response for the uplink request. This is used for cancelling observation by "reset" message in response to notify */
//lwm2m_response receive_notify_response(lwm2m_context *context, lwm2m_server_address *address, lwm2m_response *response);
//
//
///////////////////// REGISTRATION //////////////////////////////
//
///* Sends an uplink request to register to LWM2M server */
//int send_register_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Sends an uplink request to send update to LWM2M server */
//int send_update_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Sends an uplink request to deregister from LWM2M server */
//int send_deregister_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
//
///////////////////// BOOTSTRAP /////////////////////////////////
//
///* Receives a bootstrap write request from LWM2M server and calls proper functions */
//int receive_bootstrap_write_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Receives a bootstrap delete request from LWM2M server and calls proper functions */
//int receive_bootstrap_delete_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Receives a bootstrap finish request from LWM2M server and calls proper functions */
//int receive_bootstrap_finish_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request);
//
///* Sends an uplink request to request bootstrapping from LWM2M server */
//int send_bootstrap_request_request(lwm2m_context *context, lwm2m_server* server);


#endif //LYNX_LWM2M_TRANSPORT_H
