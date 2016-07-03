#ifndef LYNX_LWM2M_LWM2M_OBJECT_H
#define LYNX_LWM2M_LWM2M_OBJECT_H

#include "lwm2m_common.h"
#include "lwm2m_attributes.h"

#define SECURITY_OBJECT_ID 0
#define SERVER_OBJECT_ID 1
#define ACCESS_CONTROL_OBJECT_ID 2

typedef struct lwm2m_object_tree lwm2m_object_tree;

typedef union lwm2m_node lwm2m_node;
typedef struct lwm2m_object lwm2m_object;
typedef struct lwm2m_instance lwm2m_instance;
typedef struct lwm2m_resource lwm2m_resource;
typedef enum lwm2m_node_type lwm2m_node_type;

typedef struct lwm2m_resource_single lwm2m_resource_single;
typedef struct lwm2m_resource_multiple lwm2m_resource_multiple;
typedef union lwm2m_resource_real lwm2m_resource_real;

lwm2m_object_tree *lwm2m_object_tree_new();
lwm2m_object *lwm2m_object_new();
lwm2m_instance *lwm2m_instance_new(lwm2m_object* object, int id);
lwm2m_resource *lwm2m_resource_new(bool multiple);


/* This callback is executed BEFORE read operation is performed to update value inside resource */
typedef void (*lwm2m_resource_read_callback(lwm2m_resource *resource));

/* This callback is executed AFTER write operation to perform any action due to changing value */
typedef void (*lwm2m_resource_write_callback(lwm2m_resource *resource));


typedef void (*lwm2m_resource_execute_callback(lwm2m_resource *resource, char *args));



/* Removes instance from object */
void lwm2m_delete_instance(lwm2m_instance *instance);

//void lwm2m_delete_server_instances(lwm2m_server *server);

lwm2m_instance *refer_link(lwm2m_link lwm2m_instance);



struct lwm2m_object_tree {
    lwm2m_object **objects;
};

struct lwm2m_link {
    int object_id;
    int instance_id;
};

enum {
    OBJECT,
    INSTANCE,
    RESOURCE
};

union lwm2m_node {
    lwm2m_object object;
    lwm2m_instance instance;
    lwm2m_resource resource;
};

struct lwm2m_object {
    int id;
    bool multiple;
    char* object_urn;
    bool mandatory;
    lwm2m_instance **instances;
    lwm2m_attributes attributes;
    lwm2m_instance *aco_instance;
};

struct lwm2m_instance {
    int id;
    lwm2m_object *object;
    lwm2m_resource **resources;
    lwm2m_attributes attributes;
    lwm2m_instance *aco_instance;
};

struct lwm2m_resource {
    int id;
    char* name;
    lwm2m_instance *instance;
    lwm2m_type type;
    lwm2m_attributes attributes;
    int operations;
    int multiple;
    int mandatory;

    lwm2m_resource_real resource;

    lwm2m_resource_read_callback read_callback;
    lwm2m_resource_write_callback write_callback;
    lwm2m_resource_notify_callback notify_callback;
    lwm2m_resource_execute_callback execute_callback;
};

struct lwm2m_resource_single {
    lwm2m_value value;
};

struct lwm2m_resource_multiple {
    lwm2m_resource_single *instances;
};

union lwm2m_resource_real {
    lwm2m_resource_single single;
    lwm2m_resource_multiple multiple;
};


#endif //LYNX_LWM2M_LWM2M_OBJECT_H
