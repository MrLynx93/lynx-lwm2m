#include <lwm2m_information_reporting.h>
#include <lwm2m_attribute.h>
#include <lwm2m_access_control.h>

static bool __fulfill_PMIN(int pmin, scheduler_task *task) {
    if (task == NULL) {
        return false;
    }
    if (task->last_waking_time == 0) {
        return true;
    }
    double diff = difftime(time(0), task->last_waking_time);
    return pmin < diff;
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
    resource->length = (int) strlen(value);
    resource->value->string_value = malloc((size_t) (resource->length + 1));
    strcpy(resource->value->string_value, value);
    return old_value;
}

lwm2m_value *__set_value_opaque(lwm2m_resource *resource, char *value, int length) {
    lwm2m_value *old_value = resource->value;
    resource->value = (lwm2m_value *) malloc(sizeof(lwm2m_value));
    resource->value->opaque_value = malloc(sizeof(char) * length) + 1;
    resource->length = length;
    memcpy(resource->value->opaque_value, value, length);
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
    return NULL;
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
        int *pmin;

        /**** Notify on resource level ****/
        task = lfind(resource->observers, server->short_server_id);
        if (task != NULL) {
            pmin = get_resource_pmin(server, resource, LOOKUP_FULL);
            if (pmin == NULL || __fulfill_PMIN(*pmin, task)) {
                execute(context->scheduler, task, NULL);
            }
        }

        /**** Notify on object level ****/
        task = lfind(resource->instance->object->observers, server->short_server_id);
        if (task != NULL) {
            pmin = get_object_pmin(server, resource->instance->object, LOOKUP_FULL);
            if (pmin == NULL || __fulfill_PMIN(*pmin, task)) {
                execute(context->scheduler, task, resource->instance);
            }
        }
        /**** Notify on instance level ****/
        task = lfind(resource->instance->observers, server->short_server_id);
        if (task != NULL) {
            pmin = get_instance_pmin(server, resource->instance, LOOKUP_FULL);
            if (pmin == NULL || __fulfill_PMIN(*pmin, task)) {
                execute(context->scheduler, task, NULL);
            }
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

static void __free_value(lwm2m_value *value, lwm2m_type type) {
    if (type == STRING || type == OPAQUE) {
        free(value->string_value);
    }
    free(value);
}

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
        list *old_resource_instances = old_resource->instances;
        list *new_resource_instances = new_resource->instances;

        /**** For each resource instance in multiple resource ****/
        for (list_elem *elem = new_resource_instances->first; elem != NULL; elem = elem->next) {
            lwm2m_resource *new_instance = elem->value;
            lwm2m_resource *old_instance = lfind(old_resource_instances, elem->key);

            /**** Add new instance OR setting NULL should cause notify ****/
            if (old_instance == NULL || new_resource->value == NULL) {
                /**** Added new resource instance. Should notify both resource and instance *****/
                if (old_instance == NULL) {
                    ladd(old_resource_instances, new_instance->id, new_instance);
                }
                /**** Set NULL on some existing resource instance ****/
                else {
                    lwm2m_value *old_value = new_resource->value == NULL
                                             ? __set_null(old_instance)
                                             : __set_value(old_instance, new_resource->value, new_resource->length);
                    if (old_value != NULL) {
                        __free_value(old_value, old_resource->type);
                    }
                }
                // TODO what if resource have some value? should we override here?

                /**** Add all servers to notify ****/
                if (notify) {
                    for (list_elem *elem_s = context->servers->first; elem_s != NULL; elem_s = elem_s->next) {
                        lwm2m_server *server = elem_s->value;
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
                    for (list_elem *elem_s = servers->first; elem_s != NULL; elem_s = elem_s->next) {
                        lwm2m_server *server = elem_s->value;
                        if (!lcontains(servers_to_notify, server->short_server_id)) {
                            ladd(servers_to_notify, server->short_server_id, server);
                        }
                    }
                }
                free(old_value);
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
            list_free(servers);
        }
        if (old_value != NULL) {
            __free_value(old_value, old_resource->type);
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
            scheduler_task *task = lfind(old_resource->observers, server->short_server_id);

            if (task != NULL) {
                int *pmin = get_resource_pmin(server, old_resource, LOOKUP_FULL);
                if (pmin == NULL || __fulfill_PMIN(*pmin, task)) {
                    execute(context->scheduler, task, NULL);
                }
            }
        }
    }
    return servers_to_notify;
}

// TODO move to another file?
void notify_instance_object(lwm2m_context *context, lwm2m_instance *instance, list *servers) {
    for (list_elem *elem = servers->first; elem != NULL; elem = elem->next) {
        lwm2m_server *server = elem->value;
        scheduler_task *task;
        int *pmin;

        /**** Notify on object level ****/
        task = lfind(instance->object->observers, server->short_server_id);
        if (task != NULL) {
            pmin = get_object_pmin(server, instance->object, LOOKUP_FULL);
            if (pmin == NULL || __fulfill_PMIN(*pmin, task)) {
                execute(context->scheduler, task, instance);
            }
        }

        /**** Notify on instance level ****/
        task = lfind(instance->observers, server->short_server_id);
        if (task != NULL) {
            pmin = get_instance_pmin(server, instance, LOOKUP_FULL);
            if (pmin == NULL || __fulfill_PMIN(*pmin, task)) {
                execute(context->scheduler, task, NULL);
            }
        }
    }
}

void merge_resources(lwm2m_instance *old_instance, lwm2m_instance *new_instance, bool call_callback, bool notify) {
    list *servers_to_notify = notify ? list_new() : NULL;
    lwm2m_context *context = old_instance->object->context;

    for (list_elem *elem = new_instance->resources->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *old_resource = lfind(old_instance->resources, elem->key);
        lwm2m_resource *new_resource = elem->value;
        list *servers = merge_resource(old_resource, new_resource, call_callback, notify);

        /**** Add to list of servers to notify ****/
        if (notify) {
            for (list_elem *elem_s = servers->first; elem_s != NULL; elem_s = elem_s->next) {
                if (!lcontains(servers_to_notify, elem_s->key)) {
                    ladd(servers_to_notify, elem_s->key, elem_s->value);
                }
            }
            list_free(servers);
        }
    }
    if (notify) {
        notify_instance_object(context, old_instance, servers_to_notify);
        list_free(servers_to_notify);
    }
}

///////////// FREE MEMORY /////////////////

static void free_lwm2m_instance(lwm2m_instance *instance) {
    list_free(instance->resources); // TODO should I use free_resource here?
    list_free(instance->observers);
    free(instance);
}

// TODO ONE FREE TO RULE THEM ALL??
static void free_lwm2m_resource(lwm2m_resource *resource) {
    list_free(resource->observers); // TODO CANCEL ALL OBSERVE
    if (resource->multiple) {
        list_free(resource->instances); // TODO FREE ALL RESOURCE INSTANCES
    }
    free(resource);
}

lwm2m_object *lwm2m_object_new() {
    lwm2m_object *object = (lwm2m_object *) malloc(sizeof(lwm2m_object));
    object->attributes = list_new();
    object->observers = list_new();
    return object;
}


////////////// LWM2M INSTANCE //////////////////////

static list *__create_resources_from_def(lwm2m_object *object, lwm2m_instance *instance) {
    list *resources = list_new();
    for (int id = 0; id < object->resource_def_len; ++id) {
        lwm2m_resource *resource = (lwm2m_resource *) malloc(sizeof(lwm2m_resource));
        memcpy(resource, object->resource_def + id, sizeof(lwm2m_resource));
        resource->instances = resource->multiple ? list_new() : NULL;
        resource->instance = instance;
        resource->attributes = list_new();
        resource->observers = list_new();
        resource->value = NULL;
        resource->length = 0;
        ladd(resources, id, resource);
    }
    return resources;
}

/**
 * - alloc instance
 * - create resources
 * - assign id
 * - assign resource.instance_id
 *
 */
lwm2m_instance *lwm2m_instance_new_with_id(lwm2m_object *object, int instance_id) {
    lwm2m_instance *instance = (lwm2m_instance *) malloc(sizeof(lwm2m_instance));
    instance->id = instance_id;
    instance->object = object;
    instance->attributes = list_new();
    instance->observers = list_new();
    instance->resources = __create_resources_from_def(object, instance);
    ladd(object->instances, instance->id, instance);
    return instance;
}

lwm2m_instance *lwm2m_instance_new(lwm2m_object *object) {
    int newId = object->instances->greatest_key + 1;
    return lwm2m_instance_new_with_id(object, newId);
}

void lwm2m_delete_instance(lwm2m_instance *instance) {
    /**** If instance is not ACO instance, then delete associated ACO instance ****/
    /**** If ACO instance exists, then remove (it may not exist if there is only one server??) ****/
    if (instance->object->id != ACCESS_CONTROL_OBJECT_ID && aco_for_instance(instance) != NULL) {
        list *object_tree = instance->object->context->object_tree;
        list *aco_instances = ((lwm2m_object *) lfind(object_tree, ACCESS_CONTROL_OBJECT_ID))->instances;
        lwm2m_instance *aco_instance = aco_for_instance(instance);
        lremove(aco_instances, aco_instance->id);
        free_lwm2m_instance(aco_instance);
    }

    // delete all resources in instance
    for (list_elem *elem = instance->resources->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *resource = elem->value;
        free_lwm2m_resource(resource);
    }

    // delete instance
    lremove(instance->object->instances, instance->id);
    free_lwm2m_instance(instance);
}


////////////// LWM2M RESOURCE //////////////////////



lwm2m_resource *lwm2m_resource_new(bool multiple) {
    lwm2m_resource *resource = (lwm2m_resource *) malloc(sizeof(lwm2m_resource));
    resource->instances = multiple ? list_new() : NULL;
    resource->attributes = list_new();
    resource->observers = list_new();
    resource->value = NULL;
    resource->length = 0;
    resource->multiple = multiple;
    resource->execute_callback = NULL;
    resource->write_callback = NULL;
    resource->read_callback = NULL;
    return resource;
}