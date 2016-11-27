#ifndef LYNX_LWM2M_INFORMATION_REPORTING_H
#define LYNX_LWM2M_INFORMATION_REPORTING_H

#include <lwm2m.h>
#include <lwm2m_transport.h>
#include <lwm2m_object.h>
#include <list.h>

/***
 * These functions are called only once - when start observing
 *
 *
 *
 *
 */
lwm2m_response on_resource_observe(lwm2m_server *server, lwm2m_resource *resource, char *token);

lwm2m_response on_instance_observe(lwm2m_server *server, lwm2m_instance *instance, char *token);

lwm2m_response on_object_observe(lwm2m_server *server, lwm2m_object *object, char *token);


/**
 * Cancel observe.
 * These functions are called only once - when stop observing
 *
 *
 *
 *
 *
 */
lwm2m_response on_object_cancel_observe(lwm2m_server *server, lwm2m_object *object);

lwm2m_response on_instance_cancel_observe(lwm2m_server *server, lwm2m_instance *instance);

lwm2m_response on_resource_cancel_observe(lwm2m_server *server, lwm2m_resource *resource);



/**
 * Return list of servers which should be notified
 *
 *
 *
 *
 *
 */
list *should_notify(lwm2m_resource *resource, lwm2m_value *old_value, lwm2m_value *new_value);


#endif //LYNX_LWM2M_INFORMATION_REPORTING_H
