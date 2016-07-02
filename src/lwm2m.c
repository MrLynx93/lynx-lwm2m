#include "../include/lwm2m.h"
#include "../include/lwm2m_object.h"
#include "../include/lwm2m_attributes.h"
#include "../include/lwm2m_device_management.h"

#define SECURITY_OBJECT_ID 0
#define SERVER_OBJECT_ID 1
#define ACCESS_CONTROL_OBJECT_ID 2


int lwm2m_get_number_of_servers(lwm2m_context* context) {
    return 1;
}



lwm2m_object** create_standard_objects() {
    // Define security object
    lwm2m_object* security_object = lwm2m_object_new();
    security_object->id = SECURITY_OBJECT_ID;
    security_object->multiple = true;
    security_object->object_urn = "urn:oma:lwm2m:oma:0";
    security_object->attributes = lwm2m_attributes_new();

    // Define server object
    lwm2m_object* server_object = lwm2m_object_new();
    server_object->id = SERVER_OBJECT_ID;
    server_object->multiple = true;
    server_object->object_urn = "urn:oma:lwm2m:oma:1";
    server_object->attributes = lwm2m_attributes_new();

    // Define access control object
    lwm2m_object* access_control_object = lwm2m_object_new();
    access_control_object->id = ACCESS_CONTROL_OBJECT_ID;
    access_control_object->multiple = true;
    access_control_object->object_urn = "urn:oma:lwm2m:oma:2";
    access_control_object->attributes = lwm2m_attributes_new();

    // Create list with objects (todo map?)
    lwm2m_object** objects = malloc(sizeof(lwm2m_object*) * 3);
    objects[0] = security_object;
    objects[1] = server_object;
    objects[2] = access_control_object;
    return objects;
}

lwm2m_resource** create_standard_resources(int object_id) {
    switch (object_id) {
        case SECURITY_OBJECT_ID:
            return create_security_object_resources();
        case SERVER_OBJECT_ID:
            return create_server_object_resources();
        case ACCESS_CONTROL_OBJECT_ID:
            return create_access_control_object_resources();
    }
    return NULL;
}


static lwm2m_resource** create_security_object_resources() {
    lwm2m_resource** resources = (lwm2m_resource**) malloc(sizeof(lwm2m_resource*) * 13);
    lwm2m_resource *resource;

    resource = lwm2m_resource_new(false);
    resource->id = 0;
    resource->name = "LWM2M Server URI";
    resource->type = STRING;
    resource->mandatory = true;
    resources[0] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 1;
    resource->name = "Bootstrap Server";
    resource->type = BOOLEAN;
    resource->mandatory = true;
    resources[1] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 2;
    resource->name = "Security Mode";
    resource->type = INTEGER;
    resource->mandatory = true;
    resources[2] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 3;
    resource->name = "Public Key or Identity";
    resource->type = OPAQUE;
    resource->mandatory = true;
    resources[3] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 4;
    resource->name = "Server Public Key";
    resource->type = OPAQUE;
    resource->mandatory = true;
    resources[4] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 5;
    resource->name = "Secret Key";
    resource->type = OPAQUE;
    resource->mandatory = true;
    resources[5] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 6;
    resource->name = "SMS Security Mode";
    resource->type = INTEGER;
    resource->mandatory = false;
    resources[6] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 7;
    resource->name = "SMS Key Binding Parameters";
    resource->type = OPAQUE;
    resource->mandatory = false;
    resources[7] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 8;
    resource->name = "SMS Binding Secret Key(s)";
    resource->type = OPAQUE;
    resource->mandatory = false;
    resources[8] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 9;
    resource->name = "LWM2M Server SMS Number";
    resource->type = STRING;
    resource->mandatory = false;
    resources[9] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 10;
    resource->name = "Short Server ID";
    resource->type = INTEGER;
    resource->mandatory = false;
    resources[10] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 11;
    resource->name = "Client Hold Off Time";
    resource->type = INTEGER;
    resource->mandatory = false;
    resources[11] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 12;
    resource->name = "Bootstrap Server Account Timeout";
    resource->type = INTEGER;
    resource->mandatory = false;
    resources[12] = resource;

    // Can be modified only in bootstrap
    for (int i = 0; i < 13; i++) {
        resources[i]->operations = 0;
    }

    return resources;
}

static lwm2m_resource** create_server_object_resources() {
    lwm2m_resource** resources = (lwm2m_resource**) malloc(sizeof(lwm2m_resource*) * 19);
    lwm2m_resource *resource;

    resource = lwm2m_resource_new(false);
    resource->id = 0;
    resource->name = "Short Server ID";
    resource->type = INTEGER;
    resource->mandatory = true;
    resource->operations = READ;
    resources[0] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 1;
    resource->name = "Lifetime";
    resource->type = INTEGER;
    resource->mandatory = true;
    resource->operations = READ & WRITE;
    resources[1] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 2;
    resource->name = "Default Minimum Period";
    resource->type = INTEGER;
    resource->mandatory = false;
    resource->operations = READ & WRITE;
    resources[2] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 3;
    resource->name = "Default Maximum Period";
    resource->type = INTEGER;
    resource->mandatory = false;
    resource->operations = READ & WRITE;
    resources[3] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 4;
    resource->name = "Disable";
    resource->type = NONE;
    resource->mandatory = false;
    resource->operations = EXECUTE;
    resources[4] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 5;
    resource->name = "Disable Timeout";
    resource->type = INTEGER;
    resource->mandatory = false;
    resource->operations = READ & WRITE;
    resources[5] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 6;
    resource->name = "Notification Storing When Disabled or Offline";
    resource->type = BOOLEAN;
    resource->mandatory = true;
    resource->operations = READ & WRITE;
    resources[6] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 7;
    resource->name = "Binding";
    resource->type = STRING;
    resource->mandatory = true;
    resource->operations = READ & WRITE;
    resources[7] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 8;
    resource->name = "Registration Update Trigger";
    resource->type = NONE;
    resource->mandatory = true;
    resource->operations = EXECUTE;
    resources[8] = resource;

    return resources;
}

static lwm2m_resource** create_access_control_object_resources() {
    lwm2m_resource** resources = (lwm2m_resource**) malloc(sizeof(lwm2m_resource*) * 4);
    lwm2m_resource *resource;

    resource = lwm2m_resource_new(false);
    resource->id = 0;
    resource->name = "Object ID";
    resource->type = INTEGER;
    resource->mandatory = true;
    resource->operations = READ;
    resources[0] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 1;
    resource->name = "Object Instance ID";
    resource->type = INTEGER;
    resource->mandatory = true;
    resource->operations = READ;
    resources[1] = resource;

    resource = lwm2m_resource_new(true);
    resource->id = 2;
    resource->name = "ACL";
    resource->type = INTEGER;
    resource->mandatory = false;
    resource->operations = READ & WRITE;
    resources[2] = resource;

    resource = lwm2m_resource_new(false);
    resource->id = 3;
    resource->name = "Access Control Owner";
    resource->type = INTEGER;
    resource->mandatory = true;
    resource->operations = READ & WRITE;
    resources[3] = resource;

    return resources;
}