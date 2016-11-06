#include "lwm2m_client.h"
#include "lwm2m_transport_mqtt.h"
#include "lwm2m_register.h"
#include "lwm2m_bootstrap.h"
#include "scheduler.h"

////////////////// PREDEFINED OBJECTS ////////////////////

static lwm2m_map *create_standard_objects() {
    // Define security object
    lwm2m_object *security_object = lwm2m_object_new();
    security_object->id = SECURITY_OBJECT_ID;
    security_object->mandatory = true;
    security_object->multiple = true;
    security_object->object_urn = "urn:oma:lwm2m:oma:0";
    security_object->attributes = lwm2m_map_new();

    // Define server object
    lwm2m_object *server_object = lwm2m_object_new();
    server_object->id = SERVER_OBJECT_ID;
    server_object->mandatory = true;
    server_object->multiple = true;
    server_object->object_urn = "urn:oma:lwm2m:oma:1";
    server_object->attributes = lwm2m_map_new();

    // Define access control object
    lwm2m_object *access_control_object = lwm2m_object_new();
    access_control_object->id = ACCESS_CONTROL_OBJECT_ID;
    server_object->mandatory = false;
    access_control_object->multiple = true;
    access_control_object->object_urn = "urn:oma:lwm2m:oma:2";
    access_control_object->attributes = lwm2m_map_new();

    // Create list with objects (todo map?)
    lwm2m_map *objects = lwm2m_map_new();
    lwm2m_map_put(objects, 0, security_object);
    lwm2m_map_put(objects, 1, server_object);
    lwm2m_map_put(objects, 2, access_control_object);
    return objects;
}

