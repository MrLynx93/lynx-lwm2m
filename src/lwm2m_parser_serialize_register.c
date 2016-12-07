#include "lwm2m.h"
#include "lwm2m_parser.h"

///////////////// SERIALIZE OBJECTS AND INSTANCES ////////////////

static void create_object_string(lwm2m_object* object, char *message) {
    sprintf(message, "</%d>", object->id);
}

static void create_object_with_instances_string(lwm2m_object* object , char *message) {
    char buf[20] = {0};

    for (list_elem *elem = object->instances->first; elem != NULL; elem = elem->next) {
        sprintf(buf, "</%d/%d>", object->id, elem->key);
        strcat(message, buf);
        // If its not the last one, then append ","
        if (elem->next != NULL) {
            strcat(message, ",");
        }
    }
}

char *serialize_lwm2m_objects_and_instances(lwm2m_context *context) {
    char buffer[100] = {0};
    char *message = (char *) calloc(200, sizeof(char));

    for (list_elem *elem = context->object_tree->first; elem != NULL; elem = elem->next) {
        lwm2m_object *object = elem->value;

        if (object->instances->size == 0) {
            create_object_string(object, buffer);
            strcat(message, buffer);
        } else {
            create_object_with_instances_string(object, buffer);
            strcat(message, buffer);
        }
        // If its not the last one, then append ","
        if (elem->next != NULL) {
            strcat(message, ",");
        }
        buffer[0] = 0;
    }
    return message;
}