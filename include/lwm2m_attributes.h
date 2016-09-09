#ifndef LYNX_LWM2M_LWM2M_ATTRIBUTES_H
#define LYNX_LWM2M_LWM2M_ATTRIBUTES_H

#include "lwm2m_common.h"
#include "map.h"
#include <stdbool.h>

#define OPERATION_NOT_ALLOWED 1

// TODO utility functions to get specific attribute?

typedef struct lwm2m_attribute lwm2m_attribute;

lwm2m_map* lwm2m_attributes_new();

int lwm2m_check_attribute_access(lwm2m_attribute *attribute, int operation);

lwm2m_type lwm2m_get_attribute_type(char *attribute_name);

bool is_notify_attribute(char* attribute_name);

struct lwm2m_attribute {
    char* name;
    int name_len;
    int access_mode;
    lwm2m_type type;
    lwm2m_value numeric_value;
};

#endif //LYNX_LWM2M_LWM2M_ATTRIBUTES_H
