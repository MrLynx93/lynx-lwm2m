#ifndef LYNX_LWM2M_DEVICE_MANAGEMENT_H
#define LYNX_LWM2M_DEVICE_MANAGEMENT_H

#include "lwm2m.h"
#include "lwm2m_transport.h"

/**
 * key may be -1 to indicate that this param has no index
 *
 *
 */
// TODO For now let's assume that params cat be only index - no reversing of list needed then


///*
// * All functions except discover and write attributes return error code if some error occurred, in other case returns 0
// * List of error codes:
// * - ACCESS_RIGHT_PERMISSION_DENIED - server does not have access control
// * - OPERATION_NOT_SUPPORTED       - resource does not support corresponding operation
// * - PARSE_ERROR                   - when it was not possible to parse object/resource/instance
// */
//
//
/////////////// READ ///////////
//
///* Checks access control, parses object to TLV and writes output to message */
//int on_lwm2m_object_read(lwm2m_server* server, lwm2m_object* object, char** message);
//
///* Checks access control, parses instance to TLV and writes output to message */
//int on_lwm2m_instance_read(lwm2m_server* server, lwm2m_instance* instance, char** message);
//
///* Checks access control, parses resource to proper format and writes output to message */
//int on_lwm2m_resource_read(lwm2m_server* server, lwm2m_resource* resource, char** message);
//
//
////////////// WRITE ///////////
//
///* Checks access control, parses TLV format message and fills values in instance */
int on_instance_write(lwm2m_server* server, lwm2m_instance* instance, char* message, int message_len);
//
///* Checks access control, parses message in proper format and fills values in resource */
int on_resource_write(lwm2m_server* server, lwm2m_resource* resource, char* message, int message_len);
//

lwm2m_response on_resource_read(lwm2m_server* server, lwm2m_resource* resource);

lwm2m_response on_instance_read(lwm2m_server *server, lwm2m_instance *instance);

lwm2m_response on_object_read(lwm2m_server *server, lwm2m_object *object);

lwm2m_response on_instance_create(lwm2m_server *server, lwm2m_object *object, int *instance_id, char *message, int message_len);

lwm2m_response on_instance_delete(lwm2m_server *server, lwm2m_instance *instance);

lwm2m_response on_resource_execute(lwm2m_server *server, lwm2m_resource *resource, list *args);


// TODO move???
void DUMP_SINGLE_RESOURCE(lwm2m_resource *resource);

void DUMP_MULTIPLE_RESOURCE(lwm2m_resource *resource);

void DUMP_INSTANCE(lwm2m_instance *instance);

void DUMP_OBJECT(lwm2m_object *object);

void DUMP_ALL(lwm2m_context *context);

#endif //LYNX_LWM2M_DEVICE_MANAGEMENT_H
