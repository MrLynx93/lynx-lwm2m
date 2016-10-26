#include "lwm2m_object.h"
#include "lwm2m.h"
#include "map.h"


///////////// FREE MEMORY /////////////////

static void free_lwm2m_object(lwm2m_object *object) {
    free(object->object_urn);
    free(object->instances);
    free(object);
}

static void free_lwm2m_instance(lwm2m_instance *instance) {
    free(instance->resources);
    free(instance);
}

static void free_lwm2m_resource(lwm2m_resource *resource) {
    free(resource->name);
    free(resource);
}

static bool is_standard_object(int object_id) {
    return
            object_id == SERVER_OBJECT_ID ||
            object_id == SECURITY_OBJECT_ID ||
            object_id == ACCESS_CONTROL_OBJECT_ID;
}

////////////// ATTRIBUTE //////////////////////

lwm2m_attribute *new_int_attribute(char* name, int int_value, int access_mode) {
    lwm2m_value value;
    value.int_value = int_value;

    lwm2m_attribute *attribute = (lwm2m_attribute*) malloc(sizeof(lwm2m_attribute));
    attribute->numeric_value = value;
    attribute->access_mode = access_mode;
    attribute->name = name;
    attribute->type = INTEGER;
    return attribute;
}

lwm2m_type lwm2m_get_attribute_type(char *attribute_name) {
    // TODO implement
}

bool is_notify_attribute(char* attribute_name) {
    // TODO implement
}

////////////// LWM2M OBJECT //////////////////////

void lwm2m_delete_object(lwm2m_object *object) {
    lwm2m_map *object_tree = object->context->object_tree;

    // delete ACO instance associated with object
    lwm2m_map *aco_instances = ((lwm2m_object *) lwm2m_map_get(object_tree, ACCESS_CONTROL_OBJECT_ID))->instances;
    lwm2m_map_remove(aco_instances, object->aco_instance->id);
    free_lwm2m_instance(object->aco_instance);

    // delete all instances in object
    int keys[object->instances->size];
    for (int i = 0, instance_id = keys[i]; i < object->instances->size; i++) {
        lwm2m_instance *instance = (lwm2m_instance *) lwm2m_map_get(object->instances, instance_id);
        lwm2m_delete_instance(instance);
    }

    // delete object
    lwm2m_map_remove(object_tree, object->id);
    free_lwm2m_object(object);
}

lwm2m_object *lwm2m_object_new() {
    return (lwm2m_object *) malloc(sizeof(lwm2m_object));
}


////////////// LWM2M INSTANCE //////////////////////

lwm2m_instance *lwm2m_instance_new_with_id(lwm2m_object *object, int id) {
    lwm2m_instance *instance = (lwm2m_instance *) malloc(sizeof(lwm2m_instance));
    instance->id = id;

    if (is_standard_object(object->id)) {
        instance->resources = object->context->create_standard_resources_callback(object->id);
    }
    else {
        instance->resources = object->context->create_resources_callback(object->id);
    }
    instance->object = object;
    lwm2m_map_put(object->instances, instance->id, instance);
    return instance;
}

lwm2m_instance *lwm2m_instance_new(lwm2m_object *object) {
    int newId = 6; // TODO
    return lwm2m_instance_new_with_id(object, newId);
}

void lwm2m_delete_instance(lwm2m_instance *instance) {
    // if instance is not ACO instance, then delete associated ACO instance
    if (instance->object->id != ACCESS_CONTROL_OBJECT_ID) {
        lwm2m_map *object_tree = instance->object->context->object_tree;
        lwm2m_map *aco_instances = ((lwm2m_object *) lwm2m_map_get(object_tree, ACCESS_CONTROL_OBJECT_ID))->instances;
        lwm2m_map_remove(aco_instances, instance->aco_instance->id);
        free_lwm2m_instance(instance->aco_instance);
    }

    // delete all resources in instance
    int keys[instance->resources->size];
    for (int i = 0, resource_id = keys[i]; i < instance->resources->size; i++) {
        lwm2m_resource *resource = (lwm2m_resource *) lwm2m_map_get(instance->resources, resource_id);
        free_lwm2m_resource(resource);
    }

    // delete instance
    lwm2m_map_remove(instance->object->instances, instance->id);
    free_lwm2m_instance(instance);
}


////////////// LWM2M RESOURCE //////////////////////

lwm2m_resource *lwm2m_resource_new(bool multiple) {
    lwm2m_resource *resource = (lwm2m_resource *) malloc(sizeof(lwm2m_resource));
    resource->multiple = multiple;
    return resource;
}


/////////////// MAP UTILITY FUNCTIONS ////////////////

lwm2m_resource *lwm2m_map_get_resource(lwm2m_map *map, int key) {
    return (lwm2m_resource*) lwm2m_map_get(map, key);
}

lwm2m_instance *lwm2m_map_get_instance(lwm2m_map *map, int key) {
    return (lwm2m_instance *) lwm2m_map_get(map, key);
}

lwm2m_object *lwm2m_map_get_object(lwm2m_map *map, int key) {
    return (lwm2m_object *) lwm2m_map_get(map, key);
}

lwm2m_attribute *lwm2m_map_get_attribute(lwm2m_map *map, char *key) {
    return (lwm2m_attribute *) lwm2m_map_get_string(map, key);
}