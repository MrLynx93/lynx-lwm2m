#include "../include/lwm2m_parser.h"
#include <bool.h>

#define OBJECT_INSTANCE_TYPE    0b00000000
#define RESOURCE_INSTANCE_TYPE  0b01000000
#define MULTIPLE_RESOURCE_TYPE  0b10000000
#define RESOURCE_TYPE           0b11000000


//////////////// PUBLIC FUNCTIONS /////////////////


int deserialize_lwm2m_object(lwm2m_object* object, char* message, int message_len) {

    

}

int deserialize_lwm2m_instance(lwm2m_instance* instance, char* message, int message_len) {
    lwm2m_map* parsed_resources = lwm2m_map_new();
    char* curr_buf = message;
    int error;

    int identifier_length;
    int value_length;
    int identifier;
    int type;

    // Parse and check any resource don't have WRITE access
    while (curr_buf < message + message_len) {
        curr_buf = read_header(curr_buf, &identifier_length, &value_length, &type);
        curr_buf = read_identifier(curr_buf, identifier_length);
        curr_buf = read_length(curr_buf, &value_length); // either reads length or leaves length from header

        lwm2m_resource *parsed_resource = lwm2m_resource_new(type == MULTIPLE_RESOURCE_TYPE ? true : false);
        error = deserialize_lwm2m_resource(parsed_resource, message, value_length, TLV_FORMAT);
        if (error) {
            return error;
        }
        curr_buf += value_length;
    }

    // If we are here, we succeded, We can write values from parsed resources to real resources
    int *keys = (int *) malloc(parsed_resources->size);
    for (int i = 0, id = keys[i]; i < parsed_resources->size; i++) {
        lwm2m_resource* real_resource = lwm2m_map_get(instance->resources, id)->resource;
        lwm2m_resource* parsed_resource = lwm2m_map_get(parsed_resources, id)->resource;
        copy_value(parsed_resource, real_resource);
        free_lwm2m_resource(parsed_resource);
    }
    lwm2m_map_free(parsed_resources); //todo free everything also in serialize
}

int deserialize_lwm2m_resource(lwm2m_resource* resource, char* message, int message_len, int format) {
    if (resource->multiple) {
        ...
    }
    else {
        switch (resource->type) {
            case INTEGER:
                return deserialize_int(resource, message, message_len, format);
            case DOUBLE:
                return deserialize_double(resource, message, message_len, format);
            case STRING:
                return deserialize_string(resource, message, message_len, format);
            case OPAQUE:
                return deserialize_opaque(resource, message, message_len, format);
            case BOOLEAN:
                return deserialize_boolean(resource, message, message_len, format);
            case LINK:
                return deserialize_link(resource, message, message_len, format);
            default:
                return OPERATION_NOT_SUPPORTED;
        }
    }
}

int deserialize_node(lwm2m_node* node, char* message, int message_len, )