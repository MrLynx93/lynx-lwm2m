#ifndef LYNX_LWM2M_LWM2M_H
#define LYNX_LWM2M_LWM2M_H

#include "../include/lwm2m_object.h"
#include "../include/lwm2m_device_management.h"

#define SECURITY_OBJECT_ID 0
#define SERVER_OBJECT_ID 1
#define ACCESS_CONTROL_OBJECT_ID 2

typedef struct lwm2m_context lwm2m_context;
typedef struct lwm2m_server lwm2m_server;

typedef lwm2m_resource** (create_resources(int object_id));
typedef lwm2m_object** (create_objects(void));

lwm2m_context* lwm2m_create_context(); // TODO possibly many params
int lwm2m_get_number_of_servers(lwm2m_context* context);



struct lwm2m_server {
    int short_server_id;

    lwm2m_context* context;
    lwm2m_attributes_tree *lwm2m_attributes_tree;

    // Registration data ???? this should be in server object?
    char* endpoint_client_name;
    int lifetime;
    char* lwm2m_version;
    char* binding_mode;
    int sms_number;
    char* objects_and_instances;


};

struct lwm2m_context {
    lwm2m_object_tree* object_tree;
    lwm2m_server** servers;
    int is_bootstrap_ready;

    // definition of supported objects
    create_resources* create_resources_callback;
    create_objects* create_objects_callback;
};

#endif //LYNX_LWM2M_LWM2M_H
