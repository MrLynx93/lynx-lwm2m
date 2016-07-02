#include "../include/lwm2m_information_reporting.h"


int on_observe(lwm2m_server* server, lwm2m_resource* resource) {
    int access_error = check_resource_access(server, resource, READ);
    if (access_error) {
        return access_error;
    }
    start_observing(server, resource);
}

int cancel_observation(lwm2m_server* server, lwm2m_resource *resource) {
    stop_observing(server, resource);
}

//////////////////////// PRIVATE ////////////////////////

static int check_access(lwm2m_server* server, lwm2m_resource* resource, int operation) {
    if (!lwm2m_check_resource_access_control(server, resource, READ)) {
        return ACCESS_RIGHT_PERMISSON_DENIED;
    }
    if (!resource->operations & READ) {
        return OPERATION_NOT_SUPPORTED;
    }
    return 0;
}

