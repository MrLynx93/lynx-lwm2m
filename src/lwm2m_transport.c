#include "../include/lwm2m_transport.h"
#include "../include/lwm2m_object.h"
#include "../include/lwm2m.h"







static lwm2m_server *resolve_lwm2m_server(lwm2m_context *context, lwm2m_server_address *address) {
    return lwm2m_map_get(context->server_addresses, address);
}

static lwm2m_object *resolve_lwm2m_object(lwm2m_context *context, lwm2m_request *request) {
    return lwm2m_map_get(context->object_tree, resolve_object_id(request));
}

static lwm2m_instance *resolve_lwm2m_object(lwm2m_context *context, lwm2m_request *request) {
    lwm2m_object *object = lwm2m_map_get(context->object_tree, resolve_object_id(request));
    return lwm2m_map_get(object->instances, resolve_instance_id(request));
}

static lwm2m_resource *resolve_lwm2m_resource(lwm2m_context *context, lwm2m_request *request) {
    lwm2m_instance *instance = resolve_lwm2m_object(context, request);
    return lwm2m_map_get(instance->resources, resolve_resource_id(request));
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