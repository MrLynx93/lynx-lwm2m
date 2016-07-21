#include <stdlib.h>

#include "../include/lwm2m_object.h"
#include "../include/lwm2m_parser.h"
#include "../include/lwm2m.h"


/////////////////// CREATE ////////////////////


lwm2m_object *lwm2m_object_new() {
    return (lwm2m_object *) malloc(sizeof(lwm2m_object));
}

lwm2m_instance *lwm2m_instance_new(lwm2m_object *object) {
    lwm2m_instance *instance = (lwm2m_instance *) malloc(sizeof(lwm2m_instance));

    if (is_standard_object(object->id)) {
        instance->resources = object->context->create_standard_resources_callback(object->id);
    }
    else {
        instance->resources = object->context->create_resources_callback(object->id);
    }
    instance->object = object;
    return instance;
}

lwm2m_instance *lwm2m_instance_new_with_id(lwm2m_object *object, int id) {
    lwm2m_instance *instance = lwm2m_instance_new(object);
    instance->id = id;
    return instance;
}

lwm2m_resource *lwm2m_resource_new(bool multiple) {
    lwm2m_resource *resource = (lwm2m_resource *) malloc(sizeof(lwm2m_resource));
    resource->multiple = multiple;
    return resource;
}

/////////////////// DELETE ////////////////////


void lwm2m_delete_object(lwm2m_object *object) {
    lwm2m_map *object_tree = instance->object->context->object_tree;

    // delete ACO instance associated with object
    lwm2m_map *aco_instances = lwm2m_map_get(object_tree, ACCESS_CONTROL_OBJECT_ID)->object.instances;
    lwm2m_map_remove(aco_instances, object->aco_instance->id);
    free_lwm2m_instance(object->aco_instance);

    // delete all instances in object
    int keys[object->instances->size];
    for (int i = 0, instance_id = keys[i]; i < object->instances->size; i++) {
        lwm2m_instance *instance = lwm2m_map_get(object->instances, instance_id)->instance;
        lwm2m_delete_instance(instance);
    }

    // delete object
    lwm2m_map_remove(object_tree, object->id);
    free_lwm2m_object(object);
}


void lwm2m_delete_instance(lwm2m_instance *instance) {
    // if instance is not ACO instance, then delete associated ACO instance
    if (instance->object->id != ACCESS_CONTROL_OBJECT_ID) {
        lwm2m_map *object_tree = instance->object->context->object_tree;
        lwm2m_map *aco_instances = lwm2m_map_get(object_tree, ACCESS_CONTROL_OBJECT_ID)->object.instances;
        lwm2m_map_remove(aco_instances, instance->aco_instance->id);
        free_lwm2m_instance(instance->aco_instance);
    }

    // delete all resources in instance
    int keys[instance->resources->size];
    for (int i = 0, resource_id = keys[i]; i < instance->resources->size; i++) {
        lwm2m_resource *resource = lwm2m_map_get(instance->resources, resource_id)->resource;
        free_lwm2m_resource(resource);
    }

    // delete instance
    lwm2m_map_remove(instance->object->instances, instance->id);
    free_lwm2m_instance(instance);
}

/////////////////// FREE ////////////////////


static void free_lwm2m_object(lwm2m_object *object) {
    free(object->object_urn);
    free(object->instances);
    free(object);
}

static void free_lwm2m_instance(lwm2m_instance *instance) {
    free(instance->resources);
    free(instance);
}

void free_lwm2m_resource(lwm2m_resource *resource) {
    free(resource->name);
    free(resource);
}

static bool is_standard_object(int object_id) {
    return
            object_id == SERVER_OBJECT_ID ||
            object_id == SECURITY_OBJECT_ID ||
            object_id == ACCESS_CONTROL_OBJECT_ID;
}
