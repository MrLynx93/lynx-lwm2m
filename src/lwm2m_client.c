#include "lwm2m_client.h"
#include "lwm2m_transport_mqtt.h"
#include "lwm2m_register.h"
#include "lwm2m_bootstrap.h"
#include "scheduler.h"

////////////////// PREDEFINED OBJECTS ////////////////////

static lwm2m_resource *create_security_object_resources() {
    lwm2m_resource *resources = (lwm2m_resource *) malloc(13 * sizeof(lwm2m_resource));

    // TODO FOR ALL
//    resource->execute_callback = NULL;
//    resource->write_callback = NULL;
//    resource->read_callback = NULL;

    resources[0].multiple = false;
    resources[0].id = 0;
    resources[0].multiple = false;
    resources[0].name = "LWM2M Server URI";
    resources[0].type = STRING;
    resources[0].mandatory = true;
    resources[0].operations = NOOP;
    resources[0].read_callback = NULL;
    resources[0].write_callback = NULL;

    resources[1].multiple = false;
    resources[1].id = 1;
    resources[1].name = "Bootstrap Server";
    resources[1].type = BOOLEAN;
    resources[1].mandatory = true;
    resources[1].operations = NOOP;
    resources[1].read_callback = NULL;
    resources[1].write_callback = NULL;

    resources[2].multiple = false;
    resources[2].id = 2;
    resources[2].name = "Security Mode";
    resources[2].type = INTEGER;
    resources[2].mandatory = false; // TODO FOR NOW ONLY
    resources[2].operations = NOOP;
    resources[2].read_callback = NULL;
    resources[2].write_callback = NULL;

    resources[3].multiple = false;
    resources[3].id = 3;
    resources[3].name = "Public Key or Identity";
    resources[3].type = OPAQUE;
    resources[3].mandatory = false; // TODO FOR NOW ONLY
    resources[3].operations = NOOP;
    resources[3].read_callback = NULL;
    resources[3].write_callback = NULL;

    resources[4].multiple = false;
    resources[4].id = 4;
    resources[4].name = "Server Public Key";
    resources[4].type = OPAQUE;
    resources[4].mandatory = false; // TODO FOR NOW ONLY
    resources[4].operations = NOOP;
    resources[4].read_callback = NULL;
    resources[4].write_callback = NULL;

    resources[5].multiple = false;
    resources[5].id = 5;
    resources[5].name = "Secret Key";
    resources[5].type = OPAQUE;
    resources[5].mandatory = false; // TODO FOR NOW ONLY
    resources[5].operations = NOOP;
    resources[5].read_callback = NULL;
    resources[5].write_callback = NULL;

    resources[6].multiple = false;
    resources[6].id = 6;
    resources[6].name = "SMS Security Mode";
    resources[6].type = INTEGER;
    resources[6].mandatory = false;
    resources[6].operations = NOOP;
    resources[6].read_callback = NULL;
    resources[6].write_callback = NULL;

    resources[7].multiple = false;
    resources[7].id = 7;
    resources[7].name = "SMS Key Binding Parameters";
    resources[7].type = OPAQUE;
    resources[7].mandatory = false;
    resources[7].operations = NOOP;
    resources[7].read_callback = NULL;
    resources[7].write_callback = NULL;

    resources[8].multiple = false;
    resources[8].id = 8;
    resources[8].name = "SMS Binding Secret Key(s)";
    resources[8].type = OPAQUE;
    resources[8].mandatory = false;
    resources[8].operations = NOOP;
    resources[8].read_callback = NULL;
    resources[8].write_callback = NULL;

    resources[9].multiple = false;
    resources[9].id = 9;
    resources[9].name = "LWM2M Server SMS Number";
    resources[9].type = STRING;
    resources[9].mandatory = false;
    resources[9].operations = NOOP;
    resources[9].read_callback = NULL;
    resources[9].write_callback = NULL;

    resources[10].multiple = false;
    resources[10].id = 10;
    resources[10].name = "Short Server ID";
    resources[10].type = INTEGER;
    resources[10].mandatory = false;
    resources[10].operations = NOOP;
    resources[10].read_callback = NULL;
    resources[10].write_callback = NULL;

    resources[11].multiple = false;
    resources[11].id = 11;
    resources[11].name = "Client Hold Off Time";
    resources[11].type = INTEGER;
    resources[11].mandatory = false;
    resources[11].operations = NOOP;
    resources[11].read_callback = NULL;
    resources[11].write_callback = NULL;

    resources[12].multiple = false;
    resources[12].id = 12;
    resources[12].name = "Bootstrap Server Account Timeout";
    resources[12].type = INTEGER;
    resources[12].mandatory = false;
    resources[12].operations = NOOP;
    resources[12].read_callback = NULL;
    resources[12].write_callback = NULL;

    return resources;
}

