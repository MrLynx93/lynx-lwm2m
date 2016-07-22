#ifndef LYNX_LWM2M_PARSER_H
#define LYNX_LWM2M_PARSER_H

#include "lwm2m_object.h"

#define TEXT_FORMAT 0
#define TLV_FORMAT 1

#define PARSE_ERROR 1

/*
 * All functions return 0 if serialize/deserialize was successful, otherwise it returns PARSE_ERROR
 * Server parameter for serialize functions is needed for checking access control
 */


////// SERIALIZE OBJECTS //////

/* Converts object to TLV format and writes output in message */
int serialize_lwm2m_object(lwm2m_server *server, lwm2m_object *object, char **message, int *message_len);

/* Converts instance to TLV format and writes output in message. */
int serialize_lwm2m_instance(lwm2m_server *server, lwm2m_instance *instance, char **message, int *message_len);

/* Converts resource to proper format and writes output in message */
int serialize_lwm2m_resource(lwm2m_resource *resource, char **message, int *message_len, int format);


////// DESERIALIZE OBJECTS //////

/* Reads object from TLV format and saves values in underlying instances and resources */
int deserialize_lwm2m_object(lwm2m_object *object, char *message);

/* Reads instance from TLV format and saves values in underlying resources */
int deserialize_lwm2m_instance(lwm2m_instance *instance, char *message);

/* Reads resource from proper format and saves values */
int deserialize_lwm2m_resource(lwm2m_resource *resource, char *message, int format);


////// DESERIALIZE ATTRIBUTES /////

/* Reads attributes from GET parameters and saves values in attributes */
int deserialize_lwm2m_attributes(lwm2m_attributes *attributes, char *message);


////// SERIALIZE DISCOVER /////////

/* Creates LWM2M object discover message and writes output in message */
void serialize_lwm2m_object_discover(lwm2m_object *object, char **message);

/* Creates LWM2M instance discover message and writes output in message */
void serialize_lwm2m_instance_discover(lwm2m_instance *instance, char **message);

/* Creates LWM2M resource discover message and writes output in message */
void serialize_lwm2m_resource_discover(lwm2m_resource *resource, char **message);


////////// TEXT VALUES /////////

/* Returns text/tlv representation of value */
char *serialize_lwm2m_value(lwm2m_value value, lwm2m_type type, int format);

/* Returns lwm2m_value from text/tlv message */
lwm2m_value deserialize_lwm2m_value(char* message, lwm2m_type* type, int format);


#endif //LYNX_LWM2M_PARSER_H
