#include "lwm2m.h"
#include "lwm2m_parser.h"

#define OBJECT_INSTANCE_TYPE    0b00000000
#define RESOURCE_INSTANCE_TYPE  0b01000000
#define MULTIPLE_RESOURCE_TYPE  0b10000000
#define RESOURCE_TYPE           0b11000000


static int get_int_bytes(int value) {
    if (value < 0xFF) {
        return 1;
    }
    if (value < 0xFFFF) {
        return 2;
    }
    if (value < 0xFFFFFF) {
        return 3;
    }
    if (value < 0xFFFFFFFF) {
        return 4;
    }
    return 8;
}

static union double_bytes {
    double val;
    char bytes[8];
} double_bytes;

static void serialize_header(int id, int type, int value_len, char *message, int *message_len) {
    message[0] = 0;
    message[0] |= type;
    message[0] |= id < 0xFF ? 0b00000000 : 0b00100000;
    message[0] |= value_len < 8 ? value_len : 0x0;

    int bytes_required = get_int_bytes(value_len);

    char *length;
    // Identifier
    if (id < 0xFF) {
        message[1] = (char) id;
        length = message + 2;
        *message_len = 2;
    } else {
        message[1] = (char) id;
        message[2] = (char) (id >> 8);
        length = message + 3;
        *message_len = 3;
    }


    // Length type
    if (value_len < 4) {
        message[0] |= (value_len & 0b00000111);
    } else {
        message[0] |= (bytes_required << 3);

        // Length field
        if (bytes_required == 1) {
            length[0] = (char) value_len;
            *message_len += 1;
        }
        if (bytes_required == 2) {
            length[0] = (char) value_len;
            length[1] = (char) value_len >> 8;
            *message_len += 2;
        }
        if (bytes_required == 3) {
            length[0] = (char) value_len;
            length[1] = (char) value_len >> 8;
            length[2] = (char) value_len >> 16;
            *message_len += 3;
        }
    }
};

static void serialize_double(double value, const char *message, int message_len) {
    memcpy((void *) message, &value, 8);
}

static void serialize_value(lwm2m_resource *resource, char *message, int *message_len) {
    if (resource->type == STRING) {
        *message_len = resource->resource.single.length;
        memcpy(message, resource->resource.single.value.string_value, *message_len);
    }
    if (resource->type == OPAQUE) {
        *message_len = resource->resource.single.length;
        memcpy(message, resource->resource.single.value.opaque_value, *message_len);
    }
    if (resource->type == INTEGER) {
        int val = resource->resource.single.value.int_value;
        *message_len = get_int_bytes(val);
        memcpy(message, &val, *message_len);
    }
    if (resource->type == BOOLEAN) {
        int val = resource->resource.single.value.bool_value ? 1 : 0;
        *message_len = 1;
        memcpy(message, &val, 1);
    }
    if (resource->type == DOUBLE) {
        serialize_double(resource->resource.single.value.double_value, message, 8);
        *message_len = 8;
    }
    if (resource->type == LINK) {
        lwm2m_link link = resource->resource.single.value.link_value;
        *message_len = 4;
        message[0] = (char) (link.object_id);
        message[1] = (char) (link.object_id >> 8);
        message[2] = (char) (link.instance_id);
        message[3] = (char) (link.instance_id >> 8);
    }
}

//char *serialize_multiple_resource(lwm2m_resource *resource, int *message_len) {
//
//}

/**
 * TEXT FORMAT
 * Resource is already checked and READABLE
 */
void serialize_resource_text(lwm2m_resource *resource, char *message, int *message_len) {
    if (resource->type == STRING) {
        *message_len = resource->resource.single.length;
        memcpy(message, resource->resource.single.value.string_value, *message_len);
    }
    if (resource->type == OPAQUE) {
        *message_len = resource->resource.single.length;
        memcpy(message, resource->resource.single.value.opaque_value, *message_len);
    }
    if (resource->type == INTEGER) {
        sprintf(message, "%d", resource->resource.single.value.int_value);
        *message_len = (int) strlen(message);
    }
    if (resource->type == BOOLEAN) {
        message[0] = (char) (resource->resource.single.value.bool_value ? '1' : '0');
        message[1] = '\0';
        *message_len = 1;
    }
    if (resource->type == DOUBLE) {
        sprintf(message, "%f", resource->resource.single.value.double_value);
        *message_len = (int) strlen(message);
    }
    if (resource->type == LINK) {
        int object_id = resource->resource.single.value.link_value.object_id;
        int instance_id = resource->resource.single.value.link_value.instance_id;
        sprintf(message, "%d:%d", object_id, instance_id);
        *message_len = (int) strlen(message);
    }
}

