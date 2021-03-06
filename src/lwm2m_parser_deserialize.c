#include "lwm2m_parser.h"

#define OBJECT_INSTANCE_TYPE    0b00000000
#define RESOURCE_INSTANCE_TYPE  0b01000000
#define MULTIPLE_RESOURCE_TYPE  0b10000000
#define RESOURCE_TYPE           0b11000000

/**
 * Creates a resource using context->create_resources_callback, so resource has:
 * - name
 * - read/write/execute callbacks
 * - operations allowed
 * - multiple/single
 */
static lwm2m_resource* create_resource(lwm2m_object *object, int resource_id) {
    lwm2m_resource *resource = (lwm2m_resource *) malloc(sizeof(lwm2m_resource));
    memcpy(resource, object->resource_def + resource_id, sizeof(lwm2m_resource));
    return resource;
}

typedef struct tlv_header {
    int type;
    int length;
    int id;
} tlv_header;

static char* parse_tlv_header(char* input, tlv_header* header) {
    header->type = *input & 0b11000000;
    header->length = *input & 0b00000111;
    int id_len = *input & 0b00100000 ? 16 : 8;
    int length_len = (*input & 0b00011000) / 8;
    input = input + 1;

    // Read identifier
    if (id_len == 8) {
        header->id = (int) input[0];
        input = input + 1;
    }
    if (id_len == 16) {
        header->id = input[1] << 8;
        header->id += input[0];
        input = input + 2;
    }
    
    // Read length if not already set
    if (length_len == 1) {
        header->length = (int) input[0];
        input = input + 1;
    }
    if (length_len == 2) {
        header->length = (unsigned char) input[0] << 8;
        header->length += (unsigned char) input[1];
        input = input +2;
    }
    if (length_len == 3) {
        header->length = (unsigned char) input[0] << 16;
        header->length += (unsigned char) input[1] << 8;
        header->length += (unsigned char) input[2];
        input = input + 3;
    }
    return input;
}

static int parse_int(char *message, int message_len) {
    int int_value = 0;
    for (int i = 0; i < message_len; ++i) {
        int_value +=  (int) ((unsigned char) message[message_len - i - 1]) << (i * 8);
    }
    return int_value;
}

static double parse_double(const char *message, int message_len) {
    const char double_arr[] = {
            message[7],
            message[6],
            message[5],
            message[4],
            message[3],
            message[2],
            message[1],
            message[0]
    };
    double double_value = 0;

    if (message_len == 4) {
        float f;
        memcpy(&f, double_arr + 4, 4);
        double_value = (double) f;
    }
    if (message_len == 8) {
        memcpy(&double_value, double_arr, 8);
    }
    return double_value;
}

static lwm2m_value parse_value_text(char *message, int message_len, lwm2m_type type) {
    lwm2m_value value;
    if (type == STRING) {
        value.string_value = message;
    }
    if (type == OPAQUE) {
        value.opaque_value = message;
    }
    if (type == INTEGER) {
        value.int_value = atoi(message);
    }
    if (type == BOOLEAN) {
        value.int_value = message[0] == '1';
    }
    if (type == DOUBLE) {
        value.double_value = atof(message);
    }
    if (type == LINK) {
        // We don't care about breaking string because text format is used only in resource operation
        strtok(message, ":");
        value.link_value.object_id = atoi(message);
        strtok(message, ":");
        value.link_value.instance_id = atoi(message);
    }
    return value;
}

static lwm2m_value parse_value(char *message, int message_len, lwm2m_type type) {
    lwm2m_value value;

    switch(type) {
        case INTEGER: {
            value.int_value = parse_int(message, message_len);
            break;
        }
        case DOUBLE: {
            value.double_value = parse_double(message, message_len);
            break;
        }
        case OPAQUE:
        case STRING: {
            char *string_value = (char *) malloc((size_t) (message_len + 1));
            memcpy(string_value, message, (size_t) message_len);
            string_value[message_len] = '\0';
            value.string_value = string_value;
            break;
        }
        case BOOLEAN: {
            int int_value = parse_int(message, message_len);
            value.bool_value = int_value ? true : false;
            break;
        }
        case LINK: {
            lwm2m_link link;
            link.object_id = parse_int(message + 2, 2);
            link.instance_id = parse_int(message, 2);
            value.link_value = link;
            break;
        }
        default:
            value.int_value = -1;
            break;
    }
    return value;
}

