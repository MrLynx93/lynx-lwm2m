#ifndef LYNX_LWM2M_LWM2M_COMMON_H
#define LYNX_LWM2M_LWM2M_COMMON_H

typedef enum lwm2m_type lwm2m_type;
typedef union lwm2m_value lwm2m_value;
typedef struct lwm2m_link lwm2m_link;

enum lwm2m_type {
    INTEGER,
    DOUBLE,
    STRING,
    OPAQUE,
    BOOLEAN,
    NONE,
    LINK
};

union lwm2m_value {
    double double_value;
    int int_value;
    bool bool_value;
    lwm2m_link link_value;
    char* opaque_value;
    char* string_value;
};


#endif //LYNX_LWM2M_LWM2M_COMMON_H
