#include <lwm2m_client.h>
#include <unistd.h>
#include <lwm2m_transport_mqtt.h>
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
 * |5   | Firmwire update resource                  | E          | Single    | Optional  |         |
 * |6   | Link example resource                     | RW         | Single    | Mandatory | ObjInk  |
 * |7   | Not mandatory integer example resource    | RW         | Single    | Optional  | Integer |
 * |8   | Multiple string example resource          | RW         | Multiple  | Mandatory | String  |
 * |9   | Readonly string example resource          | R          | Single    | Optional  | String  |
 * +----+-------------------------------------------+------------+-----------+-----------+---------+
 *
 */

#define SECURITY_MODE_NOSEC 3

#define SHORT_SERVER_ID 123

#include "lwm2m_attribute.h"

list *create_example_objects();
lwm2m_resource *create_example_resources(int object_id);
int perform_factory_bootstrap(lwm2m_context *context);

int main(int argc, char *argv[]) {
    lwm2m_context *context = lwm2m_create_context();
    context->has_smartcard = false;
    context->create_objects_callback = create_example_objects;
    context->factory_bootstrap_callback = perform_factory_bootstrap;
    context->smartcard_bootstrap_callback = NULL;
    context->client_id = "lynx_ep";
    context->broker_address = "tcp://localhost:1883";
    context->endpoint_client_name = "lynx_ep";

    lwm2m_start_client(context);
    // This should not exit, as new threads are created


//    lwm2m_object *example_obj = lwm2m_map_get_object(context->object_tree, 123);
//    lwm2m_instance *example_instance = lwm2m_map_get_instance(example_obj->instances, 20);
//    lwm2m_resource *int_resource = lwm2m_map_get(example_instance->resources, 0);
//    lwm2m_resource *string_resource = lwm2m_map_get(example_instance->resources, 2);

//    printf("Setting value. Should notify all\n");
//    set_value_int(int_resource, 11);
//    sleep(6);
//
//    printf("Setting value. Should notify object\n");
//    set_value_int(int_resource, 12);
//    sleep(6);
//
//    printf("Setting value. Should notify object and instance\n");
//    set_value_int(int_resource, 13);
//    sleep(16);
//
//
//    printf("Setting value. Should notify object and instance and resource\n");
//    set_value_int(int_resource, 14);
//    sleep(30);
//
//    printf("Setting value. Should not notify (lt)\n");
//    set_value_int(int_resource, 17);
//    getchar();

//    printf("Set value float\n");
//    set_value_double(float_resource, 512.2);
//    getchar();


//    printf("Set value string\n");
//    set_value_string(string_resource, "new value");
//    getchar();

    getchar();
    printf("Stopping...\n");
    // Deregister from all servers
    for (list_elem *elem = context->servers->first; elem != NULL; elem = elem->next) {
        lwm2m_server *server = elem->value;
        deregister(server);
    }
    sleep(2);
    stop_mqtt(context);
    stop_scheduler(context->scheduler);
    return 0;
}


///////////////// DEFINITION OF RESOURCE CALLBACKS //////////////

void measure_battery_level(lwm2m_resource* resource) {
    int battery_level = 20; // On real device this would be read of some sensor
    __set_value_int(resource, battery_level);
}

void switch_light(lwm2m_resource *resource) {
    bool light_turned_on = resource->value->bool_value;
    if (light_turned_on) {
        // On real device you would call turning light on here
    } else {
        // On real device you would call turning light off here
    }
}

void update_firmwire(lwm2m_resource *resource, list *args) {
    char *url = ((execute_param*) lfind(args, 0))->string_value;
    int times = ((execute_param *) lfind(args, 1))->int_value;
    printf("[res %s] I would execute ping on %s %d times, but I cant for now :(", resource->name, url, times);
}


///////////////// DEFINITION OF FACTORY BOOTSTRAP ///////////////

//void bootstrap_custom_object(lwm2m_context *context) {
//    lwm2m_link self_link;
//    self_link.object_id = 123;
//    self_link.instance_id = 0;
//
//    lwm2m_map *multiple_string = lwm2m_map_new();
//    lwm2m_map_put(multiple_string, 0, "string1"); // TODO should this be lwm2m_value???
//    lwm2m_map_put(multiple_string, 1, "string2");
//
//    lwm2m_object *example_object = lwm2m_map_get(context->object_tree, 123);
//    lwm2m_instance *example_instance = lwm2m_instance_new_with_id(example_object, 0);
//    lwm2m_map_get_resource(example_instance->resources, 0)->resource.single.value.int_value = 80;
//    lwm2m_map_get_resource(example_instance->resources, 1)->resource.single.value.double_value = 0.5;
//    lwm2m_map_get_resource(example_instance->resources, 2)->resource.single.value.string_value = "example";
//    lwm2m_map_get_resource(example_instance->resources, 3)->resource.single.value.bool_value = false;
//    lwm2m_map_get_resource(example_instance->resources, 4)->resource.single.value.opaque_value = "opaque";
//
//    lwm2m_map_get_resource(example_instance->resources, 6)->resource.single.value.link_value = self_link;
////    TODO allow NULL for valueslwm2m_map_get_resource(example_instance->resources, 7)->resource.single.
//    // TODO WHATS WRONG HERE
//    lwm2m_map_get_resource(example_instance->resources, 8)->resource.multiple.instances = multiple_string;
//    lwm2m_map_get_resource(example_instance->resources, 8)->resource.single.value.string_value = "readonlyString";
//}

