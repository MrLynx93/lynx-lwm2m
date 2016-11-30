#ifndef LYNX_LWM2M_LWM2M_CONTEXT_H
#define LYNX_LWM2M_LWM2M_CONTEXT_H

#define _XOPEN_SOURCE 600

#include "map.h"
#include "list.h"
#include "scheduler.h"
#include "lwm2m_object.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <paho/MQTTClient.h>
#include <paho/MQTTAsync.h>

#define NOOP    0
#define READ    1 << 0
#define WRITE   1 << 1
#define EXECUTE 1 << 2
#define DELETE  1 << 3
#define CREATE  1 << 4

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
typedef list *(lwm2m_create_resources_callback(int object_id));

/* Used to define custom LWM2M objects. Called when client starts and creates object tree */
typedef list *(lwm2m_create_objects_callback(void));

/////////////////// CONTEXT /////////////////////////

typedef struct lwm2m_register_data {
    char *endpoint_client_name;
    char *binding_mode;
    int lifetime;
    char *object_and_instances;
} lwm2m_register_data;

typedef struct lwm2m_context {
    list *servers; // shortServerId -> server
//    lwm2m_map *server_addresses; // "localhost:234" -> server

    list *object_tree;
    lwm2m_state state;
    int has_smartcard;
    int is_bootstrap_ready;
    int is_bootstrapped;


    list *resources; // Its list[resource[]] - resource definitions for objects

    /* Definitions of objects */
//    lwm2m_create_resources_callback *create_standard_resources_callback; // TODO move to function
//    lwm2m_create_resources_callback *create_resources_callback;
    lwm2m_create_objects_callback *create_objects_callback;

    /* Used bootstrap to provide client with LWM2M instances */
    int (*factory_bootstrap_callback)(struct lwm2m_context *);
    int (*smartcard_bootstrap_callback)(struct lwm2m_context *);


    char* broker_address;
    char* client_id;
    char* endpoint_client_name;

    lwm2m_scheduler *scheduler;

    MQTTAsync *mqtt_client;

    pthread_mutex_t register_mutex;
    pthread_cond_t register_finished_condition;
    int registered_servers_num;

    list *update_tasks;

    pthread_mutex_t bootstrap_mutex;
    pthread_cond_t bootstrap_finished_condition;

} lwm2m_context;

typedef struct lwm2m_server {
    int short_server_id;
    lwm2m_context *context;
    lwm2m_register_data last_update_data;
    lwm2m_instance *server_instance;

} lwm2m_server;

typedef struct execute_param {
    int key;
    int int_value;
    float float_value;
    char *string_value;
} execute_param;

execute_param *param_new();

#endif //LYNX_LWM2M_LWM2M_CONTEXT_H
