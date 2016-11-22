#include "lwm2m.h"
#include "lwm2m_parser.h"

// TODO SPRINTF?????

/////////////// CREATE STRINGS /////////////////////////////

static char *value_to_text(lwm2m_value *value, lwm2m_type type) {
    char *buffer = (char *) malloc(sizeof(char) * 20);

    if (type == INTEGER) {
        sprintf(buffer, "%d", value->int_value);
        return buffer;
    }
    if (type == DOUBLE) {
        sprintf(buffer, "%f", value->double_value);
        return buffer;
    }
    return NULL;
}

static char *create_resource_string_with_instance_id(lwm2m_resource *resource, int object_id, int instance_id) {
    char *buf = (char *) calloc(300, sizeof(char));
    strcat(buf, "</");
    strcat(buf, itoa(object_id));
    strcat(buf, "/");
    strcat(buf, itoa(instance_id));
    strcat(buf, "/");
    strcat(buf, itoa(resource->id));
    strcat(buf, ">");
    return buf;
}

// Creates only "</3/0/1>" for example
static char *create_resource_string(lwm2m_resource *resource) {
    char *buf = (char *) calloc(300, sizeof(char));
    strcat(buf, "</");
    strcat(buf, itoa(resource->instance->object->id));
    strcat(buf, "/");
    strcat(buf, itoa(resource->instance->id));
    strcat(buf, "/");
    strcat(buf, itoa(resource->id));
    strcat(buf, ">");
    return buf;
}

static char *create_attributes_string(lwm2m_map *attributes) {
    char *buf = (char *) calloc(300, sizeof(char));
    char **attribute_names = (char **) malloc(sizeof(char) * 10 * attributes->size);
    lwm2m_map_get_keys_string(attributes, attribute_names);
    for (int i = 0; i < attributes->size; i++) {
        lwm2m_attribute *attribute = (lwm2m_attribute *) lwm2m_map_get_string(attributes, attribute_names[i]);
        strcat(buf, ";");
        strcat(buf, attribute->name);
        strcat(buf, "=");
        strcat(buf, value_to_text(attribute->numeric_value, attribute->type));
    }
    return buf;
}

// Creates only ";ep=1;dim=8;gt=50;lt=42.2" for example
static char *create_node_attributes_string(lwm2m_node *node, lwm2m_node_type type) {
    lwm2m_map *attributes;
    if (type == OBJECT) {
        attributes = node->object.attributes;
    }
    if (type == INSTANCE) {
        attributes = node->instance.attributes;
    }
    if (type == RESOURCE) {
        attributes = node->resource.attributes;
    }
    return create_attributes_string(attributes);
}

// Creates "</3/2/7>;dim=8;pmin=10;pmax=60;gt=50;lt=42.2" for example
static char *create_resource_string_with_attributes(lwm2m_resource *resource) {
    char *resource_string = create_resource_string(resource);
    char *attributes_string = create_node_attributes_string((lwm2m_node *) resource, RESOURCE);
    strcat(resource_string, attributes_string);
    return resource_string;
}

static lwm2m_map* __create_resources(lwm2m_context *context, int object_id) {
    if (object_id == SECURITY_OBJECT_ID || object_id == SERVER_OBJECT_ID || object_id == ACCESS_CONTROL_OBJECT_ID) {
        return context->create_standard_resources_callback(object_id);
    } else {
        return context->create_resources_callback(object_id);
    }
}

// Creates "</3/0/1>, <3/0/2>, <3/0/6>;dim=8,<3/0/7>;dim=8; gt=50;lt=42.2,<3/0/16>" for example
static char *create_instance_string_with_attributes(lwm2m_instance *instance) {
    char *buf = (char *) calloc(300, sizeof(char));
    int keys[instance->resources->size];
    lwm2m_map_get_keys(instance->resources, keys);
    for (int i = 0; i < instance->resources->size; i++) {
        strcat(buf, create_resource_string_with_attributes((lwm2m_resource *) lwm2m_map_get(instance->resources, keys[i])));
        strcat(buf, ",");
    }
    // Remove last ","
    if (strlen(buf) > 0) {
        int length = strlen(buf);
        buf[length - 1] = '\0';
    }
    return buf;
}

// Creates only "</3/0/1>, <3/0/2>" for example
static char *create_instance_string(lwm2m_instance *instance) {
    char *buf = (char *) calloc(300, sizeof(char));

    int keys[instance->resources->size];
    lwm2m_map_get_keys(instance->resources, keys);
    for (int i = 0; i < instance->resources->size; i++) {
        strcat(buf, create_resource_string((lwm2m_resource *) lwm2m_map_get(instance->resources, keys[i])));
        strcat(buf, ",");
    }
    // Remove last ","
    if (strlen(buf) > 0) {
        int length = strlen(buf);
        buf[length - 1] = '\0';
    }
    return buf;
}

// Creates only "</3/0/1>, <3/0/2>, </3/1/3>, </3/1/4>" for example
static char *create_object_string(lwm2m_object *object) {
    char *buf = (char *) calloc(500, sizeof(char));


    lwm2m_map *resources = __create_resources(object->context, object->id);
    int keys[resources->size];
    lwm2m_map_get_keys(resources, keys);
    for (int i = 0; i < resources->size; i++) {
        strcat(buf, create_resource_string_with_instance_id(lwm2m_map_get_resource(resources, keys[i]), object->id, 0));
        strcat(buf, ",");
    }
    // Remove last ","
    if (strlen(buf) > 0) {
        int length = (int) strlen(buf);
        buf[length - 1] = '\0';
    }
    return buf;
}

////// SERIALIZE DISCOVER /////////

void serialize_lwm2m_object_discover(lwm2m_object *object, char *message) {
    char *buf = (char *) calloc(100, sizeof(char));
    char *attributes_string = create_attributes_string(object->attributes);
    char *object_string = create_object_string(object);

    strcat(message, "</");
    strcat(message, itoa(object->id));
    strcat(message, ">"); // todo or ">;"
    strcat(message, attributes_string);
    if (strlen(attributes_string) > 0) {
        strcat(message, ",");
    }
    strcat(message, object_string);

    free(attributes_string);
    free(object_string);
}

void serialize_lwm2m_instance_discover(lwm2m_instance *instance, char *message) {
    char *buf = (char *) calloc(300, sizeof(char));

    char *attributes_string = create_attributes_string(instance->attributes);
    char *instance_string_with_attributes = create_instance_string_with_attributes(instance);

    strcat(message, "</");
    strcat(message, itoa(instance->object->id));
    strcat(message, "/");
    strcat(message, itoa(instance->id));
    strcat(message, ">;");
    if (strlen(attributes_string) > 0) {
        strcat(message, ",");
    }
    strcat(message, instance_string_with_attributes);

    free(attributes_string);
    free(instance_string_with_attributes);
}

// TODO INCLUDE INHERITED ATTRIBUTES
void serialize_lwm2m_resource_discover(lwm2m_resource *resource, char *message) {
    strcpy(message, create_resource_string_with_attributes(resource));
}

//////////////////////// PRIVATE ///////////////////////////////
