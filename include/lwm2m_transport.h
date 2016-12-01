#ifndef LYNX_LWM2M_TRANSPORT_H
#define LYNX_LWM2M_TRANSPORT_H

#include "lwm2m.h"

#define RESPONSE_CODE_CREATED 201
#define RESPONSE_CODE_DELETED 202
#define RESPONSE_CODE_CHANGED 204
#define RESPONSE_CODE_CONTENT 205
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
#define LWM2M_OPERATION_CREATE "mc"
#define LWM2M_OPERATION_DELETE "md"
#define LWM2M_OPERATION_EXECUTE "me"
#define LWM2M_OPERATION_WRITE "mw"
#define LWM2M_OPERATION_READ "mr"
#define LWM2M_OPERATION_DISCOVER "mm" // todo check
#define LWM2M_OPERATION_WRITE_ATTRIBUTES "ma" // todo check
#define LWM2M_OPERATION_OBSERVE "io" // todo check
#define LWM2M_OPERATION_CANCEL_OBSERVE "ic" // todo check

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

char *serialize_topic(lwm2m_topic topic, char *message);

char *serialize_response(lwm2m_response response, char *message, int *message_len);

char *generate_token();

lwm2m_response handle_bootstrap_delete_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request response);

lwm2m_response handle_bootstrap_write_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_bootstrap_finish_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_write_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_read_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_create_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_delete_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_discover_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_observe_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_cancel_observe_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_write_attributes_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

lwm2m_response handle_execute_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

list *__parse_args(lwm2m_request request); // todo static


void perform_bootstrap_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

void perform_deregister_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request);

void perform_register_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_register_request request);

void perform_update_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_register_request request);

void perform_notify_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response);

void handle_deregister_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response);

void handle_register_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response);

void handle_update_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response);

#endif //LYNX_LWM2M_TRANSPORT_H
