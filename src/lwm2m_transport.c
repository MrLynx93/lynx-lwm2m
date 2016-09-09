#include "../include/lwm2m_context.h"
#include "../include/lwm2m_transport.h"
#include "../include/lwm2m_object.h"
#include "../include/lwm2m_device_management.h"
#include "../include/lwm2m_information_reporting.h"
#include <string.h>
#include <stdlib.h>

#define SUCCESS 0


//extern int on_lwm2m_object_read(lwm2m_server* server, lwm2m_object* object, char** message);
//extern int on_lwm2m_instance_read(lwm2m_server* server, lwm2m_instance* instance, char** message);
//extern int on_lwm2m_resource_read(lwm2m_server* server, lwm2m_resource* resource, char** message);
//extern int on_lwm2m_instance_write(lwm2m_server* server, lwm2m_instance* instance, char* message);
//extern int on_lwm2m_resource_write(lwm2m_server* server, lwm2m_resource* resource, char* message);
//extern int on_lwm2m_instance_delete(lwm2m_server* server, lwm2m_instance* instance);
//extern int on_lwm2m_instance_create(lwm2m_server* server, lwm2m_object* object, char* message, int* created_instance_id);
//extern int on_lwm2m_object_discover(lwm2m_server* server, lwm2m_object* object, char** message);
//extern int on_lwm2m_instance_discover(lwm2m_server* server, lwm2m_instance* instance, char** message);
//extern int on_lwm2m_resource_discover(lwm2m_server* server, lwm2m_resource* resource, char** message);
//extern int on_lwm2m_object_write_attributes(lwm2m_server* server, lwm2m_object* object, char* message);
//extern int on_lwm2m_instance_write_attributes(lwm2m_server* server, lwm2m_instance* instance, char* message);
//extern int on_lwm2m_resource_write_attributes(lwm2m_server* server, lwm2m_resource* resource, char* message);



static int resolve_instance_id(lwm2m_request *request);
static int resolve_object_id(lwm2m_request *request);
static int resolve_resource_id(lwm2m_request *request);
static lwm2m_resource *resolve_lwm2m_resource(lwm2m_context *context, lwm2m_request *request);
static lwm2m_instance *resolve_lwm2m_instance(lwm2m_context *context, lwm2m_request *request);
static lwm2m_object *resolve_lwm2m_object(lwm2m_context *context, lwm2m_request *request);
static lwm2m_server *resolve_lwm2m_server(lwm2m_context *context, lwm2m_server_address *address);

///////////////////// DEVICE MANAGEMENT //////////////////////

static int get_address_hash(lwm2m_server_address* address) {

}

lwm2m_response receive_read_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
    lwm2m_server *server = resolve_lwm2m_server(context, address);
    lwm2m_object *object = resolve_lwm2m_object(context, request);
    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);

    int error;
    char* response_payload = (char*) malloc(sizeof(char) * 100);
    if (resource != NULL) {
        error = on_lwm2m_resource_read(server, resource, &response_payload);
    }
    if (instance != NULL) {
        error =  on_lwm2m_instance_read(server, instance, &response_payload);
    }
    if (object != NULL) {
        error = on_lwm2m_object_read(server, object, &response_payload);
    }

    lwm2m_response response;
    response.endpoint = request->endpoint;
    response.reset = 0;
    response.payload = response_payload;
    response.response_code = error ? error : SUCCESS;
    return response;
}

lwm2m_response receive_write_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
    lwm2m_server *server = resolve_lwm2m_server(context, address);
    lwm2m_object *object = resolve_lwm2m_object(context, request);
    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);

    int error;
    if (resource != NULL) {
        error = on_lwm2m_resource_write(server, resource, request->payload);
    }
    if (instance != NULL) {
        error = on_lwm2m_instance_write(server, instance, request->payload);
    }
    if (object != NULL) {
        // TODO bootstrap object write??? error = on_lwm2m_object_write(server, object, request->payload);
    }

    lwm2m_response response;
    response.endpoint = request->endpoint;
    response.reset = 0;
    response.response_code = error ? error : SUCCESS;
    return response;
}

lwm2m_response receive_create_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
    lwm2m_server *server = resolve_lwm2m_server(context, address);
    lwm2m_object *object = resolve_lwm2m_object(context, request);

    int created_instance_id;
    int error;
    if (object != NULL) {
        // TODO how to return created instance's id to server (in CoAP Created Location)
        error = on_lwm2m_instance_create(server, object, request->payload, &created_instance_id);
    }

    lwm2m_response response;
    response.endpoint = request->endpoint;
    response.reset = 0;
    response.response_code = error ? error : SUCCESS;
    response.created_instance_id = created_instance_id;
    return response;
}

lwm2m_response receive_delete_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
    lwm2m_server *server = resolve_lwm2m_server(context, address);
    lwm2m_object *object = resolve_lwm2m_object(context, request);
    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);

    int error;
    if (object != NULL) {
        // TODO how to return created instance's id to server (in CoAP Created Location)
        error = on_lwm2m_instance_delete(server, instance);
    }

    lwm2m_response response;
    response.endpoint = request->endpoint;
    response.reset = 0;
    response.response_code = error ? error : SUCCESS;
    return response;
}

