#ifndef LYNX_LWM2M_LWM2M_ATTRIBUTES_H
#define LYNX_LWM2M_LWM2M_ATTRIBUTES_H

#include "lwm2m_common.h"

// TODO utility functions to get specific attribute?

typedef struct lwm2m_attribute lwm2m_attribute;
typedef struct lwm2m_attributes lwm2m_attributes;

lwm2m_attributes* lwm2m_attributes_new();

struct lwm2m_attribute {
    char* name;
    int name_len;
    int access_mode;
    lwm2m_type type;
    lwm2m_value numeric_value;
};

struct lwm2m_attributes {
    lwm2m_attribute* attributes;
};

#endif //LYNX_LWM2M_LWM2M_ATTRIBUTES_H
