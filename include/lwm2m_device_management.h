#ifndef LYNX_LWM2M_DEVICE_MANAGEMENT_H
#define LYNX_LWM2M_DEVICE_MANAGEMENT_H

#include "lwm2m_access_control.h"
#include "lwm2m_parser.h"
#include "lwm2m_errors.h"
#include "lwm2m_parser.h"

#define READ 1
#define WRITE 2
#define EXECUTE 4
#define DELETE 8
#define CREATE 16

typedef struct lwm2m_execute_parameter lwm2m_execute_parameter;

/*
 * All functions return error code if some error occured, in other case returns 0
 * List of error codes:
 * - ACCESS_RIGHT_PERMISSON_DENIED - server does not have access control
 * - OPERATION_NOT_SUPPORTED - resource does not support corresponding operation
 */

///////////// READ ///////////

/* Checks access control, parses object to TLV and writes output to message */
int on_object_read(lwm2m_server* server, lwm2m_object* object, char** message);

/* Checks access control, parses instance to TLV and writes output to message */
int on_instance_read(lwm2m_server* server, lwm2m_instance* instance, char** message);

/* Checks access control, parses resource to proper format and writes output to message */
int on_resource_read(lwm2m_server* server, lwm2m_resource* resource, char** message);

//////////// WRITE ///////////

/* Checks access control, parses TLV format message and fills values in instance */
int on_instance_write(lwm2m_server* server, lwm2m_instance* instance, char* message);

/* Checks access control, parses message in proper format and fills values in resource */
int on_resource_write(lwm2m_server* server, lwm2m_instance* resource, char* message);

//////////// DELETE ///////////

/* Checks access control and deletes instance from object tree */
int on_instance_delete(lwm2m_server* server, lwm2m_instance* instance);

//////////// CREATE ///////////

/* Checks access control, parses instance from TLV format message and creates new instance in object tree */
int on_instance_create(lwm2m_server* server, lwm2m_object* object, char* message, int id);



struct lwm2m_execute_parameter {
    char* name;
    char* value; // TODO many types
};


#endif //LYNX_LWM2M_DEVICE_MANAGEMENT_H