lwm2m_response receive_discover_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
    lwm2m_server *server = resolve_lwm2m_server(context, address);
    lwm2m_object *object = resolve_lwm2m_object(context, request);
    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);

    char* response_payload = (char*) malloc(sizeof(char) * 100);
    int error;
    if (resource != NULL) {
        error = on_lwm2m_resource_discover(server, resource, &response_payload);
    }
    if (instance != NULL) {
        error = on_lwm2m_instance_discover(server, instance, &response_payload);
    }
    if (object != NULL) {
        error = on_lwm2m_object_discover(server, object, &response_payload);
    }

    lwm2m_response response;
    response.endpoint = request->endpoint;
    response.payload = response_payload;
    response.reset = 0;
    response.response_code = error ? error : SUCCESS;
    return response;
}

lwm2m_response receive_write_attributes_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
    lwm2m_server *server = resolve_lwm2m_server(context, address);
    lwm2m_object *object = resolve_lwm2m_object(context, request);
    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);

    int error;
    if (resource != NULL) {
        error = on_lwm2m_resource_write_attributes(server, resource, request->payload);
    }
    if (instance != NULL) {
        error = on_lwm2m_instance_write_attributes(server, instance, request->payload);
    }
    if (object != NULL) {
        error = on_lwm2m_object_write_attributes(server, object, request->payload);
    }

    lwm2m_response response;
    response.endpoint = request->endpoint;
    response.reset = 0;
    response.response_code = error ? error : SUCCESS;
    return response;
}


//////////////////// INFORMATION REPORTING ///////////////////


lwm2m_response receive_observe_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
    lwm2m_server *server = resolve_lwm2m_server(context, address);
    lwm2m_object *object = resolve_lwm2m_object(context, request);
    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);

    int error;
    if (resource != NULL) {
        error = on_lwm2m_resource_observe(server, resource);
    }
    if (instance != NULL) {
        error = on_lwm2m_instance_observe(server, instance);
    }
    if (object != NULL) {
        error = on_lwm2m_object_observe(server, object);
    }

    lwm2m_response response;
    response.endpoint = request->endpoint;
    response.reset = 0;
    response.response_code = error ? error : SUCCESS;
    return response;
}

lwm2m_response receive_cancel_observation_request(lwm2m_context *context, lwm2m_server_address *address, lwm2m_request *request) {
    lwm2m_server *server = resolve_lwm2m_server(context, address);
    lwm2m_object *object = resolve_lwm2m_object(context, request);
    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
    lwm2m_resource *resource = resolve_lwm2m_resource(context, request);

    int error;
    if (resource != NULL) {
        error = on_cancel_lwm2m_resource_observation(server, resource);
    }
    if (instance != NULL) {
        error = on_cancel_lwm2m_instance_observation(server, instance);
    }
    if (object != NULL) {
        error = on_cancel_lwm2m_object_observation(server, object);
    }

    lwm2m_response response;
    response.endpoint = request->endpoint;
    response.reset = 0;
    response.response_code = error ? error : SUCCESS;
}

lwm2m_request send_notify_request(lwm2m_context *context, lwm2m_server_address *address) {
    // TODO what here?
}

////////////////// PRIVATE /////////////////

static lwm2m_server *resolve_lwm2m_server(lwm2m_context *context, lwm2m_server_address *address) {
    return (lwm2m_server*) lwm2m_map_get(context->server_addresses, get_address_hash(address));
}

static lwm2m_object *resolve_lwm2m_object(lwm2m_context *context, lwm2m_request *request) {
    return (lwm2m_object*) lwm2m_map_get(context->object_tree, resolve_object_id(request));
}

static lwm2m_instance *resolve_lwm2m_instance(lwm2m_context *context, lwm2m_request *request) {
    lwm2m_object *object = (lwm2m_object*) lwm2m_map_get(context->object_tree, resolve_object_id(request));
    return (lwm2m_instance*) lwm2m_map_get(object->instances, resolve_instance_id(request));
}

static lwm2m_resource *resolve_lwm2m_resource(lwm2m_context *context, lwm2m_request *request) {
    lwm2m_instance *instance = resolve_lwm2m_instance(context, request);
    return (lwm2m_resource*) lwm2m_map_get(instance->resources, resolve_resource_id(request));
}

/* request->endpoint is for example "/1/2/3?x=3&y=abc" */
static int resolve_object_id(lwm2m_request *request) {
    char *result;
    char endpoint_copy[100];
    memcpy(endpoint_copy, request->endpoint, strlen(request->endpoint));
    result = strtok(endpoint_copy, "/");
    result = strtok(endpoint_copy, "/");
    return atoi(result);
}

static int resolve_instance_id(lwm2m_request *request) {
    char *result;
    char endpoint_copy[100];
    memcpy(endpoint_copy, request->endpoint, strlen(request->endpoint));
    result = strtok(endpoint_copy, "/");
    result = strtok(endpoint_copy, "/");
    result = strtok(endpoint_copy, "/");
    return atoi(result);
}

static int resolve_resource_id(lwm2m_request *request) {
    char *result;
    char endpoint_copy[100];
    memcpy(endpoint_copy, request->endpoint, strlen(request->endpoint));
    result = strtok(endpoint_copy, "/");
    result = strtok(endpoint_copy, "/");
    result = strtok(endpoint_copy, "/");
    result = strtok(endpoint_copy, "?");
    return atoi(result);
}