static lwm2m_resource *create_server_object_resources() {
    lwm2m_resource* resources = (lwm2m_resource*) malloc(9 * sizeof(lwm2m_resource));

    resources[0].multiple = false;
    resources[0].id = 0;
    resources[0].name = "Short Server ID";
    resources[0].type = INTEGER;
    resources[0].mandatory = true;
    resources[0].operations = READ;
    resources[0].read_callback = NULL;
    resources[0].write_callback = NULL;

    resources[1].multiple = false;
    resources[1].id = 1;
    resources[1].name = "Lifetime";
    resources[1].type = INTEGER;
    resources[1].mandatory = true;
    resources[1].operations = READ | WRITE;
    resources[1].read_callback = NULL;
    resources[1].write_callback = NULL;

    resources[2].multiple = false;
    resources[2].id = 2;
    resources[2].name = "Default Minimum Period";
    resources[2].type = INTEGER;
    resources[2].mandatory = false;
    resources[2].operations = READ | WRITE;
    resources[2].read_callback = NULL;
    resources[2].write_callback = NULL;

    resources[3].multiple = false;
    resources[3].id = 3;
    resources[3].name = "Default Maximum Period";
    resources[3].type = INTEGER;
    resources[3].mandatory = false;
    resources[3].operations = READ | WRITE;
    resources[3].read_callback = NULL;
    resources[3].write_callback = NULL;

    resources[4].multiple = false;
    resources[4].id = 4;
    resources[4].name = "Disable";
    resources[4].type = NONE;
    resources[4].mandatory = false;
    resources[4].operations = EXECUTE;
    resources[4].read_callback = NULL;
    resources[4].write_callback = NULL;

    resources[5].multiple = false;
    resources[5].id = 5;
    resources[5].name = "Disable Timeout";
    resources[5].type = INTEGER;
    resources[5].mandatory = false;
    resources[5].operations = READ | WRITE;
    resources[5].read_callback = NULL;
    resources[5].write_callback = NULL;

    resources[6].multiple = false;
    resources[6].id = 6;
    resources[6].name = "Notification Storing When Disabled or Offline";
    resources[6].type = BOOLEAN;
    resources[6].mandatory = true;
    resources[6].operations = READ | WRITE;
    resources[6].read_callback = NULL;
    resources[6].write_callback = NULL;

    resources[7].multiple = false;
    resources[7].id = 7;
    resources[7].name = "Binding";
    resources[7].type = STRING;
    resources[7].mandatory = true;
    resources[7].operations = READ | WRITE;
    resources[7].read_callback = NULL;
    resources[7].write_callback = NULL;

    resources[8].multiple = false;
    resources[8].id = 8;
    resources[8].name = "Registration Update Trigger";
    resources[8].type = NONE;
    resources[8].mandatory = true;
    resources[8].operations = EXECUTE;
    resources[8].read_callback = NULL;
    resources[8].write_callback = NULL;

    return resources;
}

