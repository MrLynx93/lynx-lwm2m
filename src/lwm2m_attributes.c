#include "../include/lwm2m_attributes.h"
#include "../include/lwm2m_common.h"
#include <stdlib.h>

lwm2m_attribute* lwm2m_create_attribute(char* name, int name_len, int access_mode, lwm2m_type type) {
    lwm2m_attribute *attribute = (lwm2m_attribute*) malloc(sizeof(lwm2m_attribute));
    attribute->access_mode = access_mode;
    attribute->type = type;
    attribute->name = (char*) malloc(sizeof(char) * name_len);
    strcpy(attribute->name, name);

    if (attribute->type == INTEGER) {
        attribute->numeric_value.int_value = 0;
    }
    if (attribute->type == DOUBLE) {
        attribute->numeric_value.double_value = 0.0;
    }
    return attribute;
}
