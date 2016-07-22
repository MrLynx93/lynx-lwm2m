#include "../include/lwm2m_parser.h"
#include "../include/lwm2m_errors.h"
#include <bool.h>

#define OBJECT_INSTANCE_TYPE    0b00000000
#define RESOURCE_INSTANCE_TYPE  0b01000000
#define MULTIPLE_RESOURCE_TYPE  0b10000000
#define RESOURCE_TYPE           0b11000000


//////////////// PUBLIC FUNCTIONS /////////////////


int deserialize_lwm2m_object(lwm2m_object *object, char *message, int message_len) {
    // TODO. called only in bootstrap
}

int deserialize_lwm2m_instance(lwm2m_instance *instance, char *message, int message_len) {
    lwm2m_map *parsed_resources = deserialize_lwm2m_node((lwm2m_node *) instance, message, message_len);
    if (parsed_resources == NULL) {
        return OPERATION_NOT_SUPPORTED;
    }
    return copy_and_free_all_resources(parsed_resources, (lwm2m_node *) instance, INSTANCE);
}

int deserialize_lwm2m_resource(lwm2m_resource *resource, char *message, int message_len, int format) {
    if (resource->multiple) {
        lwm2m_map *parsed_resources = deserialize_lwm2m_node((lwm2m_node *) parsed_resource, message, message, message_len);
        if (parsed_resources == NULL) {
            return OPERATION_NOT_SUPPORTED;
        }
        return copy_and_free_all_resources(parsed_resources, (lwm2m_node *) resource, RESOURCE);
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

/////////////////// DESERIALIZE MULTIPLE  ///////////////////////


/* Node can be instance or multiple resource */
static lwm2m_map *deserialize_lwm2m_node(lwm2m_node *node, char *message, int message_len) {
    lwm2m_map *parsed_resources = lwm2m_map_new();
    char *curr_buf = message;
    int error;

    int identifier_length;
    int value_length;
    int length_of_length;
    int identifier;
    int id;
    int type;

    // Parse and check any resource don't have WRITE access
    while (curr_buf < message + message_len) {
        curr_buf = read_header(curr_buf, &identifier_length, &value_length, &length_of_length, &type);
        curr_buf = read_identifier(curr_buf, &id, identifier_length);
        curr_buf = read_length(curr_buf, length_of_length,
                               &value_length); // either reads length or leaves length from header

        lwm2m_resource *parsed_resource = lwm2m_resource_new(type == MULTIPLE_RESOURCE_TYPE ? true : false);
        parsed_resource->id = id;

        error = deserialize_lwm2m_resource(parsed_resource, message, value_length, TLV_FORMAT);
        if (error) {
            return NULL;
        }
        curr_buf += value_length;
    }
    return parsed_resources;
}

static int copy_and_free_all_resources(lwm2m_map *parsed_resources, lwm2m_node *node, lwm2m_node_type node_type) {
    lwm2m_map *subnodes = node_type == INSTANCE ? node->instance.resources : node->resource.resource.multiple.instances;

    int *keys = (int *) malloc(parsed_resources->size);
    for (int i = 0, id = keys[i]; i < parsed_resources->size; i++) {
        lwm2m_resource *real_resource = lwm2m_map_get(subnodes, id)->resource;
        lwm2m_resource *parsed_resource = lwm2m_map_get(parsed_resources, id)->resource;
        copy_value(parsed_resource, real_resource);
        free_lwm2m_resource(parsed_resource);
    }
    lwm2m_map_free(parsed_resources); //todo free everything also in serialize
    return 0;
}

static void copy_value(lwm2m_resource *source, lwm2m_resource *destination) {
    lwm2m_value source_value = source->resource.single.value;
    switch (destination->type) {
        case INTEGER:
            destination->resource.single.value.int_value = source_value.int_value;
            break;
        case DOUBLE:
            destination->resource.single.value.double_value = source_value.double_value;
            break;
        case STRING:
            destination->resource.single.value.string_value = source_value.string_value;
            destination->resource.single.length = source->resource.single.length;
            break;
        case OPAQUE:
            destination->resource.single.value.opaque_value = source_value.opaque_value;
            destination->resource.single.length = source->resource.single.length;
            break;
        case BOOLEAN:
            destination->resource.single.value.bool_value = source_value.bool_value;
            break;
        case LINK:
            destination->resource.single.value.link_value = source_value.link_value;
            break;
    }
}

static char *read_header(char *buf, int *identifier_length, int *value_length, int *length_of_length, int *type) {
    *type = buf & 0b11000000;
    *identifier_length = buf & 0b00100000 ? 16 : 8;
    *length_of_length = buf & 0b00011000;
    *value_length = buf & 0b00000111;
    return buf + 1;
}

static char *read_identifier(char *buf, int *id, int identifier_length) {
    if (identifier_length == 8) {
        *id = (int) buf;
        return buf + 1;
    }
    if (identifier_length == 16) {
        *id = 0;
        *id += buf[1] << 8;
        *id += buf[0];
        return buf + 2;
    }
    return NULL;
}

static char *read_length(char *buf, int length_of_length, int *value_length) {
    if (length_of_length == 8) {
        *value_length = (int) buf;
        return buf + 1;
    }
    if (length_of_length == 16) {
        *value_length = 0;
        *value_length += buf[1] << 8;
        *value_length += buf[0];
        return buf + 2;
    }
    if (length_of_length == 24) {
        *value_length = 0;
        *value_length += buf[2] << 16;
        *value_length += buf[1] << 8;
        *value_length += buf[0];
        return buf + 3;
    }
    if (length_of_length == 0) {
        // value_length is already read from header
        return buf;
    }
}

//////////////// DESERIALIZE SINGLE RESOURCES/RESOURCE INSTANCES ///////////////


static int deserialize_int(lwm2m_resource* resource, char* message, int message_len, int format) {
    // TODO
}

static int deserialize_double(lwm2m_resource* resource, char* message, int message_len, int format) {
    // TODO
}

static int deserialize_string(lwm2m_resource* resource, char* message, int message_len, int format) {
    memcpy(resource->resource.single.value.string_value, message, message_len);
    resource->resource.single.length = message_len; // TODO UTF-8?
}

static int deserialize_opaque(lwm2m_resource* resource, char* message, int message_len, int format) {
    memcpy(resource->resource.single.value.opaque_value, message, message_len);
    resource->resource.single.length = message_len;
}

static int deserialize_boolean(lwm2m_resource* resource, char* message, int message_len, int format) {
    resource->resource.single.value.bool_value = message[0] ? true : false;
}

static int deserialize_link(lwm2m_resource* resource, char* message, int message_len, int format) {
    resource->resource.single.value.link_value.object_id = (int) message[1];
    resource->resource.single.value.link_value.instance_id = (int) message[0];
}
