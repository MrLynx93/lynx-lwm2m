//#include "lwm2m.h"
//#include "lwm2m_object.h"
#include <lwm2m_register.h>
#include "lwm2m_transport.h"
#include "lwm2m_transport_mqtt.h"
//#include "lwm2m_device_management.h"


#define CHARSET "abcdefghijklmnopqrstuvwxyz1234567890"
#define CHARSET_LENGTH 36
#define TOKEN_LENGTH 8

static char *serialize_request(lwm2m_request request, int *message_len) {
    char *buffer = (char *) calloc(1000, sizeof(char));
    buffer[0] = (char) request.content_type;
    memcpy(buffer + 1, request.payload, request.payload_len);
    *message_len = (int) strlen(buffer + 1) + 1;
    return buffer;
}

static char *serialize_register_request(lwm2m_register_request request, int *message_len) {
    char *buffer = (char *) calloc(1000, sizeof(char));
    sprintf(buffer + 1, "%s?%s", request.header, request.payload);
    buffer[0] = (char) request.content_type;
    *message_len = (int) (strlen(buffer + 1) + 1);
    return buffer;
}

static char *serialize_topic(lwm2m_topic topic) {
    char *buffer = (char *) malloc(100 * sizeof(char));
    sprintf(buffer, "lynx/%s/%s/%s/%s/%s", topic.operation, topic.type, topic.token, topic.client_id, topic.server_id);
    if (topic.object_id != -1) {
        strcat(buffer, "/");
        strcat(buffer, itoa(topic.object_id));
    }
    if (topic.instance_id != -1) {
        strcat(buffer, "/");
        strcat(buffer, itoa(topic.instance_id));
    }
    if (topic.resource_id != -1) {
        strcat(buffer, "/");
        strcat(buffer, itoa(topic.resource_id));
    }
    return buffer;
}


char *generate_token() {
    srand(time(0));
    char* token = (char *) malloc(TOKEN_LENGTH + 1);
    for (int i = 0; i < TOKEN_LENGTH; ++i) {
        token[i] = CHARSET[rand() % CHARSET_LENGTH];
    }
    token[TOKEN_LENGTH] = 0;
    return token;
}

void perform_deregister_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    int message_len;
    char *topic_str = serialize_topic(topic);
    char *message = serialize_request(request, &message_len);
    publish(context, topic_str, message, message_len);
}

void perform_register_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_register_request request) {
    int message_len;
    char *topic_str = serialize_topic(topic);
    char *message = serialize_register_request(request, &message_len);

    publish(context, topic_str, message, message_len);
}

void perform_update_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_register_request request) {
    int message_len;
    char *topic_str = serialize_topic(topic);
    char *message = serialize_register_request(request, &message_len);

    publish(context, topic_str, message, message_len);
}

void handle_deregister_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response) {
    lwm2m_server *server = lwm2m_map_get(context->servers, atoi(topic.server_id));
    on_server_deregister(server, response.response_code);
}
void handle_register_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response) {
    lwm2m_server *server = lwm2m_map_get(context->servers, atoi(topic.server_id));
    on_server_register(server, response.response_code);
}

void handle_update_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response) {
    lwm2m_server *server = lwm2m_map_get(context->servers, atoi(topic.server_id));
    on_server_update(server, response.response_code);
}























