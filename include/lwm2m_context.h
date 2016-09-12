#ifndef LYNX_LWM2M_LWM2M_CONTEXT_H
#define LYNX_LWM2M_LWM2M_CONTEXT_H

#include "map.h"


char *itoa(int number);

typedef struct lwm2m_context lwm2m_context;
typedef struct lwm2m_server lwm2m_server;

typedef enum lwm2m_bootstrap_state lwm2m_bootstrap_state;

/////////// CALLBACKS ///////////

/* Used whenever new LWM2M instance is created to create resources for that instance */
typedef lwm2m_map *(lwm2m_create_resources_callback(int object_id));

/* Used to define custom LWM2M objects. Called when client starts and creates object tree */
typedef lwm2m_map *(lwm2m_create_objects_callback(void));

/* Used in factory bootstrap to bootstrap provide client with LWM2M instances */
typedef int ((*lwm2m_bootstrap_callback(lwm2m_context *)));


/////////// DEVICE MANAGEMENT ////////////


enum lwm2m_bootstrap_state {
    STARTED,
    BOOTSTRAPPED_BY_SMARTCARD,
    FACTORY_BOOTSTRAPPED,
    REGISTERING,
    WAITING_FOR_SERVER_BOOTSTRAP,
    CLIENT_INITIATED_BOOTSTRAPPING,
    BOOTSTRAPPED
};


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

    lwm2m_create_resources_callback *create_standard_resources_callback;
};

struct lwm2m_server {
    int short_server_id;
    lwm2m_context *context;
//    lwm2m_attributes_tree *lwm2m_attributes_tree; TODO what is this? I forgot

    // Registration data ???? this should be in server object?
    char *endpoint_client_name;
    int lifetime;
    char *lwm2m_version;
    char *binding_mode;
    int sms_number;
    char *objects_and_instances;


};


#endif //LYNX_LWM2M_LWM2M_CONTEXT_H
