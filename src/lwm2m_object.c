#include "lwm2m_object.h"
#include "lwm2m.h"
#include "map.h"

void __set_value_int(lwm2m_resource *resource, int value) {
    if (resource->value == NULL) {
        resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    }
    resource->value->int_value = value;
}

void __set_value_bool(lwm2m_resource *resource, bool value) {
    if (resource->value == NULL) {
        resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    }
    resource->value->bool_value = value;
    resource->length = 0;
}

void __set_value_double(lwm2m_resource *resource, double value) {
    if (resource->value == NULL) {
        resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    }
    resource->value->double_value = value;
    resource->length = 0;
}

void __set_value_link(lwm2m_resource *resource, lwm2m_link value) {
    if (resource->value == NULL) {
        resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    }
    resource->value->link_value = value;
    resource->length = 0;
}

void __set_value_string(lwm2m_resource *resource, char *value) {
    if (resource->value == NULL) {
        resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    }
    resource->value->string_value = value;
    resource->length = (int) strlen(value);
}

void __set_value_opaque(lwm2m_resource *resource, char *value, int length) {
    if (resource->value == NULL) {
        resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    }
    resource->value->string_value = value;
    resource->length = length;
}

void __set_value(lwm2m_resource *resource, lwm2m_value value, int length) {
    if (resource->value == NULL) {
        resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    }
    if (resource->type == OPAQUE) {
        resource->value->opaque_value = value.opaque_value;
        resource->length = length;
    }
    else if (resource->type == STRING) {
        resource->value->string_value = value.string_value;
        resource->length = length;
    }
    else {
        *resource->value = value;
        resource->length = 0;
    }
}

void __set_null(lwm2m_resource *resource) {
    if (resource->value != NULL) {
        free(resource->value);
    }
    resource->value = NULL;
    resource->length = 0;
}

// new_resource is parsed one. It's convenient to just copy values and free the old ones
void merge_resource(lwm2m_resource *old_resource, lwm2m_resource *new_resource, bool call_callback) {
    if (old_resource->multiple) {
        lwm2m_map *old_resource_instances = old_resource->instances;
        lwm2m_map *new_resource_instances = new_resource->instances;

        int keys[new_resource_instances->size];
        lwm2m_map_get_keys(new_resource_instances, keys);
        for (int i = 0; i < new_resource_instances->size; ++i) {
            lwm2m_resource *new_instance = lwm2m_map_get_resource(new_resource_instances, keys[i]);
            lwm2m_resource *old_instance = lwm2m_map_get_resource(old_resource_instances, keys[i]);
            if (old_instance == NULL) {
                lwm2m_map_put(old_resource_instances, new_instance->id, new_instance);
            } else {
                if (new_resource->value == NULL) {
                    __set_null(old_instance);
                } else {
                    __set_value(old_instance, *new_instance->value, new_instance->length);
                }
            }
            // TODO free old_instance value ????
        }
    } else {
        if (new_resource->value == NULL) {
            __set_null(old_resource);
        } else {
            __set_value(old_resource, *new_resource->value, new_resource->length);
        }
    }
    if (call_callback && old_resource->write_callback != NULL) {
        old_resource->write_callback(old_resource);
    }
}

void merge_resources(lwm2m_map *old_resources, lwm2m_map *new_resources, bool call_callback) {
    int keys[new_resources->size];
    lwm2m_map_get_keys(new_resources, keys);
    for (int i = 0; i < new_resources->size; ++i) {
        lwm2m_resource *old_resource = lwm2m_map_get_resource(old_resources, keys[i]);
        lwm2m_resource *new_resource = lwm2m_map_get_resource(new_resources, keys[i]);
        merge_resource(old_resource, new_resource, call_callback);
    }
}

///////////// FREE MEMORY /////////////////

static void free_lwm2m_object(lwm2m_object *object) {
//    free(object->object_urn);
    free(object->instances);
    free(object);
}

static void free_lwm2m_instance(lwm2m_instance *instance) {
    free(instance->resources);
    free(instance->observers);
    free(instance);
}

static void free_lwm2m_resource(lwm2m_resource *resource) {
    lwm2m_map_free(resource->observers); // TODO CANCEL ALL OBSERVE
    if (resource->multiple) {
        lwm2m_map_free(resource->instances); // TODO FREE ALL RESOURCE INSTANCES
    }
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
    lwm2m_value *value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    value->int_value = int_value;

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
    lwm2m_object *object = (lwm2m_object *) malloc(sizeof(lwm2m_object));
    object->attributes = lwm2m_map_new();
    object->observers = lwm2m_map_new();
    return object;
}


////////////// LWM2M INSTANCE //////////////////////

/**
 * - alloc instance
 * - create resources
 * - assign id
 * - assign resource.instance_id
 *
 * DOES NOT set instance->object and object->instances
 *
 */
lwm2m_instance *lwm2m_instance_new_with_id(lwm2m_context *context, int object_id, int instance_id) {
    lwm2m_instance *instance = (lwm2m_instance *) malloc(sizeof(lwm2m_instance));
    instance->id = instance_id;
    instance->attributes = lwm2m_map_new();
    instance->observers = lwm2m_map_new();
    instance->resources = is_standard_object(object_id)
                          ? context->create_standard_resources_callback(object_id)
                          : context->create_resources_callback(object_id);

    /**** Set resource->instance ****/
    int keys[instance->resources->size];
    lwm2m_map_get_keys(instance->resources, keys);
    for (int i = 0; i < instance->resources->size; ++i) {
        lwm2m_map_get_resource(instance->resources, keys[i])->instance = instance;
    }
    return instance;
}

lwm2m_instance *lwm2m_instance_new(lwm2m_context *context, int object_id) {
    int newId = 6; // TODO
    return lwm2m_instance_new_with_id(context, object_id, newId);
}

void lwm2m_delete_instance(lwm2m_instance *instance) {
    /**** If instance is not ACO instance, then delete associated ACO instance ****/
    /**** If ACO instance exists, then remove (it may not exist if there is only one server??) ****/
    if (instance->object->id != ACCESS_CONTROL_OBJECT_ID && instance->aco_instance != NULL) {
        lwm2m_map *object_tree = instance->object->context->object_tree;
        lwm2m_map *aco_instances = ((lwm2m_object *) lwm2m_map_get(object_tree, ACCESS_CONTROL_OBJECT_ID))->instances;
        lwm2m_map_remove(aco_instances, instance->aco_instance->id);
        free_lwm2m_instance(instance->aco_instance);
    }

    // delete all resources in instance
    int keys[instance->resources->size];
    lwm2m_map_get_keys(instance->resources, keys);
    for (int i = 0; i < instance->resources->size; i++) {
        lwm2m_resource *resource = (lwm2m_resource *) lwm2m_map_get(instance->resources, keys[i]);
        free_lwm2m_resource(resource);
    }

    // delete instance
    lwm2m_map_remove(instance->object->instances, instance->id);
    free_lwm2m_instance(instance);
}


////////////// LWM2M RESOURCE //////////////////////



lwm2m_resource *lwm2m_resource_new(bool multiple) {
    lwm2m_resource *resource = (lwm2m_resource *) malloc(sizeof(lwm2m_resource));
    resource->instances = multiple ? lwm2m_map_new() : NULL;
    resource->attributes = lwm2m_map_new();
    resource->observers = lwm2m_map_new();
    resource->value = NULL;
    resource->length = 0;
    resource->multiple = multiple;
    resource->execute_callback = NULL;
    resource->write_callback = NULL;
    resource->read_callback = NULL;
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


// TODO SET VALUE HERE