static lwm2m_resource *create_access_control_object_resources() {
    lwm2m_resource *resources = (lwm2m_resource*) malloc(4 * sizeof(lwm2m_resource));

    resources[0].multiple = false;
    resources[0].id = 0;
    resources[0].name = "Object ID";
    resources[0].type = INTEGER;
    resources[0].mandatory = true;
    resources[0].operations = READ;
    resources[0].read_callback = NULL;
    resources[0].write_callback = NULL;

    resources[1].multiple = false;
    resources[1].id = 1;
    resources[1].name = "Object Instance ID";
    resources[1].type = INTEGER;
    resources[1].mandatory = true;
    resources[1].operations = READ;
    resources[1].read_callback = NULL;
    resources[1].write_callback = NULL;

    resources[2].id = 2;
    resources[2].name = "ACL";
    resources[2].type = INTEGER;
    resources[2].mandatory = false;
    resources[2].operations = READ | WRITE;
    resources[2].read_callback = NULL;
    resources[2].write_callback = NULL;

    resources[3].multiple = false;
    resources[3].id = 3;
    resources[3].name = "Access Control Owner";
    resources[3].type = INTEGER;
    resources[3].mandatory = true;
    resources[3].operations = READ | WRITE;
    resources[3].read_callback = NULL;
    resources[3].write_callback = NULL;

    return resources;
}

static lwm2m_resource *create_standard_resources(int object_id) {
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

static list *create_standard_objects() {
    // Define security object
    lwm2m_object *security_object = lwm2m_object_new();
    security_object->id = SECURITY_OBJECT_ID;
    security_object->mandatory = true;
    security_object->multiple = true;
    security_object->object_urn = "urn:oma:lwm2m:oma:0";
    security_object->attributes = list_new();
    security_object->resource_def = create_standard_resources(SECURITY_OBJECT_ID);
    security_object->resource_def_len = 13;

    // Define server object
    lwm2m_object *server_object = lwm2m_object_new();
    server_object->id = SERVER_OBJECT_ID;
    server_object->mandatory = true;
    server_object->multiple = true;
    server_object->object_urn = "urn:oma:lwm2m:oma:1";
    server_object->attributes = list_new();
    server_object->resource_def = create_standard_resources(SERVER_OBJECT_ID);
    server_object->resource_def_len = 9;

    // Define access control object
    lwm2m_object *access_control_object = lwm2m_object_new();
    access_control_object->id = ACCESS_CONTROL_OBJECT_ID;
    access_control_object->mandatory = false;
    access_control_object->multiple = true;
    access_control_object->object_urn = "urn:oma:lwm2m:oma:2";
    access_control_object->attributes = list_new();
    access_control_object->resource_def = create_standard_resources(ACCESS_CONTROL_OBJECT_ID);
    access_control_object->resource_def_len = 4;

    // Create list with objects
    list *objects = list_new();
    ladd(objects, 0, security_object);
    ladd(objects, 1, server_object);
    ladd(objects, 2, access_control_object);
    return objects;
}

static int create_object_tree(lwm2m_context *context) {
    list *standard_objects = create_standard_objects();
    list *user_defined_objects = context->create_objects_callback();

    for (list_elem *elem = standard_objects->first; elem != NULL; elem = elem->next) {
        lwm2m_object *object = elem->value;
        object->context = context;
        object->instances = list_new();
        ladd(context->object_tree, elem->key, object);
    }
    for (list_elem *elem = user_defined_objects->first; elem != NULL; elem = elem->next) {
        lwm2m_object *object = elem->value;
        object->context = context;
        object->instances = list_new();
        ladd(context->object_tree, elem->key, object);
    }
    return 0;
}


///////////////// MAIN ENDPOINT FUNCTIONS ///////////////////

lwm2m_context *lwm2m_create_context() {
    lwm2m_context *context = (lwm2m_context *) malloc(sizeof(lwm2m_context));
    context->object_tree = list_new();
    context->servers = list_new();
    context->update_tasks = list_new();
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