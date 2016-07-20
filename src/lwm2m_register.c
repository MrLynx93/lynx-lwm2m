#include "../include/lwm2m.h"
#include "../include/lwm2m_object.h"
#include "../include/lwm2m_register.h"

int on_register(lwm2m_server* server, lwm2m_register_message *register_message) {
    // TODO register is uplink operation, so this is probably garbage
}

int on_deregister(lwm2m_server* server) {
    // TODO this too
}