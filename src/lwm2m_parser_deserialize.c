#include "lwm2m_parser.h"

#define OBJECT_INSTANCE_TYPE    0b00000000
#define RESOURCE_INSTANCE_TYPE  0b01000000
#define MULTIPLE_RESOURCE_TYPE  0b10000000
#define RESOURCE_TYPE           0b11000000



static char *read_header(char *buf, int *identifier_length, int *value_length, int *length_of_length, int *type) {
    *type = *buf & 0b11000000;
    *identifier_length = *buf & 0b00100000 ? 16 : 8;
    *length_of_length = *buf & 0b00011000;
    *value_length = *buf & 0b00000111;
    return buf + 1;
}

static char *read_identifier(char *buf, int *id, int identifier_length) {
    if (identifier_length == 8) {
        *id = (int) buf[0];
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
        *value_length = (int) buf[0];
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
    return NULL;
}

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

        lwm2m_resource *parsed_resource = lwm2m_resource_new(type == MULTIPLE_RESOURCE_TYPE);
        // TOODO get type of resource
        parsed_resource->id = id;

        error = deserialize_lwm2m_resource(parsed_resource, message, value_length, TLV_FORMAT);
        if (error) {
            return NULL;
        }
        curr_buf += value_length;
    }
    return parsed_resources;
}

static void copy_value(lwm2m_resource *source, lwm2m_resource *destination) {
    destination->resource.single.value = source->resource.single.value;
    if (source->type == STRING || source->type == OPAQUE) {
        destination->resource.single.length = source->resource.single.length;
    }
}

static int copy_and_free_all_resources(lwm2m_map *parsed_resources, lwm2m_node *node, lwm2m_node_type node_type) {
    lwm2m_map *subnodes = node_type == INSTANCE ? node->instance.resources : node->resource.resource.multiple.instances;

    int *keys = (int *) malloc(parsed_resources->size);
    for (int i = 0, id = keys[i]; i < parsed_resources->size; i++) {
        lwm2m_resource *real_resource = (lwm2m_resource *) lwm2m_map_get(subnodes, id);
        lwm2m_resource *parsed_resource = (lwm2m_resource *) lwm2m_map_get(parsed_resources, id);
        copy_value(parsed_resource, real_resource);
//        free_lwm2m_resource(parsed_resource); TODO implement
    }
    lwm2m_map_free(parsed_resources); //todo free everything also in serialize
    return 0;
}


////// DESERIALIZE OBJECTS //////

int deserialize_lwm2m_object(lwm2m_object *object, char *message, int message_len) {
    // TODO. called only in bootstrap
    return 0;
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
        lwm2m_map *parsed_resources = deserialize_lwm2m_node((lwm2m_node *) resource, message, message_len);
        if (parsed_resources == NULL) {
            return OPERATION_NOT_SUPPORTED;
        }
        return copy_and_free_all_resources(parsed_resources, (lwm2m_node *) resource, RESOURCE);
    }
    else {
        resource->resource.single.value = deserialize_lwm2m_value(message, resource->type, format);
        if (resource->type == STRING || resource->type == OPAQUE) {
            resource->resource.single.length = message_len;
        }
    }
}

lwm2m_map *deserialize_lwm2m_attributes(char *message) {
    lwm2m_map *parsed_attributes = lwm2m_map_new();
    int current_attribute = 0;

    char *attribute_string;
    while ((attribute_string = strtok(message, "&")) != NULL) {
        if (strlen(attribute_string) > 0) {
            char attribute_copy[10];
            memcpy(attribute_copy, attribute_string, strlen(attribute_string));
            char *attribute_name = strtok(attribute_copy, "=");
            char *attribute_value_string = strtok(attribute_copy, "=");

            lwm2m_type attribute_type = lwm2m_get_attribute_type(attribute_name);
            lwm2m_value attribute_value = deserialize_lwm2m_value(attribute_value_string, attribute_type, TEXT_FORMAT);

            lwm2m_attribute* attribute = (lwm2m_attribute*) malloc(sizeof(lwm2m_attribute));
            attribute->name = attribute_name;
            attribute->name_len = strlen(attribute_name);
            attribute->type = attribute_type;
            attribute->numeric_value = attribute_value;

            lwm2m_map_put_string(parsed_attributes, attribute_name, attribute);
        }
        current_attribute++;
    }
    return parsed_attributes;
}

