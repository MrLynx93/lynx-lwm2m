#ifndef LYNX_LWM2M_LWM2M_OBJECT_H
#define LYNX_LWM2M_LWM2M_OBJECT_H

#include "map.h"

#include <stdbool.h>


#define ATTR_READ 1
#define ATTR_WRITE 1
#define ATTR_DELETE 1

// Security object
#define SECURITY_OBJECT_ID 0

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

typedef struct lwm2m_attribute {
    char* name;
    int name_len;
    int access_mode;
    lwm2m_type type;
    lwm2m_value* numeric_value;
} lwm2m_attribute;


void merge_resource(lwm2m_resource *old_resource, lwm2m_resource *new_resource, bool call_callback);

void merge_resources(lwm2m_map *old_resources, lwm2m_map *new_resources, bool call_callback);




lwm2m_attribute *new_int_attribute(char* name, int value, int access_mode);

/* Definition of general attribute value types */
lwm2m_type lwm2m_get_attribute_type(char *attribute_name);

/* Definition of general attribute types */
bool is_notify_attribute(char* attribute_name);

/////////////// CALLBACKS ///////////////////

/* This callback is executed BEFORE read operation is performed to update value inside resource */
typedef void (lwm2m_resource_read_callback)(lwm2m_resource *resource);

/* This callback is executed AFTER write operation to perform any action due to changing value */
typedef void (lwm2m_resource_write_callback(lwm2m_resource *resource));

// TODO
typedef void (lwm2m_resource_execute_callback(lwm2m_resource *resource, char *args));

////////////// LWM2M OBJECT //////////////////////

struct lwm2m_object {
    int id;
    lwm2m_context *context;
    lwm2m_map *instances;
    lwm2m_map *attributes;
    lwm2m_instance *aco_instance;
    char *object_urn;
    bool multiple;
    bool mandatory;
    lwm2m_map *observers;
};

/* Constructor */
lwm2m_object *lwm2m_object_new();

/* Removes object from tree together with instances and access control instance. This can be called only in bootstrap interface */
void lwm2m_delete_object(lwm2m_object *object);


////////////// LWM2M INSTANCE //////////////////////

struct lwm2m_instance {
    int id;
    lwm2m_object *object;
    lwm2m_map *resources;
    lwm2m_map *attributes;
    lwm2m_map *observers;
    lwm2m_instance *aco_instance;
};

/* Constructor */
lwm2m_instance *lwm2m_instance_new(lwm2m_context *context, int object_id);

/* Constructor */
lwm2m_instance *lwm2m_instance_new_with_id(lwm2m_context *context, int object_id, int instance_id);

/* Removes instance from object together with resources and access control instance */
void lwm2m_delete_instance(lwm2m_instance *instance);

lwm2m_instance *refer_link(lwm2m_link link);

////////////// LWM2M RESOURCE //////////////////////

typedef struct lwm2m_resource_single {
    lwm2m_value value;
    int length; // for string and opaque
} lwm2m_resource_single;

typedef struct lwm2m_resource_multiple {
    lwm2m_map *instances;
} lwm2m_resource_multiple;

typedef union lwm2m_resource_real {
    lwm2m_resource_single single;
    lwm2m_resource_multiple multiple;
} lwm2m_resource_real;

struct lwm2m_resource {
    int id;
    lwm2m_instance *instance;

    lwm2m_value *value;
    lwm2m_type type;
    int length;
    lwm2m_map *instances;
    lwm2m_map *observers; // Map of shortServerId -> notify_task

    char *name;
    lwm2m_map *attributes;
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
 *
 *
 */
void __set_value_int(lwm2m_resource *resource, int value);

void __set_value_bool(lwm2m_resource *resource, bool value);

void __set_value_double(lwm2m_resource *resource, double value);

void __set_value_link(lwm2m_resource *resource, lwm2m_link value);

void __set_value_string(lwm2m_resource *resource, char *value);

void __set_value_opaque(lwm2m_resource *resource, char *value, int length);

void __set_value(lwm2m_resource *resource, lwm2m_value value, int length);

void __set_null(lwm2m_resource *resource);


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
 */
// TODO implement and save last notify time somwhere (maybe same as in register/update interface?)
//void set_value_int(lwm2m_resource *resource, int value);
//
//void set_value_bool(lwm2m_resource *resource, bool value);
//
//void set_value_double(lwm2m_resource *resource, double value);
//
//void set_value_link(lwm2m_resource *resource, lwm2m_link value);
//
//void set_value_string(lwm2m_resource *resource, char *value);
//
//void set_value_opaque(lwm2m_resource *resource, char *value, int length);
//
//void set_value(lwm2m_resource *resource, lwm2m_value value, int length);
//
//void set_null(lwm2m_resource *resource);


/////////////// NODE ///////////////////

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

/////////////// MAP UTILITY FUNCTIONS ////////////////

lwm2m_resource *lwm2m_map_get_resource(lwm2m_map *map, int key);

lwm2m_instance *lwm2m_map_get_instance(lwm2m_map *map, int key);

lwm2m_object *lwm2m_map_get_object(lwm2m_map *map, int key);

lwm2m_attribute *lwm2m_map_get_attribute(lwm2m_map *map, char *key);


#endif //LYNX_LWM2M_LWM2M_OBJECT_H
