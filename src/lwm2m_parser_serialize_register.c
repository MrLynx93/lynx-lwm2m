#include "lwm2m.h"
#include "lwm2m_parser.h"

///////////////// SERIALIZE OBJECTS AND INSTANCES ////////////////

static char *create_object_string(lwm2m_object* object) {
    char *buffer = (char *) calloc(10, sizeof(char));
    sprintf(buffer, "</%d>", object->id);
    return buffer;
}

static char *create_object_with_instances_string(lwm2m_object* object) {
    char *buffer = (char *) calloc(100, sizeof(char));
    char buf[20];

    int instance_ids[object->instances->size];
    lwm2m_map_get_keys(object->instances, instance_ids);

    for (int i = 0; i < object->instances->size; ++i) {
        sprintf(buf, "</%d/%d>", object->id, instance_ids[i]);
        strcat(buffer, buf);
        // If its not the last one, then append ","
        if (i < object->instances->size - 1) {
            strcat(buffer, ",");
        }
    }
    return buffer;
}

char *serialize_lwm2m_objects_and_instances(lwm2m_context *context) {
    char *buffer = (char *) calloc(200, sizeof(char));

    int object_ids[context->object_tree->size];
    lwm2m_map_get_keys(context->object_tree, object_ids);

    for (int i = 0; i < context->object_tree->size; ++i) {
        int object_id = object_ids[i];
        lwm2m_object *object = lwm2m_map_get_object(context->object_tree, object_id);

        if (object->instances->size == 0) {
            strcat(buffer, create_object_string(object));
        } else {
            strcat(buffer, create_object_with_instances_string(object));
        }
        // If its not the last one, then append ","
        if (i < context->object_tree->size - 1) {
            strcat(buffer, ",");
        }
    }
    return buffer;
}