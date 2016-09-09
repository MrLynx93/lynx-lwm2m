#include "../include/lwm2m.h"
#include "../include/lwm2m_object.h"
#include "../include/lwm2m_register.h"
#include "../include/lwm2m_transport.h"
#include "../include/lwm2m_parser.h"
#include <stdlib.h>
#include <string.h>

static char *create_registration_endpoint(lwm2m_server_address *address);
static char *create_registration_params(lwm2m_server *server);
static char *get_next_endpoint_client_name();
static lwm2m_server *lwm2m_register(lwm2m_context *context, lwm2m_instance *server_instance);

// TODO move somewhere?
static int SHORT_SERVER_ID_RESOURCE_ID = 0;
static int LIFETIME_RESOURCE_ID = 1;
static int BINDING_RESOURCE_ID = 7;

int lwm2m_register_on_all_servers(lwm2m_context *context) {
    lwm2m_map *server_instances = ((lwm2m_object*)lwm2m_map_get(context->object_tree, SERVER_OBJECT_ID))->instances;
    int registered_servers = 0;

    int *instances_ids = (int *) malloc(sizeof(int) * server_instances->size);
    for (int i = 0; i < server_instances->size; i++) {
        int instance_id = instances_ids[i];
        lwm2m_instance *instance = (lwm2m_instance*) lwm2m_map_get(server_instances, instance_id);
        lwm2m_server *server = lwm2m_register(context, instance);
        if (server != NULL) {
            registered_servers++;
        }
    }
    return registered_servers;
}


static lwm2m_server *lwm2m_register(lwm2m_context *context, lwm2m_instance *server_instance) {
    // Get necessary values
    lwm2m_resource *id_resource = (lwm2m_resource*) lwm2m_map_get(server_instance->resources, SHORT_SERVER_ID_RESOURCE_ID);
    lwm2m_resource *lifetime_resource = (lwm2m_resource*) lwm2m_map_get(server_instance->resources, LIFETIME_RESOURCE_ID);
    lwm2m_resource *binding_resource = (lwm2m_resource*) lwm2m_map_get(server_instance->resources, BINDING_RESOURCE_ID);

    int short_server_id = id_resource->resource.single.value.int_value;
    lwm2m_server_address *address = (lwm2m_server_address*) lwm2m_map_get(context->server_addresses, short_server_id);
    char *endpoint_client_name = get_next_endpoint_client_name();


    // Create server
    lwm2m_server *server = lwm2m_server_new();
    server->context = context;
    server->endpoint_client_name = endpoint_client_name;
    server->short_server_id = short_server_id;
    server->binding_mode = binding_resource->resource.single.value.string_value;
    server->lifetime = lifetime_resource->resource.single.value.int_value;
    server->objects_and_instances = serialize_lwm2m_objects_and_instances(context);

    // Serialize endpoint
    char *endpoint = create_registration_endpoint(address);
    char *params = create_registration_params(server);
    strcat(endpoint, params);

    // Create request
    lwm2m_request *request = lwm2m_request_new();
    request->endpoint_client_name = endpoint_client_name;
    request->endpoint = endpoint;
    request->payload = server->objects_and_instances;

    // Send request
    int error = send_register_request(context, address, request);
    if (error) {
        return NULL;
    }
    lwm2m_map_put(context->server_addresses, short_server_id, (void*)server);
    return server;
}

static char *create_registration_endpoint(lwm2m_server_address *address) {
    char *endpoint = (char *) malloc(sizeof(char *) * 100);
    strcat(endpoint, address->address);
    strcat(endpoint, ":");
    strcat(endpoint, itoa(address->port));
    return endpoint;
}

static char *create_registration_params(lwm2m_server *server) {
    char *params = (char *) malloc(sizeof(char *) * 100);
    char buffer[10];
    strcat(params, "ep=");
    strcat(params, server->endpoint_client_name);
    if (server->lifetime > 0) {
        strcat(params, "&lt=");
        strcat(params, itoa(server->lifetime));
    }
    if (strlen(server->binding_mode) > 0) {
        strcat(params, "&b");
        strcat(params, server->binding_mode);
    }
    return params;
}

static char *get_next_endpoint_client_name() {
    return "abc";
}