// TODO in multiple resource map there should be map <ID, lwm2m_resource*>
list *parse_multiple_resource(lwm2m_object *object, int resource_id, char *message, int message_len) {
    list *resources = list_new();
    tlv_header resource_header;

    char *curr = message;
    while (curr < message + message_len) {
        curr = parse_tlv_header(curr, &resource_header);

        lwm2m_resource *resource_instance = create_resource(object, resource_id);
        resource_instance->id = resource_header.id;
        lwm2m_value value = parse_value(curr, resource_header.length, resource_instance->type);
        __set_value(resource_instance, &value, resource_header.length);

        ladd(resources, resource_instance->id, resource_instance);
        curr = curr + resource_header.length;
    }
    return resources;
}

// TODO String/TLV format
lwm2m_resource *parse_resource(lwm2m_object *object, int resource_id, char *message, int message_len) {
    lwm2m_resource *resource = create_resource(object, resource_id);
    if (resource->multiple) {
        resource->instances = parse_multiple_resource(object, resource_id, message, message_len);
    } else {
        lwm2m_value value = parse_value_text(message, message_len, resource->type);
        __set_value(resource, &value, message_len);
    }
    return resource;
}

/** Returns map of resources **/
list *parse_instance(lwm2m_object *object, char *message, int message_len) {
    list *resources = list_new();
    tlv_header resource_header;
    
    char *curr = message;
    while (curr < message + message_len) {
        curr = parse_tlv_header(curr, &resource_header);

        lwm2m_resource *resource = create_resource(object, resource_header.id);

        if (resource_header.type == MULTIPLE_RESOURCE_TYPE) {
            resource->instances = parse_multiple_resource(object, resource->id, curr, resource_header.length);
            resource->multiple = true;
        } else {
            lwm2m_value value = parse_value(curr, resource_header.length, resource->type);
            __set_value(resource, &value, resource_header.length);
        }
        ladd(resources, resource->id, resource);
        curr = curr + resource_header.length;
    }
    return resources;    
}

/** Returns map of instances **/
list *parse_object(lwm2m_object *object, char *message, int message_len) {
    list *instances = list_new();

    char *curr = message;
    while (curr < message + message_len) {
        tlv_header instance_header;
        curr = parse_tlv_header(curr, &instance_header);

        lwm2m_instance *instance = (lwm2m_instance *) malloc(sizeof(lwm2m_instance));
        instance->id = instance_header.id;
        instance->resources = parse_instance(object, curr, instance_header.length);

        ladd(instances, instance->id, instance);
        curr = curr + instance_header.length;
    }
    return instances;
}


void parse_attributes(lwm2m_attributes *attributes, char *message) {
    int count = 0;
    char *elems[6];
    char *buf = strtok(message, "&");

    while (buf != NULL) {
        elems[count++] = buf;
        buf = strtok(NULL, "&");
    }

    for (int i = 0; i < count; i++) {
        buf = strtok(elems[i], "=");

        if (!strcmp(buf, "dim")) {
            buf = strtok(NULL, "=");
            attributes->dim = (int*) malloc(sizeof(int));
            *attributes->dim = atoi(buf);
        }
        if (!strcmp(buf, "pmax")) {
            buf = strtok(NULL, "=");
            attributes->pmax = (int*) malloc(sizeof(int));
            *attributes->pmax = atoi(buf);
        }
        if (!strcmp(buf, "pmin")) {
            buf = strtok(NULL, "=");
            attributes->pmin = (int*) malloc(sizeof(int));
            *attributes->pmin = atoi(buf);
        }
        if (!strcmp(buf, "gt")) {
            buf = strtok(NULL, "=");
            attributes->gt = (float*) malloc(sizeof(float));
            *attributes->gt = (float) atof(buf);
        }
        if (!strcmp(buf, "lt")) {
            buf = strtok(NULL, "=");
            attributes->lt = (float*) malloc(sizeof(float));
            *attributes->lt = (float) atof(buf);
        }
        if (!strcmp(buf, "stp")) {
            buf = strtok(NULL, "=");
            attributes->stp = (float*) malloc(sizeof(float));
            *attributes->stp = (float) atof(buf);
        }
    }
}
