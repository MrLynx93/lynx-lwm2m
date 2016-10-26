#ifndef LYNX_LWM2M_LWM2M_CONTEXT_H
#define LYNX_LWM2M_LWM2M_CONTEXT_H

#include "map.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define READ 1
#define WRITE 2
#define EXECUTE 4
#define DELETE 8
#define CREATE 16

//////////////////// ERRORS //////////////////////////

#define OPERATION_NOT_SUPPORTED 1
#define ACCESS_RIGHT_PERMISSION_DENIED 2
#define STRANGE_ERROR 2
#define OPERATION_NOT_ALLOWED 1
#define PARSE_ERROR 1


//////////////////// UTILS //////////////////////////

char *itoa(int number);

///////////////////// STATE  //////////////////////

typedef enum lwm2m_state {
    STARTED,
    WAITING_FOR_BOOTSTRAP,
    BOOTSTRAPPED,
    REGISTERING,
    REGISTERED
} lwm2m_state;

/////////// CALLBACKS (DEFINITION OF OBJETS) ///////////

/* Used whenever new LWM2M instance is created to create resources for that instance */
typedef lwm2m_map *(lwm2m_create_resources_callback(int object_id));

/* Used to define custom LWM2M objects. Called when client starts and creates object tree */
typedef lwm2m_map *(lwm2m_create_objects_callback(void));

/////////////////// CONTEXT /////////////////////////

typedef struct lwm2m_context {
    lwm2m_map *servers; // shortServerId -> server
    lwm2m_map *server_addresses; // "localhost:234" -> server

    lwm2m_map *object_tree;
    lwm2m_state state;
    int has_smartcard;
    int is_bootstrap_ready;
    int is_bootstrapped;

    /* Definitions of objects */
    lwm2m_create_resources_callback *create_standard_resources_callback; // TODO move to function
    lwm2m_create_resources_callback *create_resources_callback;
    lwm2m_create_objects_callback *create_objects_callback;

    /* Used bootstrap to provide client with LWM2M instances */
    int (*factory_bootstrap_callback)(struct lwm2m_context *);
    int (*smartcard_bootstrap_callback)(struct lwm2m_context *);


    char* broker_address;
    char* client_id;
    pthread_mutex_t* register_mutex;
    pthread_mutex_t* bootstrap_mutex;
    pthread_cond_t* register_finished_condition;
    pthread_cond_t* bootstrap_finished_condition;

} lwm2m_context;

typedef struct lwm2m_server {
    int short_server_id;
    lwm2m_context *context;

    // Registration data
    char *endpoint_client_name;
    int lifetime;
    char *lwm2m_version;
    char *binding_mode;
    int sms_number;
    char *objects_and_instances;
} lwm2m_server;


#endif //LYNX_LWM2M_LWM2M_CONTEXT_H
