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
struct execute_param {
    int key;
    lwm2m_value value;
};



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

lwm2m_response on_instance_create(lwm2m_server *server, lwm2m_object *object, int instance_id, char *message, int message_len);

lwm2m_response on_instance_delete(lwm2m_server *server, lwm2m_instance *instance);




void DUMP_SINGLE_RESOURCE(lwm2m_resource *resource);

void DUMP_MULTIPLE_RESOURCE(lwm2m_resource *resource);

void DUMP_INSTANCE(lwm2m_instance *instance);

void DUMP_OBJECT(lwm2m_object *object);

void DUMP_ALL(lwm2m_context *context);
//
////////////// DELETE ///////////
//
///* Checks access control and deletes instance from object tree */
//int on_lwm2m_instance_delete(lwm2m_server* server, lwm2m_instance* instance);
//
//
////////////// CREATE ///////////
//
///* Checks access control, parses instance from TLV format message and creates new instance in object tree */
//int on_lwm2m_instance_create(lwm2m_server* server, lwm2m_object* object, char* message, int* created_instance_id);
//
//
///////////// DISCOVER //////////
//
///* Writes object discover message to message argument (don't need access control) */
//int on_lwm2m_object_discover(lwm2m_server* server, lwm2m_object* object, char** message);
//
///* Writes instance discover message to message argument (don't need access control) */
//int on_lwm2m_instance_discover(lwm2m_server* server, lwm2m_instance* instance, char** message);
//
///* Writes resource discover message to message argument (don't need access control) */
//int on_lwm2m_resource_discover(lwm2m_server* server, lwm2m_resource* resource, char** message);
//
//
///////////// WRITE ATTRIBUTES ///////////
//
///* Reads message and writes attributes into object */
//int on_lwm2m_object_write_attributes(lwm2m_server* server, lwm2m_object* object, char* message);
//
///* Reads message and writes attributes into instance */
//int on_lwm2m_instance_write_attributes(lwm2m_server* server, lwm2m_instance* instance, char* message);
//
///* Reads message and writes attributes into resource */
//int on_lwm2m_resource_write_attributes(lwm2m_server* server, lwm2m_resource* resource, char* message);
//
///////////////// EXECUTE ////////////////////
//
//typedef struct lwm2m_execute_parameter {
//    char* name;
//    char* value; // TODO many types
//} lwm2m_execute_parameter;
//
//
#endif //LYNX_LWM2M_DEVICE_MANAGEMENT_H
