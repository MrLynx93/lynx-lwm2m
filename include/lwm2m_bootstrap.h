#ifndef LYNX_LWM2M_BOOTSTRAP_H
#define LYNX_LWM2M_BOOTSTRAP_H

#include "lwm2m.h"
#include "lwm2m_parser.h"


int on_bootstrap_object_write(lwm2m_server* server, lwm2m_object* object, char* message);
int on_bootstrap_instance_write(l2m2m_server* server, lwm2m_instance* object, char* message);
int on_bootstrap_resource_write(lwm2m_server* server, lwm2m_resource* resource, char* message);
int on_bootstrap_finish(lwm2m_server* server);


#endif //LYNX_LWM2M_BOOTSTRAP_H
