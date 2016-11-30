#include "lwm2m.h"
#include "lwm2m_parser.h"

static void __append_object_attr(char *message, lwm2m_server *server, lwm2m_object *object, int lookup) {
    char buffer[10];

    int *pmin = get_object_pmin(server, object, lookup);
    if (pmin != NULL) {
        sprintf(buffer, ";pmin=%d", *pmin);
        strcat(message, buffer);
    }

    int *pmax = get_object_pmax(server, object, lookup);
    if (pmax != NULL) {
        sprintf(buffer, ";pmax=%d", *pmax);
        strcat(message, buffer);
    }
}

static void __append_instance_attr(char *message, lwm2m_server *server, lwm2m_instance *instance, int lookup) {
    char buffer[10];

    int *pmin = get_instance_pmin(server, instance, lookup);
    if (pmin != NULL) {
        sprintf(buffer, ";pmin=%d", *pmin);
        strcat(message, buffer);
    }

    int *pmax = get_instance_pmax(server, instance, lookup);
    if (pmax != NULL) {
        sprintf(buffer, ";pmax=%d", *pmax);
        strcat(message, buffer);
    }
}

static void __append_resource_attr(char *message, lwm2m_server *server, lwm2m_resource *resource, int lookup) {
    char buffer[10];

    int *dim = get_resource_dim(server, resource);
    if (dim != NULL) {
        sprintf(buffer, ";dim=%d", *dim);
        strcat(message, buffer);
    }

    float *gt = get_resource_gt(server, resource);
    if (gt != NULL) {
        sprintf(buffer, ";gt=%.3f", *gt);
        strcat(message, buffer);
    }

    float *lt = get_resource_lt(server, resource);
    if (lt != NULL) {
        sprintf(buffer, ";lt=%.3f", *lt);
        strcat(message, buffer);
    }

    float *stp = get_resource_stp(server, resource);
    if (stp != NULL) {
        sprintf(buffer, ";stp=%.3f", *stp);
        strcat(message, buffer);
    }

    int *pmin = get_resource_pmin(server, resource, lookup);
    if (pmin != NULL) {
        sprintf(buffer, ";pmin=%d", *pmin);
        strcat(message, buffer);
    }

    int *pmax = get_resource_pmax(server, resource, lookup);
    if (pmax != NULL) {
        sprintf(buffer, ";pmax=%d", *pmax);
        strcat(message, buffer);
    }

}

void serialize_lwm2m_object_discover(lwm2m_server *server, lwm2m_object *object, char *message) {
    sprintf(message, "<%d/>", object->id);
    __append_object_attr(message, server, object, 0);

    char buffer[10];
    for (int id = 0; id < object->resource_def_len; ++id) {
        sprintf(buffer, "<%d/0/%d>", object->id, id);
        strcat(message, ",");
        strcat(message, buffer);
    }
}

void serialize_lwm2m_instance_discover(lwm2m_server *server, lwm2m_instance *instance, char *message) {
    sprintf(message, "<%d/%d>", instance->object->id, instance->id);
    __append_instance_attr(message, server, instance, 0);

    char buffer[10];
    for (list_elem *elem = instance->resources->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *resource = elem->value;
        sprintf(buffer, "<%d/%d/%d>", instance->object->id, instance->id, resource->id);
        strcat(message, ",");
        strcat(message, buffer);
        __append_resource_attr(message, server, resource, 0);
    }
}

void serialize_lwm2m_resource_discover(lwm2m_server *server, lwm2m_resource *resource, char *message) {
    sprintf(message, "<%d/%d/%d>", resource->instance->object->id, resource->instance->id, resource->id);
    __append_resource_attr(message, server, resource, 1);
}
