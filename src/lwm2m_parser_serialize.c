#include "lwm2m_parser.h"
#include "lwm2m_access_control.h"
#include "lwm2m_device_management.h"


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

//////////////// SERIALIZE MULTIPLE VALUES /////////////////

static lwm2m_map *get_nodes(lwm2m_node *parent_node, lwm2m_node_type type) {
    if (type == OBJECT) {
        return parent_node->object.instances;
    }
    if (type == INSTANCE) {
        return parent_node->instance.resources;
    }
    if (type == RESOURCE) {
        return parent_node->resource.resource.multiple.instances;
    }
    return NULL;
}


static char *create_type_header(char *buf, lwm2m_node *node, lwm2m_node_type type, int len) {
    char header = 0;

    if (type == OBJECT) {
        header |= node->instance.id < 0xFF ? 0b00100000 : 0b00000000;     // length of identifier (is 8-bits long?)
    }
    if (type == INSTANCE) {
        header |= node->resource.multiple ? 0b10000000 : 0b11000000;     // type
        header |= node->resource.id < 0xFF ? 0b00100000 : 0b00000000;     // length of identifier (is 8-bits long?)
    }
    if (type == RESOURCE) {
        header |= 0b01000000;                                            // type
        header |= node->resource.id < 0xFF ? 0b00100000 : 0b00000000;     // length of identifier (is 8-bits long?)
    }

    header |= len < 8 ? len : 0b00000000;    // length if < 8

    // Number of bits required for string "length" value
    if (get_int_bits(len) == 8) {
        header |= 0b00001000;
    }
    if (get_int_bits(len) == 16) {
        header |= 0b00010000;
    }
    if (get_int_bits(len) == 24) {
        header |= 0b00011000;
    }
    buf[0] = header;
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

    lwm2m_map *nodes = get_nodes(node, type);
    int *keys = (int *) malloc(nodes->size);
    for (int i = 0; i < nodes->size; i++) {
        int id = keys[i];
        lwm2m_node *node = (lwm2m_node*) lwm2m_map_get(nodes, id);

        if (type == OBJECT && !serialize_lwm2m_instance(server, &node->instance, &node_buf, &node_len)) {
            continue; // can fail when server is not owner of instance
        }
        if (type != OBJECT && !serialize_lwm2m_resource(server, &node->resource, &node_buf, &node_len, TLV_FORMAT)) {
            continue; // can fail when READ operation is not supported for resource or is EXECUTE resource
        }

        curr_buf = main_buf;
        curr_buf = create_type_header(curr_buf, node, type, node_len);
        curr_buf = create_identifier(curr_buf, node, type);
        curr_buf = create_length(curr_buf, node_len);
        memcpy(curr_buf, node_buf, node_len);

        *message_len = *message_len + (curr_buf - main_buf + node_len);
    }
    *message = main_buf;
    return 0;
}

////// SERIALIZE OBJECTS //////

int serialize_lwm2m_instance(lwm2m_server *server, lwm2m_instance *instance, char **message, int *message_len) {
    if (!lwm2m_check_instance_access_control(server, instance, READ)) {
        return OPERATION_NOT_SUPPORTED;
    }
    return serialize_lwm2m_node(server, (lwm2m_node *) instance, INSTANCE, message, message_len);
}

int serialize_lwm2m_object(lwm2m_server *server, lwm2m_object *object, char **message, int *message_len) {
    return serialize_lwm2m_node(server, (lwm2m_node *) object, OBJECT, message, message_len);
}

int serialize_lwm2m_resource(lwm2m_server* server, lwm2m_resource *resource, char **message, int *message_len, int format) {
    if (!lwm2m_check_resource_operation_supported(resource, READ)) {
        return OPERATION_NOT_SUPPORTED;
    }
    if (resource->multiple) {
        return serialize_lwm2m_node(server, (lwm2m_node *) resource, RESOURCE, message, message_len);
    }
    else {
        *message = serialize_lwm2m_value(resource->resource.single.value, resource->type, format);
        *message_len = strlen(*message);
    }
}
