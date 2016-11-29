#include <lwm2m_attribute.h>
#include <lwm2m_parser.h>

#define WRITE_ATTR_RESPONSE { CONTENT_TYPE_TEXT, RESPONSE_CODE_CONTENT, 1, NULL, 0 }
#define DISCOVER_RESPONSE { CONTENT_TYPE_TEXT, RESPONSE_CODE_CHANGED, 1, NULL, 0 }

#define DEFAULT_PMIN_RES 2
#define DEFAULT_PMAX_RES 3



static lwm2m_attributes *__resource_attr_for_server(lwm2m_server *server, lwm2m_resource *resource) {
    return (lwm2m_attributes *) lwm2m_map_get(resource->attributes, server->short_server_id);
}

static lwm2m_attributes *__instance_attr_for_server(lwm2m_server *server, lwm2m_instance *instance) {
    return (lwm2m_attributes *) lwm2m_map_get(instance->attributes, server->short_server_id);
}

static lwm2m_attributes *__object_attr_for_server(lwm2m_server *server, lwm2m_object *object) {
    return (lwm2m_attributes *) lwm2m_map_get(object->attributes, server->short_server_id);
}

/***************** PMIN AND PMAX *****************/

int *get_resource_pmin(lwm2m_server *server, lwm2m_resource *resource, int lookup) {
    lwm2m_attributes *attr = __resource_attr_for_server(server, resource);
    int *pmin = attr == NULL ? NULL : attr->pmin;
    if (pmin == NULL && lookup) {
        pmin = get_instance_pmin(server, resource->instance, lookup);
    }
    return pmin;
}

int *get_resource_pmax(lwm2m_server *server, lwm2m_resource *resource, int lookup) {
    lwm2m_attributes *attr = __resource_attr_for_server(server, resource);
    int *pmax = attr == NULL ? NULL : attr->pmax;
    if (pmax == NULL && lookup) {
        pmax = get_instance_pmax(server, resource->instance, lookup);
    }
    return pmax;
}

int *get_instance_pmin(lwm2m_server *server, lwm2m_instance *instance, int lookup) {
    lwm2m_attributes *attr = __instance_attr_for_server(server, instance);
    int *pmin = attr == NULL ? NULL : attr->pmin;
    if (pmin == NULL && lookup) {
        pmin = get_object_pmin(server, instance->object, lookup);
    }
    return pmin;
}

int *get_instance_pmax(lwm2m_server *server, lwm2m_instance *instance, int lookup) {
    lwm2m_attributes *attr = __instance_attr_for_server(server, instance);
    int *pmax = attr == NULL ? NULL : attr->pmax;
    if (pmax == NULL && lookup) {
        pmax = get_object_pmax(server, instance->object, lookup);
    }
    return pmax;
}

int *get_object_pmin(lwm2m_server *server, lwm2m_object *object, int lookup) {
    lwm2m_attributes *attr = __object_attr_for_server(server, object);
    int *pmin = attr == NULL ? NULL : attr->pmax;
    if (pmin == NULL && lookup == 2) {
        pmin = get_default_pmin(server);
    }
    return pmin;
}

int *get_object_pmax(lwm2m_server *server, lwm2m_object *object, int lookup) {
    lwm2m_attributes *attr = __object_attr_for_server(server, object);
    int *pmax = attr == NULL ? NULL : attr->pmax;
    if (pmax == NULL && lookup == 2) {
        pmax = get_default_pmax(server);
    }
    return pmax;
}

int *get_default_pmin(lwm2m_server *server) {
    lwm2m_instance *server_instance = server->server_instance;
    lwm2m_resource *resource = lwm2m_map_get_resource(server_instance->resources, DEFAULT_PMIN_RES);
    if (resource->value != NULL) {
        return (int *) resource->value;
    }
    return NULL;
}

int *get_default_pmax(lwm2m_server *server) {
    lwm2m_instance *server_instance = server->server_instance;
    lwm2m_resource *resource = lwm2m_map_get_resource(server_instance->resources, DEFAULT_PMAX_RES);
    if (resource->value != NULL) {
        return (int *) resource->value;
    }
    return NULL;
}

/***************** RESOURCE SPECIFIC ATTRIBUTES *****************/


int *get_resource_dim(lwm2m_server *server, lwm2m_resource *resource) {
    lwm2m_attributes *attr = __resource_attr_for_server(server, resource);
    return attr == NULL ? NULL : attr->dim;
}

float *get_resource_gt(lwm2m_server *server, lwm2m_resource *resource) {
    lwm2m_attributes *attr = __resource_attr_for_server(server, resource);
    return attr == NULL ? NULL : attr->gt;
}

float *get_resource_lt(lwm2m_server *server, lwm2m_resource *resource) {
    lwm2m_attributes *attr = __resource_attr_for_server(server, resource);
    return attr == NULL ? NULL : attr->lt;
}

