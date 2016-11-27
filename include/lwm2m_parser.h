#ifndef LYNX_LWM2M_PARSER_H
#define LYNX_LWM2M_PARSER_H

#include "lwm2m.h"
#include "lwm2m_object.h"
#include "lwm2m_attribute.h"

#define TEXT_FORMAT 0
#define TLV_FORMAT 1

// TODO opis
void parse_attributes(lwm2m_attributes *attributes, char *message);


/**
 * TEXT/TLV -> ENTITY
 *
 * Note 1:
 * Functions don't parse ID from message - it is known from URI. these are given into function
 * to create a proper resource from callbacks(resource definition callbacks)
 *
 * Note 2:
 * We don't need parse_multiple_resource to create resource - we just need values of resource instances to override
 * We don't need parse_instance          to create instance - we just need values of resources to override
 * We don't need parse_object            to create object   - we just need values of instances to override
 *
 * Note 3:
 * TODO make parse_resource_text function and create ifs t orecide if use (parse_multiple_resource or parse_resource_text)
 */

lwm2m_resource *parse_resource(lwm2m_context *context, int object_id, int resource_id, char *message, int message_len);

lwm2m_map *parse_multiple_resource(lwm2m_context *context, int object_id, int resource_id, char *message, int message_len);

lwm2m_map *parse_instance(lwm2m_context *context, int object_id, char *message, int message_len);

lwm2m_map *parse_object(lwm2m_context *context, int object_id, char *message, int message_len);

/**
 * ENTITY -> TEXT/TLV
 *
 * It is assumed that:
 * 1. Access control for (instance, READ) is granted
 * 2. Resources have READ operation allowed
 *
 * Note:
 * serialize_multiple_resource don't create header for MULTIPLE_RESOURCE (it's done on higher level)
 * serialize_single_resource   don't create header for RESOURCE (it's done on higher level)
 * serialize_instance          don't create header for INSTANCE (it's done on higher level)
 * serialize_object            don't create header for OBJECT (it doesn't exist)
 *
 * Message should be already allocated
 */

void serialize_resource_text(lwm2m_resource *resource, char *message, int *message_len);

void serialize_single_resource(lwm2m_resource *resource, char *message, int *message_len);

void serialize_multiple_resource(lwm2m_map *resources, char *message, int *message_len);

void serialize_instance(lwm2m_map *resources, char *message, int *message_len);

void serialize_object(lwm2m_map *instances, char *message, int *message_len);


/**
 * Serializing discover messages
 *
 *
 *
 */
void serialize_lwm2m_object_discover(lwm2m_server *server, lwm2m_object *object, char *message);
void serialize_lwm2m_instance_discover(lwm2m_server *server, lwm2m_instance *instance, char *message);
void serialize_lwm2m_resource_discover(lwm2m_server *server, lwm2m_resource *resource, char *message);


/**
 * Serializing register messages
 *
 *
 *
 */
char *serialize_lwm2m_objects_and_instances(lwm2m_context *context);


#endif //LYNX_LWM2M_PARSER_H
