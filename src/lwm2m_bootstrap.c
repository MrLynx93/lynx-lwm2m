#include <lwm2m_transport.h>
#include <lwm2m_access_control.h>
#include <lwm2m_transport_mqtt.h>
#include "lwm2m.h"
#include "lwm2m_bootstrap.h"
#include "lwm2m_parser.h"

static void __free_attribute(lwm2m_attributes *attribute) {
    if (attribute->pmin != NULL) free(attribute->pmin);
    if (attribute->pmax != NULL) free(attribute->pmin);
    if (attribute->dim != NULL) free(attribute->pmin);
    if (attribute->stp != NULL) free(attribute->pmin);
    if (attribute->lt != NULL) free(attribute->pmin);
    if (attribute->gt != NULL) free(attribute->pmin);
    free(attribute);
}

static void __free_attributes(list *attributes) {
    for (list_elem *elem = attributes->first; elem != NULL; elem = elem->next) {
        __free_attribute(elem->value);
    }
    list_free(attributes);
}

static void __free_parsed_resource(lwm2m_resource *resource) {
    if (resource->multiple) {
        if (resource->instances != NULL) {
            for (list_elem *elem = resource->instances->first; elem != NULL; elem = elem->next) {
                lwm2m_resource *instance = elem->value;
                if (instance->value != NULL) {
                    free(instance->value);
                }
                free(instance);
            }
            list_free(resource->instances);
        }
    }
    else if (resource->value != NULL) {
        free(resource->value);
    }
    free(resource);
}

static void __free_parsed_resources(list *resources) {
    for (list_elem *elem = resources->first; elem != NULL; elem = elem->next) {
        __free_parsed_resource(elem->value);
    }
    list_free(resources);
}

static void __free_instance(lwm2m_instance *instance) {
    __free_attributes(instance->attributes);
    __free_parsed_resources(instance->resources);
    list_free(instance->observers);
    free(instance);
}

static void __free_instances(list* instances) {
    for (list_elem *elem = instances->first; elem != NULL; elem = elem->next) {
        __free_instance(elem->value);
    }
}

/****
 *
 * Only in Bootstrap Interface, Delete operation MAY target to “/” URI to delete all

the existing Object Instances - except LWM2M Bootstrap Server Account - in the LWM2M Client, for initialization purpose

before LWM2M Bootstrap Server sends Write operation(s) to the LWM2M Client. Different from „Write“ operation in

Device Management and Service Enablement interface, the LWM2M Client MUST write the value included in the payload

regardless of an existence of the targeting Object Instance(s) or Resource and access rights.
 *
 *
 *
 *
 *
 *
 *
 *
 * @param context
 * @param object_id
 * @return
 */


static bool __has_server_instances(lwm2m_context *context) {
    lwm2m_object *server_object = lfind(context->object_tree, SERVER_OBJECT_ID);
    return server_object->instances->size > 0;
}

static int __bootstrap_create_instance(lwm2m_object *object, lwm2m_instance *parsed_instance, int instance_id) {
    lwm2m_instance *instance = lwm2m_instance_new_with_id(object, instance_id);

    /***** If one instance is allowed, then instance's ID=0 *****/
    if (object->multiple == false) {
        instance->id = 0;
    }

    for (list_elem *elem = instance->resources->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *template_resource = elem->value;
        lwm2m_resource *parsed_resource =  lfind(parsed_instance->resources, elem->key);

        /***** Error if not all mandatory resources are provided  ******/
        if (template_resource->mandatory && template_resource->type != NONE && parsed_resource == NULL) {
            return RESPONSE_CODE_METHOD_NOT_ALLOWED;
        }

        /**** Don't check if resource is writeable ****/
    }
    merge_resources(instance, parsed_instance, true, false);
    return RESPONSE_CODE_CREATED;
}

///////////// CALLBACKS ////////////////////

int on_bootstrap_object_write(lwm2m_object *object, char *message, int message_len) {
    list *parsed_instances = parse_object(object, message, message_len);

    for (list_elem *elem = parsed_instances->first; elem != NULL; elem = elem->next) {
        lwm2m_instance *old_instance = lfind(object->instances, elem->key);
        lwm2m_instance *new_instance = elem->value;

        if (old_instance == NULL) {
            __bootstrap_create_instance(object, new_instance, new_instance->id);
        } else {
            merge_resources(old_instance, new_instance, true, false);
        }
    }
    return RESPONSE_CODE_CHANGED;
}

