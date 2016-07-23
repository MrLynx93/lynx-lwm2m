#ifndef LYNX_LWM2M_LWM2M_H
#define LYNX_LWM2M_LWM2M_H

#include "../include/lwm2m_object.h"
#include "../include/lwm2m_device_management.h"
#include "../include/lwm2m_bootstrap.h"

// TODO zrobic cos, zeby byly 2 sesje na bootstrapa i reszte

typedef struct lwm2m_context lwm2m_context;
typedef struct lwm2m_server lwm2m_server;

///////////////// MAIN CALLBACKS ////////////////////////////

/* Used whenever new LWM2M instance is created to create resources for that instance */
typedef lwm2m_map* (lwm2m_create_resources_callback(int object_id));

/* Used to define custom LWM2M objects. Called when client starts and creates object tree */
typedef lwm2m_map *(lwm2m_create_objects_callback(void));

/* Used in factory bootstrap to bootstrap provide client with LWM2M instances */
typedef int ((*lwm2m_bootstrap_callback(lwm2m_context *)));


///////////////// MAIN ENDPOINT FUNCTIONS ///////////////////

/* Creates a new LWM2M client context */
lwm2m_context *lwm2m_create_context();

/* Creates LWM2M objects, performs bootstrap sequence and starts transport layer */
int lwm2m_start_client(lwm2m_context *context);

///* Performs factory bootstrap using bootstrap_callback to get LWM2M instances. Also creates a lwm2m_server object */
//int lwm2m_factory_bootstrap(lwm2m_context *context, lwm2m_bootstrap_callback *bootstrap_callback);
//
///* Performs LWM2M bootstrap request */
//int lwm2m_request_bootstrap(lwm2m_context *context);

////////////////// STRUCTS ///////////////////////////////////

/* Checks if any Server Object instance is created in context */
bool has_server_instances(lwm2m_context* context);

struct lwm2m_context {
    lwm2m_map *servers; // shortServerId -> server
    lwm2m_map *server_addresses; // "localhost:234" -> server

    lwm2m_map *object_tree;
    lwm2m_bootstrap_state bootstrap_state;
    int has_smartcard;
    int is_bootstrap_ready;
    int is_bootstrapped;

    lwm2m_create_resources_callback *create_resources_callback;
    lwm2m_create_objects_callback *create_objects_callback;

    lwm2m_bootstrap_callback *factory_bootstrap_callback;
    lwm2m_bootstrap_callback *smartcard_bootstrap_callback;

    static lwm2m_create_resources_callback *create_standard_resources_callback;
};

struct lwm2m_server {
    int short_server_id;
    lwm2m_context *context;
    lwm2m_attributes_tree *lwm2m_attributes_tree;

    // Registration data ???? this should be in server object?
    char *endpoint_client_name;
    int lifetime;
    char *lwm2m_version;
    char *binding_mode;
    int sms_number;
    char *objects_and_instances;


};

#endif //LYNX_LWM2M_LWM2M_H
