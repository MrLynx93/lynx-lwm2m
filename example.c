#include "include/lwm2m.h"

/*
 * This example shows creating LWM2M client, which supports example object.
 * What is happening:
 * 1. create_objects_callback and create_resources_callback are assigned.
 *    This is a way of defining a custom LWM2M object
 *
 * 2. lwm2m_start is called.
 *    This starts transport layer and creates lwm2m_object objects according to create_objects_callback
 *    Besides, it creates standard objects like "security", which don't have to be defined by user.
 *
 * 3. lwm2m_factory_bootstrap is called with bootstrapping callback as argument
 *    Bootstrapping callback should be used to create instances of LWM2M objects
 *
 *
 * Also you may note 3 functions are defined:
 * a. measure_battery_level - this function will be called when LWM2M read operation is performed on resource. It is
 *                            called before parsing of resource, so that value of resource can be updated.
 *                            It is set as read_callback of "Battery Level" resource (in create_resources_callback)
 *
 * b. switch_light          - this function will be called when LWM2M write operation is performed on resource. It is
 *                            called after deserializing write payload, so that new value can be used by callback.
 *                            It is set as write_callback of "Light on" resource (in create_resources_callback)
 *
 * c. update_firmwire       - this function will be called when LWM2M execute operation is performed on resource.
 *                            It is set as execute_callback of "Firmwire update" resource (in create_resources_callback)
 *
 *
 * Example object:
 * +------------+------------------------------+
 * | Name       |   Example                    |
 * | ID         |   123                        |
 * | Instances  |   multiple                   |
 * | Mandatory  |   false                      |
 * | Object     |   URN urn:oma:lwm2m:oma:123  |
 * +------------+------------------------------+
 *
 * Resources:
 * +----+-------------------------------------------+------------+-----------+-----------+---------+
 * |ID  | Name                                      | Operations | Instances | Mandatory | Type    |
 * +----+-------------------------------------------+------------+-----------+-----------+---------+
 * |0   | Battery level                             | RW         | Single    | Mandatory | Integer |
 * |1   | Double example resource                   | RW         | Single    | Mandatory | Double  |
 * |2   | String example resource                   | RW         | Single    | Mandatory | String  |
 * |3   | Light on                                  | RW         | Single    | Mandatory | Boolean |
 * |4   | Opaque example resource                   | RW         | Single    | Mandatory | Opaque  |
 * |5   | Firmwire update resource                  | E          | Single    | Mandatory |         |
 * |6   | Link example resource                     | RW         | Single    | Mandatory | ObjInk  |
 * |7   | Not mandatory integer example resource    | RW         | Single    | Optional  | Integer |
 * |8   | Multiple string example resource          | RW         | Multiple  | Mandatory | String  |
 * |9   | Readonly string example resource          | R          | Single    | Mandatory | String  |
 * +----+-------------------------------------------+------------+-----------+-----------+---------+
 *
 */

#define SECURITY_MODE_NOSEC 3

lwm2m_object **create_example_objects();
lwm2m_resource **create_example_resources(int object_id);
void perform_bootstrap(lwm2m_context *context);

int main(int argc, char *argv[]) {
    lwm2m_context *context = lwm2m_create_context();
    context->create_objects_callback = create_example_objects;
    context->create_resources_callback = create_example_resources;

    lwm2m_start(context);
    lwm2m_factory_bootstrap(context, perform_bootstrap);
    // This should not exit, as new threads are created
}


///////////////// DEFINITION OF RESOURCE CALLBACKS //////////////

void measure_battery_level(lwm2m_resource* resource) {
    int battery_level = 20; // On real device this would be read of some sensor
    resource->resource.single.value.int_value = battery_level;
}

void switch_light(lwm2m_resource *resource) {
    bool light_turned_on = resource->resource.single.value.bool_value;
    if (light_turned_on) {
        // On real device you would call turning light on here
    } else {
        // On real device you would call turning light off here
    }
}

void update_firmwire(lwm2m_resource *resource, char *args) {
    char* version = 1.2; // In reality you would parse it from args
    // In this place you would call your method
}


///////////////// DEFINITION OF FACTORY BOOTSTRAP ///////////////

void perform_bootstrap(lwm2m_context *context) {
    bootstrap_security_object(context);
    bootstrap_server_object(context);
}

void bootstrap_security_object(lwm2m_context *context) {
    lwm2m_object *security_object = context->object_tree->objects.get(0);
    lwm2m_instance *security_instance = lwm2m_instance_new(security_object, 0);

    lwm2m_resource_real server_uri = security_instance->resources[0]->resource;
    lwm2m_resource_real bootstrap_server = security_instance->resources[1]->resource;
    lwm2m_resource_real security_mode = security_instance->resources[2]->resource;
    lwm2m_resource_real public_key = security_instance->resources[3]->resource;
    lwm2m_resource_real server_public_key = security_instance->resources[4]->resource;
    lwm2m_resource_real short_server_id = security_instance->resources[5]->resource;

    server_uri.single.value.string_value = "localhost:123";
    bootstrap_server.single.value.bool_value = true;
    security_mode.single.value.int_value = SECURITY_MODE_NOSEC;
    public_key.single.value.opaque_value = NULL;
    server_public_key.single.value.opaque_value = NULL;
    short_server_id.single.value.int_value = 1;
}

