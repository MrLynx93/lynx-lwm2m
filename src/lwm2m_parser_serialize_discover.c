#include "lwm2m.h"
#include "lwm2m_parser.h"


/////////////// CREATE STRINGS /////////////////////////////

// Creates only "</3/0/1>" for example
static char *create_resource_string(lwm2m_resource *resource) {
    char *buf = (char *) calloc(100, sizeof(char));
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
    char *buf = (char *) calloc(100, sizeof(char));
    char **attribute_names = (char **) malloc(sizeof(char) * 10 * attributes->size);
    lwm2m_map_get_keys_string(attributes, attribute_names);
    for (int i = 0; i < attributes->size; i++) {
        lwm2m_attribute *attribute = (lwm2m_attribute *) lwm2m_map_get_string(attributes, attribute_names[i]);
        strcat(buf, ";");
        strcat(buf, attribute->name);
        strcat(buf, "=");
        strcat(buf, serialize_lwm2m_value(attribute->numeric_value, attribute->type, TEXT_FORMAT));
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
        attributes = node->instance.attributes;
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

// Creates "</3/0/1>, <3/0/2>, <3/0/6>;dim=8,<3/0/7>;dim=8; gt=50;lt=42.2,<3/0/16>" for example
static char *create_instance_string_with_attributes(lwm2m_instance *instance) {
    char *buf = (char *) calloc(100, sizeof(char));
    int *resource_ids = (int *) malloc(sizeof(int) * instance->resources->size);
    lwm2m_map_get_keys(instance->resources, resource_ids);
    for (int i = 0, resource_id = resource_ids[i]; i < instance->resources->size; i++) {
        strcat(buf, create_resource_string_with_attributes(
                (lwm2m_resource *) lwm2m_map_get(instance->resources, resource_id)));
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
    char *buf = (char *) calloc(100, sizeof(char));
    int object_id = instance->object->id;

    int *resource_ids = (int *) malloc(sizeof(int) * instance->resources->size);
    lwm2m_map_get_keys(instance->resources, resource_ids);
    for (int i = 0, resource_id = resource_ids[i]; i < instance->resources->size; i++) {
        strcat(buf, create_resource_string((lwm2m_resource *) lwm2m_map_get(instance->resources, resource_id)));
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
    char *buf = (char *) calloc(100, sizeof(char));
    int object_id = object->id;

    int *instance_ids = (int *) malloc(sizeof(int) * object->instances->size);
    lwm2m_map_get_keys(object->instances, instance_ids);
    for (int i = 0, instance_id = instance_ids[i]; i < object->instances->size; i++) {
        strcat(buf, create_instance_string((lwm2m_instance *) lwm2m_map_get(object->instances, instance_id)));
        strcat(buf, ",");
    }
    // Remove last ","
    if (strlen(buf) > 0) {
        int length = strlen(buf);
        buf[length - 1] = '\0';
    }
    return buf;
}

////// SERIALIZE DISCOVER /////////

void serialize_lwm2m_object_discover(lwm2m_object *object, char **message) {
    char *buf = (char *) calloc(100, sizeof(char));
    char *attributes_string = create_attributes_string(object->attributes);
    char *object_string = create_object_string(object);

    strcat(*message, "</");
    strcat(*message, itoa(object->id));
    strcat(*message, ">;");
    strcat(*message, attributes_string);
    if (strlen(attributes_string) > 0) {
        strcat(*message, ",");
    }
    strcat(*message, object_string);

    free(attributes_string);
    free(object_string);
}

void serialize_lwm2m_instance_discover(lwm2m_instance *instance, char **message) {
    char *buf = (char *) calloc(100, sizeof(char));

    char *attributes_string = create_attributes_string(instance->attributes);
    char *instance_string_with_attributes = create_instance_string_with_attributes(instance);

    strcat(*message, "</");
    strcat(*message, itoa(instance->object->id));
    strcat(*message, "/");
    strcat(*message, itoa(instance->id));
    strcat(*message, ">;");
    if (strlen(attributes_string) > 0) {
        strcat(*message, ",");
    }
    strcat(*message, instance_string_with_attributes);

    free(attributes_string);
    free(instance_string_with_attributes);
}

void serialize_lwm2m_resource_discover(lwm2m_resource *resource, char **message) {
    strcpy(*message, create_resource_string_with_attributes(resource));
}

//////////////////////// PRIVATE ///////////////////////////////