/**
 * TLV FORMAT
 * Resource is already checked and READABLE
 */
void serialize_single_resource(lwm2m_resource *resource, char *message, int *message_len) {
    serialize_value(resource, message, message_len);
}

/**
 * TLV FORMAT
 * Resource is already checked and READABLE
 */
void serialize_multiple_resource(lwm2m_map *resources, char *message, int *message_len) {
    char *value_buffer = malloc(sizeof(char) * 100);
    char *header_buffer = malloc(sizeof(char) * 20);
    int header_length, value_length;
    int total_length = 0;

    int keys[resources->size];
    lwm2m_map_get_keys(resources, keys);
    for (int i = 0; i < resources->size; ++i) {
        // Serialize resource instance
        lwm2m_resource *resource_instance = lwm2m_map_get_resource(resources, keys[i]);
        serialize_single_resource(resource_instance, value_buffer, &value_length);
        serialize_header(resource_instance->id, RESOURCE_INSTANCE_TYPE, value_length, header_buffer, &header_length);

        // Copy these to message
        memcpy(message, header_buffer, header_length);
        memcpy(message + header_length, value_buffer, value_length);
        message += header_length + value_length;
        total_length += header_length + value_length;

    }
    free(value_buffer);
    free(header_buffer);
    *message_len = total_length;
}

/**
 * TLV FORMAT
 * 1. Instance is already granted READ access control for reading server
 * 2. Resources are already filtered out so they support READ operation
 */
void serialize_instance(lwm2m_map *resources, char *message, int *message_len) {
    char *value_buffer = malloc(sizeof(char) * 1000);
    char *header_buffer = malloc(sizeof(char) * 20);
    int header_length, value_length;
    int total_length = 0;

    int keys[resources->size];
    lwm2m_map_get_keys(resources, keys);
    for (int i = 0; i < resources->size; ++i) {
        lwm2m_resource *resource = lwm2m_map_get_resource(resources, keys[i]);

        // Serialize resource
        if (resource->multiple) {
            serialize_multiple_resource(resource->resource.multiple.instances, value_buffer, &value_length);
            serialize_header(resource->id, MULTIPLE_RESOURCE_TYPE, value_length, header_buffer, &header_length);

        } else {
            serialize_single_resource(resource, value_buffer, &value_length);
            serialize_header(resource->id, RESOURCE_TYPE, value_length, header_buffer, &header_length);
        }

        // Copy these to message
        memcpy(message, header_buffer, header_length);
        memcpy(message + header_length, value_buffer, value_length);
        message += header_length + value_length;
        total_length += header_length + value_length;
    }
    free(value_buffer);
    free(header_buffer);
    *message_len = total_length;
}

/**
 * TLV FORMAT
 * Map of instances that are already:
 * 1. Granted READ access control for reading server
 * 2. Resources are already filtered out so they support READ operation
 */
void serialize_object(lwm2m_map *instances, char *message, int* message_len) {
    char *value_buffer = malloc(sizeof(char) * 1000);
    char *header_buffer = malloc(sizeof(char) * 20);
    int header_length, value_length;
    int total_length = 0;

    int instance_len;
    int keys[instances->size];
    lwm2m_map_get_keys(instances, keys);
    for (int i = 0; i < instances->size; ++i) {
        // Serialize instance
        lwm2m_instance *instance = lwm2m_map_get_instance(instances, keys[i]);

        serialize_instance(instance->resources, value_buffer, &value_length);
        serialize_header(instance->id, OBJECT_INSTANCE_TYPE, value_length, header_buffer, &header_length);

        // Copy these to message
        memcpy(message, header_buffer, header_length);
        memcpy(message + header_length, value_buffer, value_length);
        message += header_length + value_length;
        total_length += header_length + value_length;
    }
    free(value_buffer);
    free(header_buffer);
    *message_len = total_length;
}