//
//#define SUCCESS 0
//
//
//////////////////// RESOLVING SERVER ////////////////
//
////static lwm2m_server *resolve_lwm2m_server(lwm2m_context *context, lwm2m_server_address *address) {
////    return (lwm2m_server*) lwm2m_map_get(context->server_addresses, get_address_hash(address));
////}
//
//
//////////////////// RESOLVING OBJECT ////////////////
//
/////* request->endpoint is for example "/1/2/3?x=3&y=abc" */
////static int resolve_object_id(lwm2m_request *request) {
////    char *result;
////    char endpoint_copy[100];
////    memcpy(endpoint_copy, request->endpoint, strlen(request->endpoint));
////    result = strtok(endpoint_copy, "/");
////    result = strtok(endpoint_copy, "/");
////    return atoi(result);
////}
////
////static lwm2m_object *resolve_lwm2m_object(lwm2m_context *context, lwm2m_request *request) {
////    return (lwm2m_object*) lwm2m_map_get(context->object_tree, resolve_object_id(request));
////}
//
//
//////////////////// RESOLVING INSTANCE ////////////////
//
//static int resolve_instance_id(lwm2m_request *request) {
//    char *result;
//    char endpoint_copy[100];
//    memcpy(endpoint_copy, request->endpoint, strlen(request->endpoint));
//    result = strtok(endpoint_copy, "/");
//    result = strtok(endpoint_copy, "/");
//    result = strtok(endpoint_copy, "/");
//    return atoi(result);
//}
//
//static lwm2m_instance *resolve_lwm2m_instance(lwm2m_context *context, lwm2m_request *request) {
//    lwm2m_object *object = (lwm2m_object*) lwm2m_map_get(context->object_tree, resolve_object_id(request));
//    return (lwm2m_instance*) lwm2m_map_get(object->instances, resolve_instance_id(request));
//}
//
//
//////////////////// RESOLVING RESOURCE ////////////////
//
//static int resolve_resource_id(lwm2m_request *request) {
//    char *result;
//    char endpoint_copy[100];
//    memcpy(endpoint_copy, request->endpoint, strlen(request->endpoint));
//    result = strtok(endpoint_copy, "/");
//    result = strtok(endpoint_copy, "/");
//    result = strtok(endpoint_copy, "/");
//    result = strtok(endpoint_copy, "?");
//    return atoi(result);
//}
//
//static lwm2m_resource *resolve_lwm2m_resource(lwm2m_context *context, lwm2m_request *request) {
//    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
//    return (lwm2m_resource*) lwm2m_map_get(instance->resources, resolve_resource_id(request));
//}
//
/////////////////////// REGISTER ///////////////////////////////
//int send_register_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
//    return 0; // todo
//}
//
/////////////////////// DEVICE MANAGEMENT //////////////////////
//
////lwm2m_response receive_read_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
////    lwm2m_server *server = resolve_lwm2m_server(context, address);
////    lwm2m_object *object = resolve_lwm2m_object(context, request);
////    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
////    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);
////
////    int error;
////    char* response_payload = (char*) malloc(sizeof(char) * 100);
////    if (resource != NULL) {
////        error = on_lwm2m_resource_read(server, resource, &response_payload);
////    }
////    if (instance != NULL) {
////        error =  on_lwm2m_instance_read(server, instance, &response_payload);
////    }
////    if (object != NULL) {
////        error = on_lwm2m_object_read(server, object, &response_payload);
////    }
////
////    lwm2m_response response;
////    response.endpoint = request->endpoint;
////    response.reset = 0;
////    response.payload = response_payload;
////    response.response_code = error ? error : SUCCESS;
////    return response;
////}
////
////lwm2m_response receive_write_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
////    lwm2m_server *server = resolve_lwm2m_server(context, address);
////    lwm2m_object *object = resolve_lwm2m_object(context, request);
////    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
////    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);
////
////    int error;
////    if (resource != NULL) {
////        error = on_lwm2m_resource_write(server, resource, request->payload);
////    }
////    if (instance != NULL) {
////        error = on_lwm2m_instance_write(server, instance, request->payload);
////    }
////    if (object != NULL) {
////        // TODO bootstrap object write??? error = on_lwm2m_object_write(server, object, request->payload);
////    }
////
////    lwm2m_response response;
////    response.endpoint = request->endpoint;
////    response.reset = 0;
////    response.response_code = error ? error : SUCCESS;
////    return response;
////}
////
////lwm2m_response receive_create_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
////    lwm2m_server *server = resolve_lwm2m_server(context, address);
////    lwm2m_object *object = resolve_lwm2m_object(context, request);
////
////    int created_instance_id;
////    int error;
////    if (object != NULL) {
////        // TODO how to return created instance's id to server (in CoAP Created Location)
////        error = on_lwm2m_instance_create(server, object, request->payload, &created_instance_id);
////    }
////
////    lwm2m_response response;
////    response.endpoint = request->endpoint;
////    response.reset = 0;
////    response.response_code = error ? error : SUCCESS;
////    response.created_instance_id = created_instance_id;
////    return response;
////}
////
////lwm2m_response receive_delete_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
////    lwm2m_server *server = resolve_lwm2m_server(context, address);
////    lwm2m_object *object = resolve_lwm2m_object(context, request);
////    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
////
////    int error;
////    if (object != NULL) {
////        // TODO how to return created instance's id to server (in CoAP Created Location)
////        error = on_lwm2m_instance_delete(server, instance);
////    }
////
////    lwm2m_response response;
////    response.endpoint = request->endpoint;
////    response.reset = 0;
////    response.response_code = error ? error : SUCCESS;
////    return response;
////}
////
////lwm2m_response receive_discover_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
////    lwm2m_server *server = resolve_lwm2m_server(context, address);
////    lwm2m_object *object = resolve_lwm2m_object(context, request);
////    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
////    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);
////
////    char* response_payload = (char*) malloc(sizeof(char) * 100);
////    int error;
////    if (resource != NULL) {
////        error = on_lwm2m_resource_discover(server, resource, &response_payload);
////    }
////    if (instance != NULL) {
////        error = on_lwm2m_instance_discover(server, instance, &response_payload);
////    }
////    if (object != NULL) {
////        error = on_lwm2m_object_discover(server, object, &response_payload);
////    }
////
////    lwm2m_response response;
////    response.endpoint = request->endpoint;
////    response.payload = response_payload;
////    response.reset = 0;
////    response.response_code = error ? error : SUCCESS;
////    return response;
////}
//
////lwm2m_response receive_write_attributes_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
////    lwm2m_server *server = resolve_lwm2m_server(context, address);
////    lwm2m_object *object = resolve_lwm2m_object(context, request);
////    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
////    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);
////
////    int error;
////    if (resource != NULL) {
////        error = on_lwm2m_resource_write_attributes(server, resource, request->payload);
////    }
////    if (instance != NULL) {
////        error = on_lwm2m_instance_write_attributes(server, instance, request->payload);
////    }
////    if (object != NULL) {
////        error = on_lwm2m_object_write_attributes(server, object, request->payload);
////    }
////
////    lwm2m_response response;
////    response.endpoint = request->endpoint;
////    response.reset = 0;
////    response.response_code = error ? error : SUCCESS;
////    return response;
////}
//
//
////////////////////// INFORMATION REPORTING ///////////////////
//
//
////lwm2m_response receive_observe_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
////    lwm2m_server *server = resolve_lwm2m_server(context, address);
////    lwm2m_object *object = resolve_lwm2m_object(context, request);
////    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
////    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);
////
////    int error;
////    if (resource != NULL) {
////        error = on_lwm2m_resource_observe(server, resource);
////    }
////    if (instance != NULL) {
////        error = on_lwm2m_instance_observe(server, instance);
////    }
////    if (object != NULL) {
////        error = on_lwm2m_object_observe(server, object);
////    }
////
////    lwm2m_response response;
////    response.endpoint = request->endpoint;
////    response.reset = 0;
////    response.response_code = error ? error : SUCCESS;
////    return response;
////}
//
////lwm2m_response receive_cancel_observation_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
////    lwm2m_server *server = resolve_lwm2m_server(context, address);
////    lwm2m_object *object = resolve_lwm2m_object(context, request);
////    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
////    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);
////
////    int error;
////    if (resource != NULL) {
////        error = on_cancel_lwm2m_resource_observation(server, resource);
////    }
////    if (instance != NULL) {
////        error = on_cancel_lwm2m_instance_observation(server, instance);
////    }
////    if (object != NULL) {
////        error = on_cancel_lwm2m_object_observation(server, object);
////    }
////
////    lwm2m_response response;
////    response.endpoint = request->endpoint;
////    response.reset = 0;
////    response.response_code = error ? error : SUCCESS;
////}
////
////lwm2m_request send_notify_request(lwm2m_context *context, lwm2m_server_address *address) {
////    // TODO what here?
////}
//
/////////////////// SERVER ADDRESS, REQUEST AND RESPONSE /////////////////////
//
//lwm2m_server_address *lwm2m_server_address_new() {
//    lwm2m_server_address *address = (lwm2m_server_address *) malloc(sizeof(lwm2m_server_address));
//    return address;
//}
//
//lwm2m_request *lwm2m_request_new() {
//    lwm2m_request *request = (lwm2m_request *) malloc(sizeof(lwm2m_request));
//    return request;
//}
//
//lwm2m_response *lwm2m_response_new() {
//    lwm2m_response *response = (lwm2m_response *) malloc(sizeof(lwm2m_response));
//    return response;
//}