int on_bootstrap_instance_write(lwm2m_object *object, int instance_id, char *message, int message_len) {
    lwm2m_instance *old_instance = lfind(object->instances, instance_id);
    lwm2m_instance parsed_instance = {
            .resources = parse_instance(object, message, message_len)
    };

    if (old_instance == NULL) {
        return __bootstrap_create_instance(object, &parsed_instance, instance_id);
    } else {
        merge_resources(old_instance, &parsed_instance, true, false);
    }
    __free_parsed_resources(parsed_instance.resources);
    return RESPONSE_CODE_CHANGED;
}

int on_bootstrap_resource_write(lwm2m_resource *resource, char *message, int message_len) {
    /**** Parse and copy values ****/
    lwm2m_resource *parsed_resource;
    if (resource->multiple) {
        parsed_resource = (lwm2m_resource *) malloc(sizeof(lwm2m_resource));
        parsed_resource->id = resource->id;
        parsed_resource->multiple = true;
        parsed_resource->instances = parse_multiple_resource(resource->instance->object, resource->id, message, message_len);
    } else {
        parsed_resource = parse_resource(resource->instance->object, resource->id, message, message_len);
    }
    merge_resource(resource, parsed_resource, true, false);
    __free_parsed_resource(parsed_resource);
    return RESPONSE_CODE_CHANGED;
}

// TODO EXCEPT Bootstrap server account
int on_bootstrap_delete_all(lwm2m_context *context) {
    for (list_elem *elem = context->object_tree->first; elem != NULL; elem = elem->next) {
        lwm2m_object *object = elem->value;
        __free_instances(object->instances); // TODO ALSO REMOVE FROM LIST!!!
    } // TODO bootstrap write error when object not implemented
    return 0;
}

int on_bootstrap_delete(lwm2m_context *context, lwm2m_instance *instance) {
    lwm2m_object *object = instance->object;
    lremove(object->instances, instance->id);
    __free_instance(instance);
    return 0;
}


int on_bootstrap_finish(lwm2m_context *context) {
    pthread_mutex_lock(&context->bootstrap_mutex);
    context->state = BOOTSTRAPPED;
    pthread_cond_signal(&context->bootstrap_finished_condition);
    pthread_mutex_unlock(&context->bootstrap_mutex);
    return 0;
}

///////////// OTHER ///////////////////////

void lwm2m_wait_for_server_bootstrap(lwm2m_context *context) {
    pthread_mutex_lock(&context->bootstrap_mutex);
    while (context->state == WAITING_FOR_BOOTSTRAP || context->state == STARTED) {
        // TODO time wait for clientHolfOffTime
        pthread_cond_wait(&context->bootstrap_finished_condition, &context->bootstrap_mutex);
    }
    pthread_mutex_unlock(&context->bootstrap_mutex);
}

int initiate_bootstrap(lwm2m_context *context) {
    lwm2m_topic topic = {
            .operation = LWM2M_OPERATION_DEREGISTER,
            .type = "req",
            .token = generate_token(),
            .client_id = context->client_id,
//            .server_id = TODO server_id from security object
            .object_id = -1,
            .instance_id = -1,
            .resource_id = -1,
    };

    lwm2m_request request = {
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload = "",
            .payload_len = 0
    };
    perform_bootstrap_request(context, topic, request);
    return 0;
}

int lwm2m_bootstrap(lwm2m_context *context) {
    context->state = STARTED;
    context->is_bootstrap_ready = false;

    if (context->has_smartcard && context->smartcard_bootstrap_callback != NULL) {
        if (context->smartcard_bootstrap_callback(context) == 0 && __has_server_instances(context)) {
            context->state = BOOTSTRAPPED;
        }
    } else if (context->factory_bootstrap_callback != NULL) {
        if (context->factory_bootstrap_callback(context) == 0 && __has_server_instances(context)) {
            context->state = BOOTSTRAPPED;
        }
    }

    publish_connected(context);

//    if (context->state != BOOTSTRAPPED) {
        // Server initiated bootstrap
        lwm2m_wait_for_server_bootstrap(context);
        if (context->state != BOOTSTRAPPED) {
            // Client initiated bootstrap TODO check response of bootstrap request
            initiate_bootstrap(context);
            context->state = WAITING_FOR_BOOTSTRAP;
            lwm2m_wait_for_server_bootstrap(context);
            if (context->state != BOOTSTRAPPED) {
                return -1;
            }
        }
//    }
    return 0;
}