void bootstrap_server_object(lwm2m_context *context) {
    lwm2m_object *server_object = context->object_tree->objects.get(1);
    lwm2m_instance *server_instance = lwm2m_instance_new(security_object, 0);

    lwm2m_resource_real short_server_id = server_instance->resources[0]->resource;
    lwm2m_resource_real lifetime = server_instance->resources[1]->resource;
    lwm2m_resource_real default_pmin = server_instance->resources[2]->resource;
    lwm2m_resource_real default_pmax = server_instance->resources[2]->resource;
    lwm2m_resource_real notification_storing = server_instance->resources[6]->resource;
    lwm2m_resource_real binding = server_instance->resources[7]->resource;
    // TODO implement registration update trigger resource inside library (set execute callback)

    short_server_id.single.value.int_value = 1;
    lifetime.single.value.int_value = 60;
    default_pmin.single.value.int_value = 10;
    default_pmax.single.value.int_value = 30;
    notification_storing.single.value.bool_value = false;
    binding.single.value.string_value = "T"; // TCP - this is not specified in LWM2M 1.0
}

//////////////// DEFINITION OF SUPPORTED OBJECTS ////////////////

lwm2m_object **create_example_objects() {
    lwm2m_object **objects = (lwm2m_object **) malloc(sizeof(lwm2m_object *) * 1);

    lwm2m_object *example_object = lwm2m_object_new();
    example_object->id = 123;
    example_object->mandatory = false;
    example_object->multiple = true;
    example_object->object_urn = "urn:oma:lwm2m:oma:123";
    example_object->attributes = lwm2m_attributes_new();

    // Set object's minimum period attribute to 10 sec
    lwm2m_attribute pmin_attribute = example_object->attributes.attributes[0]; //todo get by function;
    pmin_attribute.numeric_value.int_value = 10;

    objects[0] = example_object;
    return objects;
}

// TODO demonstrate read and write resource callbacks
lwm2m_resource **create_example_resources(int object_id) {
    if (object_id == 123) {
        lwm2m_resource **resources = (lwm2m_resource **) malloc(sizeof(lwm2m_resource *) * 10);
        lwm2m_resource *resource;

        resource = lwm2m_resource_new(false);
        resource->id = 0;
        resource->name = "Battery level";
        resource->type = INTEGER;
        resource->mandatory = true;
        resource->operations = READ & WRITE;
        resource->read_callback = measure_battery_level;
        resources[0] = resource;

        resource = lwm2m_resource_new(false);
        resource->id = 1;
        resource->name = "Double example resource";
        resource->type = DOUBLE;
        resource->mandatory = true;
        resource->operations = READ & WRITE;
        resources[1] = resource;

        resource = lwm2m_resource_new(false);
        resource->id = 2;
        resource->name = "String example resource";
        resource->type = STRING;
        resource->mandatory = true;
        resource->operations = READ & WRITE;
        resources[2] = resource;

        resource = lwm2m_resource_new(false);
        resource->id = 3;
        resource->name = "Light on";
        resource->type = BOOLEAN;
        resource->mandatory = true;
        resource->operations = READ & WRITE;
        resource->write_callback = switch_light;
        resources[3] = resource;

        resource = lwm2m_resource_new(false);
        resource->id = 4;
        resource->name = "Opaque example resource";
        resource->type = OPAQUE;
        resource->mandatory = true;
        resource->operations = READ & WRITE;
        resources[4] = resource;

        resource = lwm2m_resource_new(false);
        resource->id = 5;
        resource->name = "Firmwire update resource";
        resource->type = OPAQUE;
        resource->mandatory = true;
        resource->operations = EXECUTE;
        resource->execute_callback = update_firmwire;
        resources[5] = resource;

        resource = lwm2m_resource_new(false);
        resource->id = 6;
        resource->name = "Link example resource";
        resource->type = LINK;
        resource->mandatory = true;
        resource->operations = READ & WRITE;
        resources[6] = resource;

        resource = lwm2m_resource_new(false);
        resource->id = 7;
        resource->name = "Not mandatory integer example resource";
        resource->type = INTEGER;
        resource->mandatory = false;
        resource->operations = READ & WRITE;
        resources[7] = resource;

        resource = lwm2m_resource_new(true);
        resource->id = 8;
        resource->name = "Multiple string example resource";
        resource->type = STRING;
        resource->mandatory = true;
        resource->operations = READ & WRITE;
        resources[8] = resource;

        resource = lwm2m_resource_new(true);
        resource->id = 9;
        resource->name = "Readonly string example resource";
        resource->type = STRING;
        resource->mandatory = true;
        resource->operations = READ;
        resources[9] = resource;

        return resources;
    }
    return NULL;
}