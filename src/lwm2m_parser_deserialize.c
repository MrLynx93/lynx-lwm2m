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

}

// TODO get rid of this
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

static lwm2m_resource* create_resource(lwm2m_context *context, int object_id, int resource_id) {
    if (object_id == SECURITY_OBJECT_ID || object_id == SERVER_OBJECT_ID || object_id == ACCESS_CONTROL_OBJECT_ID) {
        return lwm2m_map_get_resource(context->create_standard_resources_callback(object_id), resource_id);
    } else {
        return lwm2m_map_get_resource(context->create_resources_callback(object_id), resource_id);
    }
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
        header->length = (int) input[1] << 8;
        header->length += (int) input[0];
        input = input +2;
    }
    if (length_len == 3) {
        header->length = (int) input[2] << 16;
        header->length += (int) input[1] << 8;
        header->length += (int) input[0];
        input = input + 3;
    }
    return input;
}

static int parse_int(char *message, int message_len) {
    int int_value = 0;
    for (int i = 0; i < message_len; ++i) {
        int_value |= (message[message_len - i - 1] << (i * 8));
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
    // TODO :)
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
            link.object_id = parse_int(message + 2, 16);
            link.instance_id = parse_int(message, 16);
            value.link_value = link;
            break;
        }
    }
    return value;
}

// TODO in multiple resource map there should be map <ID, lwm2m_resource*>
lwm2m_map *parse_multiple_resource(lwm2m_context *context, int object_id, int resource_id, char *message, int message_len) {
    lwm2m_map *resources = lwm2m_map_new();
    tlv_header resource_header;

    char *curr = message;
    while (curr < message + message_len) {
        curr = parse_tlv_header(curr, &resource_header);

        lwm2m_resource *resource_instance = create_resource(context, object_id, resource_id);
        resource_instance->id = resource_header.id;
        resource_instance->resource.single.value = parse_value(curr, resource_header.length, resource_instance->type);

        lwm2m_map_put(resources, resource_instance->id, resource_instance);
        curr = curr + resource_header.length;
    }
    return resources;
}

// TODO String/TLV format
lwm2m_resource *parse_resource(lwm2m_context *context, int object_id, int resource_id, char *message, int message_len) {
    lwm2m_resource *resource = create_resource(context, object_id, resource_id);
    if (resource->multiple) {
        lwm2m_map *resource_instances = parse_multiple_resource(context, object_id, resource_id, message, message_len);
        resource->resource.multiple.instances = resource_instances;
    } else {
        lwm2m_value value = parse_value_text(message, message_len, resource->type);
        resource->resource.single.value = value;
        if (resource->type == STRING || resource->type == OPAQUE) {
            resource->resource.single.length = message_len;
        }
    }
    return resource;
}

/** Returns map of resources **/
lwm2m_map *parse_instance(lwm2m_context *context, int object_id, char *message, int message_len) {
    lwm2m_map *resources = lwm2m_map_new();
    tlv_header resource_header;
    
    char *curr = message;
    while (curr < message + message_len) {
        curr = parse_tlv_header(curr, &resource_header);

        lwm2m_resource *resource = create_resource(context, object_id, resource_header.id);

        if (resource_header.type == MULTIPLE_RESOURCE_TYPE) {
            lwm2m_map *resource_instances = parse_multiple_resource(context, object_id, resource->id, curr, resource_header.length);
            resource->resource.multiple.instances = resource_instances;

        } else {
            lwm2m_value value = parse_value(curr, resource_header.length, resource->type);
            resource->resource.single.value = value;
            if (resource->type == STRING || resource->type == OPAQUE) {
                resource->resource.single.length = resource_header.length;
            }
        }
        lwm2m_map_put(resources, resource->id, resource);
        curr = curr + resource_header.length;
    }
    return resources;    
}

/** Returns map of instances **/
lwm2m_map *parse_object(lwm2m_context *context, int object_id, char *message, int message_len) {
    lwm2m_map *instances = lwm2m_map_new();

    char *curr = message;
    while (curr < message + message_len) {
        tlv_header instance_header;
        curr = parse_tlv_header(curr, &instance_header);

        lwm2m_instance *instance = (lwm2m_instance *) malloc(sizeof(lwm2m_instance));
        instance->id = instance_header.id;
        instance->resources = parse_instance(context, object_id, curr, instance_header.length);

        lwm2m_map_put(instances, instance->id, instance);
        curr = curr + instance_header.length;
    }
    return instances;
}





















































/* Node can be instance or multiple resource */
static lwm2m_map *deserialize_lwm2m_node(lwm2m_node *node, char *message, int message_len) { // TODO header argument
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
        curr_buf = read_length(curr_buf, length_of_length, &value_length); // either reads length or leaves length from header

        lwm2m_resource *parsed_resource = lwm2m_resource_new(type == MULTIPLE_RESOURCE_TYPE);
        // TODO get type of resource
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
//    lwm2m_map *parsed_instances = deserialize_lwm2m_node()
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
        resource->resource.single.value = deserialize_lwm2m_value(message, message_len, resource->type, format);
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
            lwm2m_value attribute_value = deserialize_lwm2m_value(attribute_value_string, strlen(attribute_value_string), attribute_type, TEXT_FORMAT);

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

