#include <lwm2m_client.h>
#include <unistd.h>
/*
 * This example shows creating LWM2M client, which supports example object.
 * What is happening:
 * 1. create_objects_callback and create_resources_callback are assigned.
 *    This is a way of defining a custom LWM2M object
 *
 * 2. lwm2m_start_client is called.
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

#define SHORT_SERVER_ID 123

lwm2m_map *create_example_objects();
lwm2m_map *create_example_resources(int object_id);
int perform_factory_bootstrap(lwm2m_context *context);

int main(int argc, char *argv[]) {
    lwm2m_context *context = lwm2m_create_context();
    context->has_smartcard = false;
    context->create_objects_callback = create_example_objects;
    context->create_resources_callback = create_example_resources;
    context->factory_bootstrap_callback = perform_factory_bootstrap;
    context->smartcard_bootstrap_callback = NULL;
    context->client_id = "lynx_ep";
    context->broker_address = "tcp://localhost:1883";
    context->endpoint_client_name = "lynx_ep";

    lwm2m_start_client(context);
    // This should not exit, as new threads are created

    getchar();

    // Deregister from all servers
    int keys[context->servers->size];
    lwm2m_map_get_keys(context->servers, keys);
    for (int i = 0; i < context->servers->size; ++i) {
        lwm2m_server *server = lwm2m_map_get(context->servers, keys[i]);
        deregister(server);
    }
    sleep(2);
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
    char* version = "1.2"; // In reality you would parse it from args
    // In this place you would start procedure of updating firmwire
}


///////////////// DEFINITION OF FACTORY BOOTSTRAP ///////////////

void bootstrap_custom_object(lwm2m_context *context) {
    lwm2m_link self_link;
    self_link.object_id = 123;
    self_link.instance_id = 0;

    lwm2m_map *multiple_string = lwm2m_map_new();
    lwm2m_map_put(multiple_string, 0, "string1"); // TODO should this be lwm2m_value???
    lwm2m_map_put(multiple_string, 1, "string2");

    lwm2m_object *example_object = lwm2m_map_get(context->object_tree, 123);
    lwm2m_instance *example_instance = lwm2m_instance_new_with_id(example_object, 0);
    lwm2m_map_get_resource(example_instance->resources, 0)->resource.single.value.int_value = 80;
    lwm2m_map_get_resource(example_instance->resources, 1)->resource.single.value.double_value = 0.5;
    lwm2m_map_get_resource(example_instance->resources, 2)->resource.single.value.string_value = "example";
    lwm2m_map_get_resource(example_instance->resources, 3)->resource.single.value.bool_value = false;
    lwm2m_map_get_resource(example_instance->resources, 4)->resource.single.value.opaque_value = "opaque";

    lwm2m_map_get_resource(example_instance->resources, 6)->resource.single.value.link_value = self_link;
//    TODO allow NULL for valueslwm2m_map_get_resource(example_instance->resources, 7)->resource.single.
    lwm2m_map_get_resource(example_instance->resources, 8)->resource.multiple.instances = multiple_string;
    lwm2m_map_get_resource(example_instance->resources, 8)->resource.single.value.string_value = "readonlyString";
}

void bootstrap_security_object(lwm2m_context *context) {
    lwm2m_object *security_object = lwm2m_map_get(context->object_tree, 0);
    lwm2m_instance *security_instance = lwm2m_instance_new_with_id(security_object, 0);
    lwm2m_map_get_resource(security_instance->resources, 0)->resource.single.value.string_value = "lynx-bootstrap-server-old-name";
    lwm2m_map_get_resource(security_instance->resources, 1)->resource.single.value.bool_value = true;
    lwm2m_map_get_resource(security_instance->resources, 10)->resource.single.value.int_value = 555;
}

void bootstrap_server_object(lwm2m_context *context) {
    lwm2m_object *server_object = lwm2m_map_get(context->object_tree, 1);
    lwm2m_instance *server_instance = lwm2m_instance_new_with_id(server_object, 0);
    lwm2m_map_get_resource(server_instance->resources, 0)->resource.single.value.int_value = SHORT_SERVER_ID;
    lwm2m_map_get_resource(server_instance->resources, 1)->resource.single.value.int_value = 20;
    lwm2m_map_get_resource(server_instance->resources, 2)->resource.single.value.int_value = 10;
    lwm2m_map_get_resource(server_instance->resources, 3)->resource.single.value.int_value = 30;
    lwm2m_map_get_resource(server_instance->resources, 6)->resource.single.value.bool_value = false;
    lwm2m_map_get_resource(server_instance->resources, 7)->resource.single.value.string_value = "T"; // TCP - this is not specified in LWM2M 1.0
    // TODO implement registration update trigger resource inside library (set execute callback)
}

int perform_factory_bootstrap(lwm2m_context *context) {
    bootstrap_security_object(context);
//    bootstrap_server_object(context);
//    bootstrap_custom_object(context);
    return 0; // success
}

//////////////// DEFINITION OF SUPPORTED OBJECTS ////////////////

lwm2m_map *create_example_objects() {
    lwm2m_map *objects = lwm2m_map_new();

    lwm2m_object *example_object = lwm2m_object_new();
    example_object->id = 123;
    example_object->mandatory = false;
    example_object->multiple = true;
    example_object->object_urn = "urn:oma:lwm2m:oma:123";
    example_object->attributes = lwm2m_map_new();

    // Set object's minimum period attribute to 10 sec
    lwm2m_attribute *pmin_attribute = new_int_attribute("pmin", 10, ATTR_READ | ATTR_WRITE);
    lwm2m_map_put_string(example_object->attributes, "pmin", pmin_attribute);

    lwm2m_map_put(objects, example_object->id, (void*) example_object);
    return objects;
}

// TODO demonstrate read and write resource callbacks
lwm2m_map *create_example_resources(int object_id) {
    if (object_id == 123) {
        lwm2m_map *resources = lwm2m_map_new();
        lwm2m_resource *resource;

        resource = lwm2m_resource_new(false);
        resource->id = 0;
        resource->name = "Battery level";
        resource->type = INTEGER;
        resource->mandatory = true;
        resource->operations = READ | WRITE;
        resource->read_callback = measure_battery_level;
        lwm2m_map_put(resources, 0, resource);

        resource = lwm2m_resource_new(false);
        resource->id = 1;
        resource->name = "Double example resource";
        resource->type = DOUBLE;
        resource->mandatory = true;
        resource->operations = READ | WRITE;
        lwm2m_map_put(resources, 1, resource);

        resource = lwm2m_resource_new(false);
        resource->id = 2;
        resource->name = "String example resource";
        resource->type = STRING;
        resource->mandatory = true;
        resource->operations = READ | WRITE;
        lwm2m_map_put(resources, 2, resource);

        resource = lwm2m_resource_new(false);
        resource->id = 3;
        resource->name = "Light on";
        resource->type = BOOLEAN;
        resource->mandatory = true;
        resource->operations = READ | WRITE;
        resource->write_callback = switch_light;
        lwm2m_map_put(resources, 3, resource);

        resource = lwm2m_resource_new(false);
        resource->id = 4;
        resource->name = "Opaque example resource";
        resource->type = OPAQUE;
        resource->mandatory = true;
        resource->operations = READ | WRITE;
        lwm2m_map_put(resources, 4, resource);

        resource = lwm2m_resource_new(false);
        resource->id = 5;
        resource->name = "Firmwire update resource";
        resource->type = OPAQUE;
        resource->mandatory = true;
        resource->operations = EXECUTE;
        resource->execute_callback = update_firmwire;
        lwm2m_map_put(resources, 5, resource);

        resource = lwm2m_resource_new(false);
        resource->id = 6;
        resource->name = "Link example resource";
        resource->type = LINK;
        resource->mandatory = true;
        resource->operations = READ | WRITE;
        lwm2m_map_put(resources, 6, resource);

        resource = lwm2m_resource_new(false);
        resource->id = 7;
        resource->name = "Not mandatory integer example resource";
        resource->type = INTEGER;
        resource->mandatory = false;
        resource->operations = READ | WRITE;
        lwm2m_map_put(resources, 7, resource);

        resource = lwm2m_resource_new(true);
        resource->id = 8;
        resource->name = "Multiple string example resource";
        resource->type = STRING;
        resource->mandatory = true;
        resource->operations = READ | WRITE;
        lwm2m_map_put(resources, 8, resource);

        resource = lwm2m_resource_new(true);
        resource->id = 9;
        resource->name = "Readonly string example resource";
        resource->type = STRING;
        resource->mandatory = true;
        resource->operations = READ;
        lwm2m_map_put(resources, 9, resource);

        return resources;
    }
    return NULL;
}