#ifndef LYNX_LWM2M_LWM2M_H
#define LYNX_LWM2M_LWM2M_H

typedef struct lwm2m_context lwm2m_context;
typedef struct lwm2m_server lwm2m_server;

typedef lwm2m_resource** (create_resources(int object_id));

lwm2m_context* lwm2m_create_context(); // TODO possibly many params
int lwm2m_get_number_of_servers(lwm2m_context* context);

struct lwm2m_server {
    int shortId;

    lwm2m_context* context;
    lwm2m_object_tree* object_tree;
    lwm2m_attributes_tree *lwm2m_attributes_tree;

    // Registration data
    char* endpoint_client_name;
    int lifetime;
    char* lwm2m_version;
    char* binding_mode;
    int sms_number;
    char* objects_and_instances;


} lwm2m_server;

struct lwm2m_context {
    lwm2m_server* server;
    int is_bootstrap_ready;

    // definition of supported objects
    create_resources create_resources_callback;
    int default_acl_values;
};

#endif //LYNX_LWM2M_LWM2M_H
