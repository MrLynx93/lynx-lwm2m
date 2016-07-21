#include "../include/lwm2m_parser.h"
#include "../include/lwm2m_object.h"
#include "../include/lwm2m_access_control.h"
#include "../include/lwm2m_device_management.h"



//////////////// PUBLIC FUNCTIONS /////////////////


int serialize_lwm2m_instance(lwm2m_server *server, lwm2m_instance *instance, char **message, int *message_len) {
    if (!lwm2m_check_instance_access_control(server, node->instance, READ)) {
        return OPERATION_NOT_SUPPORTED;
    }
    return serialize_lwm2m_node(server, (lwm2m_node *) instance, INSTANCE, message, message_len);
}

int serialize_lwm2m_object(lwm2m_server *server, lwm2m_object *object, char **message, int *message_len) {
    return serialize_lwm2m_node(server, (lwm2m_node *) object, OBJECT, message, message_len);
}

int serialize_lwm2m_resource(lwm2m_resource *resource, char **message, int *message_len, int format) {
    if (!lwm2m_check_resource_operation_supported(resource, READ)) {
        return OPERATION_NOT_SUPPORTED;
    }
    if (resource->multiple) {
        return serialize_lwm2m_node((lwm2m_node *) resource, RESOURCE, message, message_len);
    }
    else {
        lwm2m_resource_single single = resource->resource.single;
        lwm2m_value value = single.value;
        switch (resource->type) {
            case INTEGER:
                return serialize_int(value.int_value, message, message_len, format);
            case DOUBLE:
                return serialize_double(value.double_value, message, message_len, format);
            case STRING:
                return serialize_string(value.string_value, single.length, message, message_len, format);
            case OPAQUE:
                return serialize_opaque(value.opaque_value, single.length, message, message_len, format);
            case BOOLEAN:
                return serialize_boolean(value.bool_value, message, message_len, format);
            case LINK:
                return serialize_link(value.link_value, message, message_len, format);
            default:
                return OPERATION_NOT_SUPPORTED;
        }
    }
}

//////////////// SERIALIZE MULTIPLE VALUES /////////////////


static int serialize_lwm2m_node(
        lwm2m_server *server,
        lwm2m_node *node,
        lwm2m_node_type type,
        char **message,
        int *message_len) {

    char *main_buf = (char *) malloc(sizeof(char) * 100);
    char *node_buf = (char *) malloc(sizeof(char) * 100);
    char *curr_buf;
    int node_len;

    *message_len = 0;

    lwm2m_map nodes = get_nodes(node, type);
    int *keys = (int *) malloc(nodes.size);
    for (int i = 0, id = keys[i]; i < nodes.size; i++) {
        lwm2m_node *node = lwm2m_map_get(nodes, id);

        if (type == OBJECT && !serialize_lwm2m_instance(server, node->instance, &node_buf, &node_len)) {
            continue; // can fail when server is not owner of instance
        }
        if (type != OBJECT && !serialize_lwm2m_resource(node->resource, &node_buf, &node_len, TLV_FORMAT)) {
            continue; // can fail when READ operation is not supported for resource or is EXECUTE resource
        }

        curr_buf = main_buf;
        curr_buf = create_type_header(curr_buff, node, type, node_len);
        curr_buf = create_identifier(curr_buff, node, type);
        curr_buf = create_length(curr_buf, node_len);
        memcpy(curr_buf, node_buf, node_len);

        *message_len = *message_len + (curr_buf - main_buf + node_len);
    }
    *message = main_buf;
    return 0;
}

static lwm2m_node *get_node(lwm2m_node *parent_node, lwm2m_node_type type) {
    if (type == OBJECT) {
        return node->object.instances;
    }
    if (type == INSTANCE) {
        return node->instance.resources;
    }
    if (type == RESOURCE) {
        return node->resource.resource.multiple.instances;
    }
    return NULL;
}

static char *create_type_header(char *buf, lwm2m_node *node, lwm2m_node_type type, int len) {
    char header = 0;

    if (type == OBJECT) {
        header |= node->instance.id < 0xFF ? b00100000 : b00000000;     // length of identifier (is 8-bits long?)
    }
    if (type == INSTANCE) {
        header |= node->resource->multiple ? b10000000 : b11000000;     // type
        header |= node->resource.id < 0xFF ? b00100000 : b00000000;     // length of identifier (is 8-bits long?)
    }
    if (type == RESOURCE) {
        header |= b01000000;                                            // type
        header |= node->resource.id < 0xFF ? b00100000 : b00000000;     // length of identifier (is 8-bits long?)
    }

    header |= len < 8 ? len : b00000000;    // length if < 8

    // Number of bits required for string "length" value
    if (get_int_bits(len) == 8) {
        header |= b00001000;
    }
    if (get_int_bits(len) == 16) {
        header |= b00010000;
    }
    if (get_int_bits(len) == 24) {
        header |= b00011000;
    }
    *buf[0] = header;
    return buf + 1;
}

static char *create_identifier(char *buf, lwm2m_node *node, lwm2m_node_type type) {
    int id = type == OBJECT ? node->instance.id : node->resource.id;

    if (id < 0xFF) {
        buf[0] = (char) id;
        return buf + 1;
    }
    else {
        buf[0] = (char) id;
        buf[1] = (char) id >> 8;
        return buf + 2;
    }
}

static char *create_length(char *buf, int len) {
    if (len < 8) {
        return buf;
    }
    if (get_int_bits(len) == 8) {
        buf[0] = (char) len;
        return buf + 1;
    }
    if (get_int_bits(len) == 16) {
        buf[0] = (char) len;
        buf[1] = (char) len >> 8;
        return buf + 2;
    }
    if (get_int_bits(len) == 24) {
        buf[0] = (char) len;
        buf[1] = (char) len >> 8;
        buf[2] = (char) len >> 16;
        return buf + 3;
    }
    return NULL;
}

//////////////// SERIALIZE VALUES /////////////////


static int serialize_int(int value, char **message, int *message_len, int format) {
    // TODO
}

static void serialize_double(int value, char **message, int *message_len, int format) {
    // TODO
}

static void serialize_boolean(int value, char **message, int *message_len, int format) {
    // TODO
}

static void serialize_link(int value, char **message, int *message_len, int format) {
    // TODO
}

static void serialize_string(int value, int value_len, char **message, int *message_len, int format) {
    // TODO
}

static void serialize_opaque(int value, int value_len, char **message, int *message_len, int format) {
    // TODO
}

//////////////// HELPER FUNCTIONS /////////////////


static int get_int_bits(int number) {
    // TODO
}

static int get_int_digits(int number) {
    int digits = 0;
    while (number) {
        digits++;
        number /= 10;
    }
    return digits;
}