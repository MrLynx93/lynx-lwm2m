#include "../include/lwm2m.h"
#include "../include/lwm2m_object.h"
#include "../include/lwm2m_register.h"
#include "../include/lwm2m_transport.h"


int lwm2m_register_on_all_servers(lwm2m_context* context) {
    lwm2m_map* server_instances = lwm2m_map_get(context->object_tree, SERVER_OBJECT_ID)->object.instances;
    int registered_servers = 0;

    int *instances_ids = (int *) malloc(sizeof(int) * server_instances->size);
    for(int i = 0, instance_id = server_instances[i]; i < server_instances->size; i++) {
        lwm2m_instance *instance = lwm2m_map_get(server_instances, instance_id)->instance;
        lwm2m_server *server = lwm2m_register(context, instance);
        if (server != NULL) {
            registered_servers++;
        }
    }
    return registered_servers;
}


lwm2m_server *lwm2m_register(lwm2m_context *context, lwm2m_instance *server_instance) {
    // Get necessary values
    lwm2m_resource *id_resource = lwm2m_map_get(instance->resources, SHORT_SERVER_ADDRESS_RESOURCE_ID)->resource;
    lwm2m_resource *lifetime_resource = lwm2m_map_get(server_instance->resources, LIFETIME_RESOURCE_ID)->resource;
    lwm2m_resource *binding_resource = lwm2m_map_get(server_instance->resources, BINDING_RESOURCE_ID)->resource;

    int short_server_id = id_resource.resource.single.value.int_value;
    lwm2m_server_address *address = lwm2m_map_get(context->server_addresses,short_server_id);
    char *endpoint_client_name = get_next_endpoint_client_name();


    // Create server
    lwm2m_server *server = lwm2m_server_new();
    server->context = context;
    server->endpoint_client_name = endpoint_client_name;
    server->short_server_id = short_server_id;
    server->binding_mode = binding_resource->resource.single.value.string_value;
    server->lifetime = lifetime_resource.resource.single.value.int_value;
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
    int error = send_register_request(context, server_address, request);
    if (error) {
        return NULL;
    }
    lwm2m_map_put(context->server_addresses, short_server_id, server);
    return server;
}

static char *create_registration_endpoint(lwm2m_server_address *address) {
    char *endpoint = (char *) malloc(sizeof(char *) * 100);
    strcat(endpoint, server_address->address);
    strcat(endpoint, ":");
    strcat(endpoint, itoa(server_address->port));
    return endpoint;
}

static char *create_registration_params(lwm2m_server *server) {
    char *params = (char *) malloc(sizeof(char *) * 100);

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

static char* get_next_endpoint_client_name() {
    return "abc";
}