static lwm2m_map *create_security_object_resources() {
    lwm2m_map* resources = lwm2m_map_new();
    lwm2m_resource *resource;

    resource = lwm2m_resource_new(false);
    resource->id = 0;
    resource->name = "LWM2M Server URI";
    resource->type = STRING;
    resource->mandatory = true;
    lwm2m_map_put(resources, 0, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 1;
    resource->name = "Bootstrap Server";
    resource->type = BOOLEAN;
    resource->mandatory = true;
    lwm2m_map_put(resources, 1, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 2;
    resource->name = "Security Mode";
    resource->type = INTEGER;
    resource->mandatory = true;
    lwm2m_map_put(resources, 2, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 3;
    resource->name = "Public Key or Identity";
    resource->type = OPAQUE;
    resource->mandatory = true;
    lwm2m_map_put(resources, 3, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 4;
    resource->name = "Server Public Key";
    resource->type = OPAQUE;
    resource->mandatory = true;
    lwm2m_map_put(resources, 4, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 5;
    resource->name = "Secret Key";
    resource->type = OPAQUE;
    resource->mandatory = true;
    lwm2m_map_put(resources, 5, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 6;
    resource->name = "SMS Security Mode";
    resource->type = INTEGER;
    resource->mandatory = false;
    lwm2m_map_put(resources, 6, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 7;
    resource->name = "SMS Key Binding Parameters";
    resource->type = OPAQUE;
    resource->mandatory = false;
    lwm2m_map_put(resources, 7, (void*)resource);


    resource = lwm2m_resource_new(false);
    resource->id = 8;
    resource->name = "SMS Binding Secret Key(s)";
    resource->type = OPAQUE;
    resource->mandatory = false;
    lwm2m_map_put(resources, 8, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 9;
    resource->name = "LWM2M Server SMS Number";
    resource->type = STRING;
    resource->mandatory = false;
    lwm2m_map_put(resources, 9, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 10;
    resource->name = "Short Server ID";
    resource->type = INTEGER;
    resource->mandatory = false;
    lwm2m_map_put(resources, 10, (void*)resource);


    resource = lwm2m_resource_new(false);
    resource->id = 11;
    resource->name = "Client Hold Off Time";
    resource->type = INTEGER;
    resource->mandatory = false;
    lwm2m_map_put(resources, 11, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 12;
    resource->name = "Bootstrap Server Account Timeout";
    resource->type = INTEGER;
    resource->mandatory = false;
    lwm2m_map_put(resources, 12, (void*)resource);

    // Can be modified only in bootstrap
    for (int i = 0; i < 13; i++) {
        ((lwm2m_resource*)lwm2m_map_get(resources, i))->operations = 0;
    }

    return resources;
}

static lwm2m_map *create_server_object_resources() {
    lwm2m_map* resources = lwm2m_map_new();
    lwm2m_resource *resource;

    resource = lwm2m_resource_new(false);
    resource->id = 0;
    resource->name = "Short Server ID";
    resource->type = INTEGER;
    resource->mandatory = true;
    resource->operations = READ;
    lwm2m_map_put(resources, 0, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 1;
    resource->name = "Lifetime";
    resource->type = INTEGER;
    resource->mandatory = true;
    resource->operations = READ & WRITE;
    lwm2m_map_put(resources, 1, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 2;
    resource->name = "Default Minimum Period";
    resource->type = INTEGER;
    resource->mandatory = false;
    resource->operations = READ & WRITE;
    lwm2m_map_put(resources, 2, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 3;
    resource->name = "Default Maximum Period";
    resource->type = INTEGER;
    resource->mandatory = false;
    resource->operations = READ & WRITE;
    lwm2m_map_put(resources, 3, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 4;
    resource->name = "Disable";
    resource->type = NONE;
    resource->mandatory = false;
    resource->operations = EXECUTE;
    lwm2m_map_put(resources, 4, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 5;
    resource->name = "Disable Timeout";
    resource->type = INTEGER;
    resource->mandatory = false;
    resource->operations = READ & WRITE;
    lwm2m_map_put(resources, 5, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 6;
    resource->name = "Notification Storing When Disabled or Offline";
    resource->type = BOOLEAN;
    resource->mandatory = true;
    resource->operations = READ & WRITE;
    lwm2m_map_put(resources, 6, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 7;
    resource->name = "Binding";
    resource->type = STRING;
    resource->mandatory = true;
    resource->operations = READ & WRITE;
    lwm2m_map_put(resources, 7, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 8;
    resource->name = "Registration Update Trigger";
    resource->type = NONE;
    resource->mandatory = true;
    resource->operations = EXECUTE;
    lwm2m_map_put(resources, 8, (void*)resource);

    return resources;
}

static lwm2m_map *create_access_control_object_resources() {
    lwm2m_map* resources = lwm2m_map_new();
    lwm2m_resource *resource;

    resource = lwm2m_resource_new(false);
    resource->id = 0;
    resource->name = "Object ID";
    resource->type = INTEGER;
    resource->mandatory = true;
    resource->operations = READ;
    lwm2m_map_put(resources, 0, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 1;
    resource->name = "Object Instance ID";
    resource->type = INTEGER;
    resource->mandatory = true;
    resource->operations = READ;
    lwm2m_map_put(resources, 1, (void*)resource);

    resource = lwm2m_resource_new(true);
    resource->id = 2;
    resource->name = "ACL";
    resource->type = INTEGER;
    resource->mandatory = false;
    resource->operations = READ & WRITE;
    lwm2m_map_put(resources, 2, (void*)resource);

    resource = lwm2m_resource_new(false);
    resource->id = 3;
    resource->name = "Access Control Owner";
    resource->type = INTEGER;
    resource->mandatory = true;
    resource->operations = READ & WRITE;
    lwm2m_map_put(resources, 3, (void*)resource);

    return resources;
}

static lwm2m_map *create_standard_resources(int object_id) {
    switch (object_id) {
        case SECURITY_OBJECT_ID:
            return create_security_object_resources();
        case SERVER_OBJECT_ID:
            return create_server_object_resources();
        case ACCESS_CONTROL_OBJECT_ID:
            return create_access_control_object_resources();
        default:
            return NULL;
    }
}

static int create_object_tree(lwm2m_context *context) {
    lwm2m_map *standard_objects = create_standard_objects();
    lwm2m_map *user_defined_objects = context->create_objects_callback();
    int keys[standard_objects->size + user_defined_objects->size];

    lwm2m_map_get_keys(standard_objects, keys);
    for (int i = 0; i < standard_objects->size; i++) {
        lwm2m_object *object = (lwm2m_object*) lwm2m_map_get(standard_objects, keys[i]);
        object->context = context;
        object->instances = lwm2m_map_new();
        lwm2m_map_put(context->object_tree, keys[i], (void*)object);
    }
    lwm2m_map_get_keys(user_defined_objects, keys);
    for (int i = 0; i < user_defined_objects->size; i++) {
        lwm2m_object *object = (lwm2m_object*) lwm2m_map_get(user_defined_objects, keys[i]);
        object->context = context;
        object->instances = lwm2m_map_new();
        lwm2m_map_put(context->object_tree, keys[i], (void*)object);
    }
    return 0;
}


///////////////// MAIN ENDPOINT FUNCTIONS ///////////////////

lwm2m_context *lwm2m_create_context() {
    lwm2m_context *context = (lwm2m_context *) malloc(sizeof(lwm2m_context));
    context->create_standard_resources_callback = create_standard_resources;
    context->object_tree = lwm2m_map_new();
    context->servers = lwm2m_map_new();
    context->update_tasks = lwm2m_map_new();
    context->is_bootstrap_ready = true;
    context->is_bootstrapped = false;

    context->bootstrap_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    context->bootstrap_finished_condition = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    return context;
}

void deregister(lwm2m_server *server) {
    deregister_on_server(server->context, server);
}

int lwm2m_start_client(lwm2m_context *context) {
    create_object_tree(context);

    start_mqtt(context);
    publish_connected(context);
    lwm2m_bootstrap(context);

    context->scheduler = (lwm2m_scheduler *) malloc(sizeof(lwm2m_scheduler));
    scheduler_start(context->scheduler);

    lwm2m_register(context);
    return 0;
}