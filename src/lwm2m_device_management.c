#include "../include/lwm2m_device_management.h"
#include "../include/lwm2m_access_control.h"
#include "../include/lwm2m_parser.h"
#include "../include/lwm2m_object.h"
#include "../include/lwm2m_attributes.h"
#include "../include/lwm2m.h"

///////////////////// READ /////////////////////////

int on_lwm2m_object_read(lwm2m_server *server, lwm2m_object *object, char **message) {
    serialize_lwm2m_object(object, message);
    return 0;
}

int on_lwm2m_instance_read(lwm2m_server *server, lwm2m_instance *instance, char **message) {
    int access_error = check_instance_access(server, instance, READ);
    if (access_error) {
        return access_error;
    }
    serialize_lwm2m_instance(instance);
    return 0;
}


int on_lwm2m_resource_read(lwm2m_server *server, lwm2m_resource *resource, char **message) {
    int access_error = check_resource_access(server, resource, READ);
    if (access_error) {
        return access_error;
    }
    resource->read_callback(resource);
    serialize_lwm2m_resource(resource, message);
    return 0;
}

///////////////////// WRITE ////////////////////////
// TODO also sending notification

int on_lwm2m_instance_write(lwm2m_server *server, lwm2m_instance *instance, char *message) {
    int access_error = check_instance_access(server, instance, WRITE);
    if (access_error) {
        return access_error;
    }
    deserialize_lwm2m_instance(instance, message);
    return 0;
}

int on_lwm2m_resource_write(lwm2m_server *server, lwm2m_resource *resource, char *message) {
    int access_error = check_resource_access(server, resource, WRITE);
    if (access_error) {
        return access_error;
    }
    deserialize_lwm2m_resource(resource, message);
    return 0;
}

///////////////////////// DELETE //////////////////////

int on_lwm2m_instance_delete(lwm2m_server *server, lwm2m_instance *instance) {
    int access_error = check_instance_access(server, resource, DELETE);
    if (access_error) {
        return access_error;
    }
    lwm2m_delete_instance(instance);
}

///////////////////////// CREATE //////////////////////

int on_lwm2m_instance_create(lwm2m_server *server, lwm2m_object *object, char *message, int id) {
    int access_error = check_object_access(server, object); // todo create aco instance
    if (access_error) {
        return access_error;
    }
    lwm2m_instance *instance = lwm2m_instance_new();
    deserialize_lwm2m_instance(instance, message);
    instance->aco_instance = lwm2m_instance_create_aco_instance(server, instance);
    instance->id = id;
}


///////////////////////// DISCOVER //////////////////////


int on_lwm2m_object_discover(lwm2m_server *server, lwm2m_object *object, char **message) {
    serialize_lwm2m_object_discover(object, message);
    return 0;
}

int on_lwm2m_instance_discover(lwm2m_server *server, lwm2m_instance *instance, char **message) {
    serialize_lwm2m_instance_discover(instance, message);
    return 0;
}

int on_lwm2m_resource_discover(lwm2m_server *server, lwm2m_resource *resource, char **message) {
    serialize_lwm2m_resource_discover(resource, message);
    return 0;
}


//////////////////// WRITE ATTRIBUTES ////////////////////


int on_lwm2m_object_write_attributes(lwm2m_server *server, lwm2m_object *object, char *message) {
    return on_lwm2m_node_write_attributes(server, object, OBJECT, message);
}

int on_lwm2m_instance_write_attributes(lwm2m_server *server, lwm2m_instance *instance, char *message) {
    return on_lwm2m_node_write_attributes(server, instance, INSTANCE, message);
}

int on_lwm2m_resource_write_attributes(lwm2m_server *server, lwm2m_resource *resource, char *message) {
    return on_lwm2m_node_write_attributes(server, resource, RESOURCE, message);
}


///////////////////////// PRIVATE /////////////////////

static int check_object_access(lwm2m_server *server, lwm2m_object *object) {
    if (!lwm2m_check_object_access_control(server, object)) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }
    return 0;
}

static int check_instance_access(lwm2m_server *server, lwm2m_instance *instance, int operation) {
    if (!lwm2m_check_instance_access_control(server, instance, READ)) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }
    return 0;
}

static int check_resource_access(lwm2m_server *server, lwm2m_resource *resource, int operation) {
    if (!lwm2m_check_instance_access_control(server, resource->instance, READ)) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }
    if (!lwm2m_check_resource_operation_supported(resource, operation)) {
        return OPERATION_NOT_SUPPORTED;
    }
    return 0;
}

static int on_lwm2m_node_write_attributes(lwm2m_server *server, lwm2m_node *node, lwm2m_type type, char *message) {
    lwm2m_map_string *parsed_attributes = deserialize_lwm2m_attributes(message);
    lwm2m_map_string *attributes = get_node_attributes(node, type);

    char **attribute_names = (char **) malloc(sizeof(char) * 10 * parsed_attributes->size);
    lwm2m_string_map_get_keys(parsed_attributes, attribute_names);
    for (int i = 0; i < parsed_attributes->size; i++) {
        lwm2m_attribute parsed_attribute = lwm2m_string_map_get(parsed_attributes, attribute_names[i]);
        lwm2m_attribute attribute = lwm2m_map_string_get(attributes, attribute_names[i]);

        if (!is_notify_attribute(parsed_attribute.name)) {
            return STRANGE_ERROR;
        }
        int error = lwm2m_check_attribute_access(parsed_attribute, READ);
        if (error) {
            return error;
        }

        if (attribute == NULL) {
            lwm2m_map_string_put(attributes, attribute_names[i], parsed_attribute);
        }
        else {
            attribute.numeric_value = parsed_attribute.numeric_value;
        }
    }
    return 0;
}

static lwm2m_map_string *get_node_attributes(lwm2m_node *node, lwm2m_type type) {
    if (type == OBJECT) {
        return node->object.attributes;
    }
    if (type == INSTANCE) {
        return node->instance.attributes;
    }
    if (type == RESOURCE) {
        return node->resource.attributes;
    }
    return NULL;
}