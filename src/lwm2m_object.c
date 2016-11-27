#include <lwm2m_information_reporting.h>
#include "lwm2m_object.h"
#include "lwm2m.h"
#include "lwm2m_attribute.h"


static void __set_value_any(lwm2m_resource *resource, lwm2m_value *value, int length, lwm2m_type type) {
    lwm2m_value *old_value = resource->value;
    if (resource->value == NULL) {
        resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    }
    *resource->value = *value;
    resource->length = length;


}

/****************** SETTING VALUE WITHOUT NOTIFY **************************/
/**************************************************************************/

lwm2m_value *__set_value_int(lwm2m_resource *resource, int value) {
    lwm2m_value *old_value = resource->value;
    resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    resource->value->int_value = value;
    resource->length = 0;
    return old_value;
}

lwm2m_value *__set_value_double(lwm2m_resource *resource, double value) {
    lwm2m_value *old_value = resource->value;
    resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    resource->value->double_value = value;
    resource->length = 0;
    return old_value;
}

lwm2m_value *__set_value_bool(lwm2m_resource *resource, bool value) {
    lwm2m_value *old_value = resource->value;
    resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    resource->value->bool_value = value;
    resource->length = 0;
    return old_value;
}

lwm2m_value *__set_value_link(lwm2m_resource *resource, lwm2m_link value) {
    lwm2m_value *old_value = resource->value;
    resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    resource->value->link_value = value;
    resource->length = 0;
    return old_value;
}

lwm2m_value *__set_value_string(lwm2m_resource *resource, char *value) {
    lwm2m_value *old_value = resource->value;
    resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    resource->value->string_value = value;
    resource->length = (int) strlen(value);
    return old_value;
}

lwm2m_value *__set_value_opaque(lwm2m_resource *resource, char *value, int length) {
    lwm2m_value *old_value = resource->value;
    resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    resource->value->string_value = value;
    resource->length = length;
    return old_value;
}

lwm2m_value *__set_value(lwm2m_resource *resource, lwm2m_value *value, int length) {
    if (value == NULL) {
        return __set_null(resource);
    }
    switch (resource->type) {
        case INTEGER:
            return __set_value_int(resource, value->int_value);
        case DOUBLE:
            return __set_value_double(resource, value->double_value);
        case BOOLEAN:
            return __set_value_bool(resource, value->bool_value);
        case LINK:
            return __set_value_link(resource, value->link_value);
        case STRING:
            return __set_value_string(resource, value->string_value);
        case OPAQUE:
            return __set_value_opaque(resource, value->opaque_value, length);
        case NONE:
            break;
    }
}

lwm2m_value *__set_null(lwm2m_resource *resource) {
    lwm2m_value *old_value = resource->value;
    resource->value = NULL;
    resource->length = 0;
    return old_value;
}

/****************** SETTING VALUE WITH NOTIFY *****************************/
/**************************************************************************/

void set_value(lwm2m_resource *resource, lwm2m_value *value, int length) {
    lwm2m_value *old_value = __set_value(resource, value, length);

    lwm2m_context *context = resource->instance->object->context;
    list *servers = should_notify(resource, old_value, value);
    for (list_elem *elem = servers->first; elem != NULL; elem = elem->next) {
        lwm2m_server *server = (lwm2m_server*) elem->value;
        scheduler_task *task;

        /**** Notify on resource level ****/
        task = lwm2m_map_get(resource->observers, server->short_server_id);
        if (task != NULL) {
            execute(context->scheduler, task);
        }

        /**** Notify on instance level ****/
        task = lwm2m_map_get(resource->instance->observers, server->short_server_id);
        if (task != NULL) {
            execute(context->scheduler, task);
        }

        /**** Notify on object level ****/
        task = lwm2m_map_get(resource->instance->object->observers, server->short_server_id);
        if (task != NULL) {
            execute(context->scheduler, task);
        }
    }
}

void set_value_int(lwm2m_resource *resource, int value) {
    lwm2m_value new_value = {.int_value = value};
    set_value(resource, &new_value, 0);
}

void set_value_bool(lwm2m_resource *resource, bool value) {
    lwm2m_value new_value = {.bool_value = value};
    set_value(resource, &new_value, 0);
}

void set_value_double(lwm2m_resource *resource, double value) {
    lwm2m_value new_value = {.double_value = value};
    set_value(resource, &new_value, 0);
}