float *get_resource_stp(lwm2m_server *server, lwm2m_resource *resource) {
    lwm2m_attributes *attr = __resource_attr_for_server(server, resource);
    return attr == NULL ? NULL : attr->stp;
}

static void __merge_attributes(lwm2m_attributes *old_attr, lwm2m_attributes *new_attr) {
    if (new_attr->dim != NULL) {
        if (old_attr->dim != NULL) free(old_attr->dim);
        old_attr->dim = new_attr->dim;
    }
    if (new_attr->pmax != NULL) {
        if (old_attr->dim != NULL) free(old_attr->pmax);
        old_attr->pmax = new_attr->pmax;
    }
    if (new_attr->pmin != NULL) {
        if (old_attr->dim != NULL) free(old_attr->pmin);
        old_attr->pmin = new_attr->pmin;
    }
    if (new_attr->gt != NULL) {
        if (old_attr->dim != NULL) free(old_attr->gt);
        old_attr->gt = new_attr->gt;
    }
    if (new_attr->lt != NULL) {
        if (old_attr->dim != NULL) free(old_attr->lt);
        old_attr->lt = new_attr->lt;
    }
    if (new_attr->stp != NULL) {
        if (old_attr->dim != NULL) free(old_attr->stp);
        old_attr->stp = new_attr->stp;
    }
}

/***************** WRITE ATTRIBUTE *****************/

lwm2m_response on_resource_write_attributes(lwm2m_server *server, lwm2m_resource *resource, lwm2m_request request) {
    /**** Don't have to check any access ****/
    lwm2m_attributes *old_attr = __resource_attr_for_server(server, resource);
    if (old_attr == NULL) {
        old_attr = malloc(sizeof(lwm2m_attributes));
        *old_attr = (lwm2m_attributes) EMPTY_ATTR;
        lwm2m_map_put(resource->attributes, server->short_server_id, old_attr);
    }
    lwm2m_attributes new_attr = EMPTY_ATTR;
    parse_attributes(&new_attr, request.payload);

    __merge_attributes(old_attr, &new_attr);
    return (lwm2m_response) WRITE_ATTR_RESPONSE;
}

lwm2m_response on_instance_write_attributes(lwm2m_server *server, lwm2m_instance *instance, lwm2m_request request) {
    /**** Don't have to check any access ****/
    lwm2m_attributes *old_attr = __instance_attr_for_server(server, instance);
    if (old_attr == NULL) {
        old_attr = malloc(sizeof(lwm2m_attributes));
        *old_attr = (lwm2m_attributes) EMPTY_ATTR;
        lwm2m_map_put(instance->attributes, server->short_server_id, old_attr);
    }
    lwm2m_attributes new_attr = EMPTY_ATTR;
    parse_attributes(&new_attr, request.payload);

    __merge_attributes(old_attr, &new_attr); // TODO should I change scheduler?
    return (lwm2m_response) WRITE_ATTR_RESPONSE;
}

lwm2m_response on_object_write_attributes(lwm2m_server *server, lwm2m_object *object, lwm2m_request request) {
    /**** Don't have to check any access ****/
    lwm2m_attributes *old_attr = __object_attr_for_server(server, object);
    if (old_attr == NULL) {
        old_attr = malloc(sizeof(lwm2m_attributes));
        *old_attr = (lwm2m_attributes) EMPTY_ATTR;
        lwm2m_map_put(object->attributes, server->short_server_id, old_attr);
    }
    lwm2m_attributes new_attr = EMPTY_ATTR;
    parse_attributes(&new_attr, request.payload);

    __merge_attributes(__object_attr_for_server(server, object), &new_attr);
    return (lwm2m_response) WRITE_ATTR_RESPONSE;
}

/***************** DISCOVER *****************/

lwm2m_response on_object_discover(lwm2m_server *server, lwm2m_object *object) {
    lwm2m_response response = DISCOVER_RESPONSE;
    response.payload = (char *) malloc(sizeof(char) * 500);

    /**** Don't have to check any access ****/
    serialize_lwm2m_object_discover(server, object, response.payload);
    response.payload_len = (int) strlen(response.payload);
    return response;
}

lwm2m_response on_instance_discover(lwm2m_server *server, lwm2m_instance *instance) {
    lwm2m_response response = DISCOVER_RESPONSE;
    response.payload = (char *) malloc(sizeof(char) * 500);

    /**** Don't have to check any access ****/
    serialize_lwm2m_instance_discover(server, instance, response.payload);
    response.payload_len = (int) strlen(response.payload);
    return response;
}

lwm2m_response on_resource_discover(lwm2m_server *server, lwm2m_resource *resource) {
    lwm2m_response response = DISCOVER_RESPONSE;
    response.payload = (char *) malloc(sizeof(char) * 500);

    /**** Don't have to check any access ****/
    serialize_lwm2m_resource_discover(server, resource, response.payload);
    response.payload_len = (int) strlen(response.payload);
    return response;
}