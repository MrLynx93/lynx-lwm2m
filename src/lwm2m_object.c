#include <stdlib.h>

#include "../include/lwm2m_object.h"
#include "../include/lwm2m_parser.h"
#include "../include/lwm2m.h"

void lwm2m_create_instance(lwm2m_server* server, lwm2m_object* object, int id, char* message) {
    lwm2m_instance* instance = (lwm2m_instance*) malloc(sizeof(lwm2m_instance));
    instance->id = id;
    instance->object = object;

    // if acl object do

    instance->resources = server->context->create_resources_callback(object->id);
    deserialize_lwm2m_instance(instance, message);
}


//void lwm2m_create_instance(int id, lwm2m_object* object) {
//
//
//}
//
//void lwm2m_remove_instance(lwm2m_object* object, int id) {
//
//}
