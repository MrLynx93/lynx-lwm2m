#include <lwm2m_attribute.h>
#include <lwm2m_parser.h>

#define WRITE_ATTR_RESPONSE { 1, 205, 1, NULL, 0 }

#define DEFAULT_PMIN_RES 2
#define DEFAULT_PMAX_RES 3

static lwm2m_attributes* __resource_attr_for_server(lwm2m_server *server, lwm2m_resource *resource) {
    return (lwm2m_attributes *) lwm2m_map_get(resource->attributes, server->short_server_id);
}

static lwm2m_attributes* __instance_attr_for_server(lwm2m_server *server, lwm2m_instance *instance) {
    return (lwm2m_attributes *) lwm2m_map_get(instance->attributes, server->short_server_id);
}

static lwm2m_attributes* __object_attr_for_server(lwm2m_server *server, lwm2m_object *object) {
    return (lwm2m_attributes *) lwm2m_map_get(object->attributes, server->short_server_id);
}

/***************** PMIN AND PMAX *****************/

int *get_resource_pmin(lwm2m_server *server, lwm2m_resource *resource, int lookup) {
    int *pmin = __resource_attr_for_server(server, resource)->pmin;
    if (pmin == NULL && lookup) {
        pmin = get_instance_pmin(server, resource->instance, lookup);
    }
    return pmin;
}

int *get_resource_pmax(lwm2m_server *server, lwm2m_resource *resource, int lookup) {
    int *pmax = __resource_attr_for_server(server, resource)->pmax;
    if (pmax == NULL && lookup) {
        pmax = get_instance_pmax(server, resource->instance, lookup);
    }
    return pmax;
}

int *get_instance_pmin(lwm2m_server *server, lwm2m_instance *instance, int lookup){
    int *pmin = __instance_attr_for_server(server, instance)->pmin;
    if (pmin == NULL && lookup) {
        pmin = get_object_pmin(server, instance->object, lookup);
    }
    return pmin;
}

int *get_instance_pmax(lwm2m_server *server, lwm2m_instance *instance, int lookup){
    int *pmax = __instance_attr_for_server(server, instance)->pmax;
    if (pmax == NULL && lookup) {
        pmax = get_object_pmax(server, instance->object, lookup);
    }
    return pmax;
}

int *get_object_pmin(lwm2m_server *server, lwm2m_object *object, int lookup) {
    int *pmin = __object_attr_for_server(server, object)->pmax;
    if (pmin == NULL && lookup == 2) {
        pmin = get_default_pmin(server);
    }
    return pmin;
}

int *get_object_pmax(lwm2m_server *server, lwm2m_object *object, int lookup) {
    int *pmax = __object_attr_for_server(server, object)->pmax;
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
    return __resource_attr_for_server(server, resource)->dim;
}

float *get_resource_gt(lwm2m_server *server, lwm2m_resource *resource) {
    return __resource_attr_for_server(server, resource)->gt;
}

float *get_resource_lt(lwm2m_server *server, lwm2m_resource *resource) {
    return __resource_attr_for_server(server, resource)->lt;
}

float *get_resource_stp(lwm2m_server *server, lwm2m_resource *resource) {
    return __resource_attr_for_server(server, resource)->stp;
}

/************** SERIALIZE */
// TODO move???


// TODO free memory
static void merge_attributes(lwm2m_attributes *old_attr, lwm2m_attributes *new_attr) {
    if (new_attr->dim != NULL)  old_attr->dim  = new_attr->dim;
    if (new_attr->pmax != NULL) old_attr->pmax = new_attr->pmax;
    if (new_attr->pmin != NULL) old_attr->pmin = new_attr->pmin;
    if (new_attr->gt != NULL)   old_attr->gt   = new_attr->gt;
    if (new_attr->lt != NULL)   old_attr->lt   = new_attr->lt;
    if (new_attr->stp != NULL)  old_attr->stp  = new_attr->stp;
}

lwm2m_response on_resource_write_attributes(lwm2m_server *server, lwm2m_resource *resource, lwm2m_request request) {
    /**** Don't have to check any access ****/
    lwm2m_attributes attributes = EMPTY_ATTR;
    parse_attributes(&attributes, request.payload);
    merge_attributes(__resource_attr_for_server(server, resource), &attributes);
    return (lwm2m_response) WRITE_ATTR_RESPONSE;
}

lwm2m_response on_instance_write_attributes(lwm2m_server *server, lwm2m_instance *instance, lwm2m_request request) {
    /**** Don't have to check any access ****/
    lwm2m_attributes attributes = EMPTY_ATTR;
    parse_attributes(&attributes, request.payload);
    merge_attributes(__instance_attr_for_server(server, instance), &attributes);
    return (lwm2m_response) WRITE_ATTR_RESPONSE;
}

lwm2m_response on_object_write_attributes(lwm2m_server *server, lwm2m_object *object, lwm2m_request request) {
    /**** Don't have to check any access ****/
    lwm2m_attributes attributes = EMPTY_ATTR;
    parse_attributes(&attributes, request.payload);
    merge_attributes(__object_attr_for_server(server, object), &attributes);
    return (lwm2m_response) WRITE_ATTR_RESPONSE;
}