#ifndef LWM2M_ATTRIBUTE_H
#define LWM2M_ATTRIBUTE_H

#include <lwm2m.h>
#include "lwm2m_transport.h"

#define EMPTY_ATTR {NULL, NULL, NULL, NULL, NULL, NULL};


typedef struct lwm2m_attributes {
    int *pmin;
    int *pmax;
    float *gt;
    float *lt;
    float *stp;
    int *dim;
} lwm2m_attributes;

/**
 * resource specific attribute accessors
 *
 *
 *
 *
 *
 */
int *get_resource_dim(lwm2m_server *server, lwm2m_resource *resource);
float *get_resource_gt(lwm2m_server *server, lwm2m_resource *resource);
float *get_resource_lt(lwm2m_server *server, lwm2m_resource *resource);
float *get_resource_stp(lwm2m_server *server, lwm2m_resource *resource);


/**
 * pmin and pmax attribute accessors
 * + if lookup = 0 then return level-specific attribute
 * + if lookup = 1 then:
 *      for resource look for first of: [resource->instance->object]
 *      for instance look for first of: [instance->object]
 *      for object   same as lookup = 0
 * + if lookup = 2 then:
 *      for resource look for first of: [resource->instance->object->default]
 *      for instance look for first of: [instance->object->default]
 *      for object   look for first of: [object->default]
 *
 *  where "default" is default attribute defined in server's Security object
 */
// TODO which object?
int *get_resource_pmin(lwm2m_server *server, lwm2m_resource *resource, int lookup);
int *get_resource_pmax(lwm2m_server *server, lwm2m_resource *resource, int lookup);
int *get_instance_pmin(lwm2m_server *server, lwm2m_instance *instance, int lookup);
int *get_instance_pmax(lwm2m_server *server, lwm2m_instance *instance, int lookup);
int *get_object_pmin(lwm2m_server *server, lwm2m_object *object, int lookup);
int *get_object_pmax(lwm2m_server *server, lwm2m_object *object, int lookup);
int *get_default_pmin(lwm2m_server *server);
int *get_default_pmax(lwm2m_server *server);

// TODO move it?


/**
 * Handle requests for write attribute
 *
 *
 *
 *
 */
lwm2m_response on_resource_write_attributes(lwm2m_server *server, lwm2m_resource *resource, lwm2m_request request);
lwm2m_response on_instance_write_attributes(lwm2m_server *server, lwm2m_instance *instance, lwm2m_request request);
lwm2m_response on_object_write_attributes(lwm2m_server *server, lwm2m_object *object, lwm2m_request request);





#endif