//void bootstrap_security_object(lwm2m_context *context) {
//    lwm2m_object *security_object = lwm2m_map_get(context->object_tree, 0);
//    lwm2m_instance *security_instance = lwm2m_instance_new_with_id(security_object, 0);
//    lwm2m_map_get_resource(security_instance->resources, 0)->resource.single.value.string_value = "lynx-bootstrap-server-old-name";
//    lwm2m_map_get_resource(security_instance->resources, 1)->resource.single.value.bool_value = true;
//    lwm2m_map_get_resource(security_instance->resources, 10)->resource.single.value.int_value = 555;
//}
//
//void bootstrap_server_object(lwm2m_context *context) {
//    lwm2m_object *server_object = lwm2m_map_get(context->object_tree, 1);
//    lwm2m_instance *server_instance = lwm2m_instance_new_with_id(server_object, 0);
//    lwm2m_map_get_resource(server_instance->resources, 0)->resource.single.value.int_value = SHORT_SERVER_ID;
//    lwm2m_map_get_resource(server_instance->resources, 1)->resource.single.value.int_value = 20;
//    lwm2m_map_get_resource(server_instance->resources, 2)->resource.single.value.int_value = 10;
//    lwm2m_map_get_resource(server_instance->resources, 3)->resource.single.value.int_value = 30;
//    lwm2m_map_get_resource(server_instance->resources, 6)->resource.single.value.bool_value = false;
//    lwm2m_map_get_resource(server_instance->resources, 7)->resource.single.value.string_value = "T"; // TCP - this is not specified in LWM2M 1.0
//    // TODO implement registration update trigger resource inside library (set execute callback)
//}

int perform_factory_bootstrap(lwm2m_context *context) {
//    bootstrap_security_object(context);
//    bootstrap_server_object(context);
//    bootstrap_custom_object(context);
    return 0; // success
}

//////////////// DEFINITION OF SUPPORTED OBJECTS ////////////////

list *create_example_objects() {
    list *objects = list_new();

    lwm2m_object *example_object = lwm2m_object_new();
    example_object->id = 123;
    example_object->mandatory = false;
    example_object->multiple = true;
    example_object->object_urn = "urn:oma:lwm2m:oma:123";
    example_object->attributes = list_new();
    example_object->resource_def = create_example_resources(123);
    example_object->resource_def_len = 10;

    // Set object's minimum period attribute to 10 sec
    // TODO new way of declaring attributes
//    lwm2m_attribute *pmin_attribute = new_int_attribute("pmin", 10, ATTR_READ | ATTR_WRITE);
//    lwm2m_map_put_string(example_object->attributes, "pmin", pmin_attribute);

    ladd(objects, example_object->id, (void*) example_object);
    return objects;
}

// TODO demonstrate read and write resource callbacks
lwm2m_resource *create_example_resources(int object_id) {
    if (object_id == 123) {
        lwm2m_resource *resources = (lwm2m_resource*) malloc(10 * sizeof(lwm2m_resource));

        resources[0].multiple = false;
        resources[0].id = 0;
        resources[0].name = "Battery level";
        resources[0].type = INTEGER;
        resources[0].mandatory = true;
        resources[0].operations = READ | WRITE;
        resources[0].read_callback = measure_battery_level;

        resources[1].multiple = false;
        resources[1].id = 1;
        resources[1].name = "Double example resource";
        resources[1].type = DOUBLE;
        resources[1].mandatory = true;
        resources[1].operations = READ | WRITE;

        resources[2].multiple = false;
        resources[2].id = 2;
        resources[2].name = "String example resource";
        resources[2].type = STRING;
        resources[2].mandatory = true;
        resources[2].operations = READ | WRITE;

        resources[3].multiple = false;
        resources[3].id = 3;
        resources[3].name = "Light on";
        resources[3].type = BOOLEAN;
        resources[3].mandatory = true;
        resources[3].operations = READ | WRITE;
        resources[3].write_callback = switch_light;

        resources[4].multiple = false;
        resources[4].id = 4;
        resources[4].name = "Opaque example resource";
        resources[4].type = OPAQUE;
        resources[4].mandatory = true;
        resources[4].operations = READ | WRITE;

        resources[5].multiple = false;
        resources[5].id = 5;
        resources[5].name = "Firmwire update resource";
        resources[5].type = NONE;
        resources[5].mandatory = false;
        resources[5].operations = EXECUTE;
        resources[5].execute_callback = update_firmwire;

        resources[6].multiple = false;
        resources[6].id = 6;
        resources[6].name = "Link example resource";
        resources[6].type = LINK;
        resources[6].mandatory = true;
        resources[6].operations = READ | WRITE;

        resources[7].multiple = false;
        resources[7].id = 7;
        resources[7].name = "Not mandatory integer example resource";
        resources[7].type = INTEGER;
        resources[7].mandatory = false;
        resources[7].operations = READ | WRITE;

        resources[8].id = 8;
        resources[8].name = "Multiple string example resource";
        resources[8].type = STRING;
        resources[8].mandatory = true;
        resources[8].operations = READ | WRITE;

        resources[9].id = 9;
        resources[9].name = "Readonly string example resource";
        resources[9].type = STRING;
        resources[9].mandatory = false;
        resources[9].operations = READ;

        return resources;
    }
    return NULL;
}