void set_value_link(lwm2m_resource *resource, lwm2m_link value) {
    lwm2m_value new_value;
    new_value.link_value = value;
    set_value(resource, &new_value, 0);
}

void set_value_string(lwm2m_resource *resource, char *value) {
    lwm2m_value new_value;
    new_value.string_value = value;
    set_value(resource, &new_value, 0);
}

void set_value_opaque(lwm2m_resource *resource, char *value, int length) {
    lwm2m_value new_value;
    new_value.opaque_value = value;
    set_value(resource, &new_value, length);
}

void set_null(lwm2m_resource *resource) {
    __set_null(resource);
}

/************************** MERGE RESOURCES *******************************/
/**************************************************************************/

static bool __fulfill_PMIN(int pmin, scheduler_task *task) {
    return task->last_waking_time == 0 || pmin > difftime(time(0), task->last_waking_time);
}

// new_resource is parsed one. It's convenient to just copy values and free the old ones

/****
 * if notify then notify on resource level
 * return list of servers, that should be notified at instance-level
 *
 *
 */
// TODO free old_instance value ????
list *merge_resource(lwm2m_resource *old_resource, lwm2m_resource *new_resource, bool call_callback, bool notify) {
    lwm2m_context *context = old_resource->instance->object->context;
    list *servers_to_notify = notify ? list_new() : NULL;

    /**** Multiple resource *****/
    if (old_resource->multiple) {
        lwm2m_map *old_resource_instances = old_resource->instances;
        lwm2m_map *new_resource_instances = new_resource->instances;

        /**** For each resource instance in multiple resource ****/
        int keys[new_resource_instances->size];
        lwm2m_map_get_keys(new_resource_instances, keys);
        for (int i = 0; i < new_resource_instances->size; ++i) {
            lwm2m_resource *new_instance = lwm2m_map_get_resource(new_resource_instances, keys[i]);
            lwm2m_resource *old_instance = lwm2m_map_get_resource(old_resource_instances, keys[i]);

            /**** Add new instance OR setting NULL should cause notify ****/
            if (old_instance == NULL || new_resource->value == NULL) {
                /**** Added new resource instance. Should notify both resource and instance *****/
                if (old_instance == NULL) {
                    lwm2m_map_put(old_resource_instances, new_instance->id, new_instance);
                }
                /**** Set NULL on some existing resource instance ****/
                if (new_resource->value == NULL) {
                    __set_null(old_instance);
                }

                /**** Add all servers to notify ****/
                if (notify) {
                    int serv_keys[context->servers->size];
                    lwm2m_map_get_keys(context->servers, serv_keys);
                    for (int j = 0; j < context->servers->size; ++j) {
                        lwm2m_server *server = lwm2m_map_get(context->servers, serv_keys[i]);
                        if (!lcontains(servers_to_notify, server->short_server_id)) {
                            ladd(servers_to_notify, server->short_server_id, server);
                        }
                    }
                }
            } 
            /**** Change resource instance should have notify checking as normal resource ****/
            else {
                /**** Set new value on some existing resource instance ****/
                lwm2m_value *old_value = __set_value(old_instance, new_instance->value, new_instance->length);
                
                /**** Adding servers that passed numeric observing conditions ****/
                if (notify) {
                    list *servers = should_notify(old_resource, old_value, new_instance->value);
                    for (list_elem *elem = servers->first; elem != NULL; elem = elem->next) {
                        lwm2m_server *server = elem->value;
                        if (!lcontains(servers_to_notify, server->short_server_id)) {
                            ladd(servers_to_notify, server->short_server_id, server);
                        }
                    }
                }
            }
        }
    } 
    /**** Single resource *****/
    else {
        lwm2m_value *old_value = new_resource->value == NULL
                    ? __set_null(old_resource)
                    : __set_value(old_resource, new_resource->value, new_resource->length);

        /**** Adding servers that passed numeric observing conditions ****/
        if (notify) {
            list *servers = should_notify(old_resource, old_value, new_resource->value);
            for (list_elem *elem = servers->first; elem != NULL; elem = elem->next) {
                lwm2m_server *server = elem->value;
                if (!lcontains(servers_to_notify, server->short_server_id)) {
                    ladd(servers_to_notify, server->short_server_id, server);
                }
            }
        }
    }
    /**** Call write callback. For example to turn on light when writing to LightOn resource ****/
    if (call_callback && old_resource->write_callback != NULL) {
        old_resource->write_callback(old_resource);
    }
    /**** Notifying on resource level all servers, that pass PMIN condition ****/
    if (notify) {
        for (list_elem *elem = servers_to_notify->first; elem != NULL; elem = elem->next) {
            lwm2m_server *server = elem->value;
            scheduler_task *task = lwm2m_map_get(old_resource->observers, server->short_server_id);

            int *pmin = get_resource_pmin(server, old_resource, LOOKUP_FULL);
            if (pmin == NULL || __fulfill_PMIN(*pmin, task)) {
                execute(context->scheduler, task);
            }
        }
    }
    return servers_to_notify;
}

