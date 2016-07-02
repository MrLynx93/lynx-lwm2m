#ifndef LYNX_LWM2M_SERVER_H
#define LYNX_LWM2M_SERVER_H

#include "include/lwm2m_object.h"

typedef struct lwm2m_server lwm2m_server;


struct lwm2m_server {
    lwm2m_object_tree* object_tree;
    lwm2m_object* server_object;

};

#endif //LYNX_LWM2M_SERVER_H
