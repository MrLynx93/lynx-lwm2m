#ifndef LYNX_LWM2M_LWM2M_OBJECT_H
#define LYNX_LWM2M_LWM2M_OBJECT_H

#include <stdbool.h>


#define ATTR_READ 1
#define ATTR_WRITE 1
#define ATTR_DELETE 1

// Security object
#define SECURITY_OBJECT_ID 0
#define SERVER_URI_RESOURCE_ID 0

// Server object
#define SERVER_OBJECT_ID 1
#define SHORT_SERVER_ID_RESOURCE_ID 0
#define LIFETIME_RESOURCE_ID 1
#define BINDING_RESOURCE_ID 7

// Access control object
#define ACCESS_CONTROL_OBJECT_ID 2
#define OBJECT_ID_RESOURCE_ID 0
#define INSTANCE_ID_RESOURCE_ID 1
#define ACL_RESOURCE_ID 2
#define ACO_RESOURCE_ID 3

/////////////// TYPES /////////////////////////

typedef struct lwm2m_object lwm2m_object;
typedef struct lwm2m_instance lwm2m_instance;
typedef struct lwm2m_resource lwm2m_resource;
typedef struct lwm2m_context lwm2m_context;

typedef enum lwm2m_type {
    INTEGER,
    DOUBLE,
    STRING,
    OPAQUE,
    BOOLEAN,
    NONE,
    LINK
} lwm2m_type;

typedef struct lwm2m_link {
    int object_id;
    int instance_id;
} lwm2m_link;

typedef union lwm2m_value {
    double double_value;
    int int_value;
    bool bool_value;
    lwm2m_link link_value;
    char* opaque_value;
    char* string_value;
} lwm2m_value;


////////////// ATTRIBUTE //////////////////////

list *merge_resource(lwm2m_resource *old_resource, lwm2m_resource *new_resource, bool call_callback, bool notify);

void merge_resources(lwm2m_instance *old_instance, lwm2m_instance *new_instance, bool call_callback, bool notify);

void notify_instance_object(lwm2m_context *context, lwm2m_instance *instance, list *servers);



/////////////// CALLBACKS ///////////////////

/* This callback is executed BEFORE read operation is performed to update value inside resource */
typedef void (lwm2m_resource_read_callback)(lwm2m_resource *resource);

/* This callback is executed AFTER write operation to perform any action due to changing value */
typedef void (lwm2m_resource_write_callback(lwm2m_resource *resource));

// TODO
typedef void (lwm2m_resource_execute_callback(lwm2m_resource *resource, list *args));

////////////// LWM2M OBJECT //////////////////////

struct lwm2m_object {
    int id;
    lwm2m_context *context;
    list *instances;
    list *attributes;
    char *object_urn;
    bool multiple;
    bool mandatory;
    list *observers;

    int resource_def_len;
    lwm2m_resource *resource_def; // definitions of resources
};

/* Constructor */
lwm2m_object *lwm2m_object_new();

/* Removes object from tree together with instances and access control instance. This can be called only in bootstrap interface */
void lwm2m_delete_object(lwm2m_object *object);


////////////// LWM2M INSTANCE //////////////////////

struct lwm2m_instance {
    int id;
    lwm2m_object *object;
    list *resources;
    list *attributes;
    list *observers;
};

/* Constructor */
lwm2m_instance *lwm2m_instance_new(lwm2m_object *object);

/* Constructor */
lwm2m_instance *lwm2m_instance_new_with_id(lwm2m_object *object, int instance_id);

/* Removes instance from object together with resources and access control instance */
void lwm2m_delete_instance(lwm2m_instance *instance);

lwm2m_instance *refer_link(lwm2m_link link);

////////////// LWM2M RESOURCE //////////////////////

struct lwm2m_resource {
    int id;
    lwm2m_instance *instance;

    lwm2m_value *value;
    lwm2m_type type;
    int length;
    list *instances;
    list *observers; // Map of shortServerId -> notify_task

    char *name;
    list *attributes; // TODO Map of serverId -> struct lwm2m_attributes
    int operations;
    bool multiple;
    int mandatory;
    lwm2m_resource_read_callback *read_callback;
    lwm2m_resource_write_callback *write_callback;
    lwm2m_resource_execute_callback *execute_callback;
};

/* Constructor */
lwm2m_resource *lwm2m_resource_new(bool multiple);

/**
 * These functions just save value and length. They do NOT
 * start notifying new values to servers.
 *
 * Should be used only internally
 *
 *
 * return old value as lwm2m_value* pointer
 *
 */
lwm2m_value *__set_value_int(lwm2m_resource *resource, int value);

lwm2m_value *__set_value_bool(lwm2m_resource *resource, bool value);

lwm2m_value *__set_value_double(lwm2m_resource *resource, double value);

lwm2m_value *__set_value_link(lwm2m_resource *resource, lwm2m_link value);

lwm2m_value *__set_value_string(lwm2m_resource *resource, char *value);

lwm2m_value *__set_value_opaque(lwm2m_resource *resource, char *value, int length);

lwm2m_value *__set_value(lwm2m_resource *resource, lwm2m_value *value, int length);

lwm2m_value *__set_null(lwm2m_resource *resource);


/**
 * These functions not only save value and length. Also,
 * they notify servers about change if conditions are met:
 * - time since last update < pmin
 * - gt, lt, step conditions are met
 *
 * They should be used when status of some analog device
 * has changed (ex. Light on/off)
 *
 *
 * return true if parent should notify too
 */
void set_value_int(lwm2m_resource *resource, int value);
void set_value_bool(lwm2m_resource *resource, bool value);
void set_value_double(lwm2m_resource *resource, double value);
void set_value_link(lwm2m_resource *resource, lwm2m_link value);
void set_value_string(lwm2m_resource *resource, char *value);
void set_value_opaque(lwm2m_resource *resource, char *value, int length);
void set_value(lwm2m_resource *resource, lwm2m_value *value, int length);
void set_null(lwm2m_resource *resource);


/////////////// NODE /////////////////// TOdO REMOVE THIS?

typedef union lwm2m_node {
    lwm2m_object object;
    lwm2m_instance instance;
    lwm2m_resource resource;
} lwm2m_node;

typedef enum lwm2m_node_type {
    OBJECT,
    INSTANCE,
    RESOURCE,
    RESOURCE_INSTANCE
} lwm2m_node_type;

#endif //LYNX_LWM2M_LWM2M_OBJECT_H
