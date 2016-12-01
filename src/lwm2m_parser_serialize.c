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
    if (resource->value == NULL) {
        *message_len = 0;
        return;
    }

    if (resource->type == STRING) {
        *message_len = resource->length;
        strcpy(message, resource->value->string_value);
    }
    if (resource->type == OPAQUE) {
        *message_len = resource->length;
        memcpy(message, resource->value->opaque_value, (size_t) *message_len);
    }
    if (resource->type == INTEGER) {
        int val = resource->value->int_value;
        *message_len = get_int_bytes(val);
        memcpy(message, &val, *message_len);
    }
    if (resource->type == BOOLEAN) {
        int val = resource->value->bool_value ? 1 : 0;
        *message_len = 1;
        memcpy(message, &val, 1);
    }
    if (resource->type == DOUBLE) {
        serialize_double(resource->value->double_value, message, 8);
        *message_len = 8;
    }
    if (resource->type == LINK) {
        lwm2m_link link = resource->value->link_value;
        *message_len = 4;
        message[0] = (char) (link.object_id);
        message[1] = (char) (link.object_id >> 8);
        message[2] = (char) (link.instance_id);
        message[3] = (char) (link.instance_id >> 8);
    }
}

/**
 * TEXT FORMAT
 * Resource is already checked and READABLE
 */
void serialize_resource_text(lwm2m_resource *resource, char *message, int *message_len) {
    if (resource->value == NULL) {
        *message_len = 0;
        return;
    }

    if (resource->type == STRING) {
        *message_len = resource->length;
        strcpy(message, resource->value->string_value);
    }
    if (resource->type == OPAQUE) {
        *message_len = resource->length;
        memcpy(message, resource->value->opaque_value, (size_t) *message_len);
    }
    if (resource->type == INTEGER) {
        sprintf(message, "%d", resource->value->int_value);
        *message_len = (int) strlen(message);
    }
    if (resource->type == BOOLEAN) {
        message[0] = (char) (resource->value->bool_value ? '1' : '0');
        message[1] = '\0';
        *message_len = 1;
    }
    if (resource->type == DOUBLE) {
        sprintf(message, "%f", resource->value->double_value);
        *message_len = (int) strlen(message);
    }
    if (resource->type == LINK) {
        int object_id = resource->value->link_value.object_id;
        int instance_id = resource->value->link_value.instance_id;
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
void serialize_multiple_resource(list *resources, char *message, int *message_len) {
    char *value_buffer = malloc(sizeof(char) * 100);
    char *header_buffer = malloc(sizeof(char) * 20);
    int header_length, value_length;
    int total_length = 0;

    for (list_elem *elem = resources->first; elem != NULL; elem = elem->next) {
        // Serialize resource instance
        lwm2m_resource *resource_instance = elem->value;
        serialize_single_resource(resource_instance, value_buffer, &value_length);
        serialize_header(resource_instance->id, RESOURCE_INSTANCE_TYPE, value_length, header_buffer, &header_length);

        // Copy these to message
        memcpy(message, header_buffer, (size_t) header_length);
        memcpy(message + header_length, value_buffer, (size_t) value_length);
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
void serialize_instance(list *resources, char *message, int *message_len) {
    char *value_buffer = malloc(sizeof(char) * 1000);
    char *header_buffer = malloc(sizeof(char) * 20);
    int header_length, value_length;
    int total_length = 0;

    for (list_elem *elem = resources->first; elem != NULL; elem = elem->next) {
        lwm2m_resource *resource = elem->value;

        // Serialize resource
        if (resource->multiple) {
            serialize_multiple_resource(resource->instances, value_buffer, &value_length);
            serialize_header(resource->id, MULTIPLE_RESOURCE_TYPE, value_length, header_buffer, &header_length);

        } else {
            serialize_single_resource(resource, value_buffer, &value_length);
            serialize_header(resource->id, RESOURCE_TYPE, value_length, header_buffer, &header_length);
        }

        // Copy these to message
        memcpy(message, header_buffer, (size_t) header_length);
        memcpy(message + header_length, value_buffer, (size_t) value_length);
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
void serialize_object(list *instances, char *message, int* message_len) {
    char *value_buffer = malloc(sizeof(char) * 1000);
    char *header_buffer = malloc(sizeof(char) * 20);
    int header_length, value_length;
    int total_length = 0;

    for (list_elem *elem = instances->first; elem != NULL; elem = elem->next) {
        // Serialize instance
        lwm2m_instance *instance = elem->value;
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