void merge_resources(lwm2m_instance *old_instance, lwm2m_instance *new_instance, bool call_callback, bool notify) {
    list *servers_to_notify = notify ? list_new() : NULL;
    lwm2m_context *context = old_instance->object->context;

    int keys[new_instance->resources->size];
    lwm2m_map_get_keys(new_instance->resources, keys);
    for (int i = 0; i < new_instance->resources->size; ++i) {
        lwm2m_resource *old_resource = lwm2m_map_get_resource(old_instance->resources, keys[i]);
        lwm2m_resource *new_resource = lwm2m_map_get_resource(new_instance->resources, keys[i]);
        list *servers = merge_resource(old_resource, new_resource, call_callback, notify);

        /**** Add to list of servers to notify ****/
        if (notify) {
            for (list_elem *elem = servers->first; elem != NULL; elem = elem->next) {
                ladd(servers_to_notify, elem->key, elem->value);
            }
        }
    }

    if (notify) {
        for (list_elem *elem = servers_to_notify->first; elem != NULL; elem = elem->next) {
            lwm2m_server *server = elem->value;
            scheduler_task *task;
            int *pmin;

            /**** Notify on object level ****/
            task = lwm2m_map_get(new_instance->object->observers, server->short_server_id);
            pmin = get_object_pmin(server, old_instance->object, LOOKUP_FULL);
            if (pmin == NULL || __fulfill_PMIN(*pmin, task)) {
                task->arg2 = old_instance;
                execute(context->scheduler, task); // TODO OBJECT NOTIFY IS ON INSTANCE LEVEL. ONLY NEED TO CHANGE TOPIC INSTANCE
            }

            /**** Notify on instance level ****/
            task = lwm2m_map_get(new_instance->observers, server->short_server_id);
            pmin = get_instance_pmin(server, old_instance, LOOKUP_FULL);
            if (pmin == NULL || __fulfill_PMIN(*pmin, task)) {
                task->arg2 = old_instance;
                execute(context->scheduler, task); // TODO OBJECT NOTIFY IS ON INSTANCE LEVEL. ONLY NEED TO CHANGE TOPIC INSTANCE
            }
        }
    }
}

//// todo
//void merge_resources(lwm2m_instance *old_instance, lwm2m_map *new_resources, bool call_callback, bool notify) {
//    int keys[new_resources->size];
//    lwm2m_map_get_keys(new_resources, keys);
//    for (int i = 0; i < new_resources->size; ++i) {
//        lwm2m_resource *old_resource = lwm2m_map_get_resource(old_instance->resources, keys[i]);
//        lwm2m_resource *new_resource = lwm2m_map_get_resource(new_resources, keys[i]);
//        merge_resource(old_resource, new_resource, call_callback, notify);
//    }
//}

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

//lwm2m_attribute *new_int_attribute(char* name, int int_value, int access_mode) {
//    lwm2m_value *value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
//    value->int_value = int_value;
//
//    lwm2m_attribute *attribute = (lwm2m_attribute*) malloc(sizeof(lwm2m_attribute));
//    attribute->numeric_value = value;
//    attribute->access_mode = access_mode;
//    attribute->name = name;
//    attribute->type = INTEGER;
//    return attribute;
//}
//
//lwm2m_type lwm2m_get_attribute_type(char *attribute_name) {
//    // TODO implement
//}
//
//bool is_notify_attribute(char* attribute_name) {
//    // TODO implement
//}

////////////// LWM2M OBJECT //////////////////////
// TODO why would you delete object?
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

//lwm2m_attribute *lwm2m_map_get_attribute(lwm2m_map *map, char *key) {
//    return (lwm2m_attribute *) lwm2m_map_get_string(map, key);
//}


// TODO SET VALUE HERE