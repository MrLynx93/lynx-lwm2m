#include "include/lwm2m.h"

/*
 * Example object:
 * +-------------------------------------------+
 * | Name       |   Example                    |
 * | ID         |   123                        |
 * | Instances  |   multiple                   |
 * | Mandatory  |   false                      |
 * | Object     |   URN urn:oma:lwm2m:oma:123  |
 * +-------------------------------------------+
 *
 * Resources:
 * +----+-------------------------------------------+------------+-----------+-----------+---------+
 * |ID  | Name                                      | Operations | Instances | Mandatory | Type    |
 * +----+-------------------------------------------+------------+-----------+-----------+---------+
 * |0   | Integer example resource                  | RW         | Single    | Mandatory | Integer |
 * |1   | Double example resource                   | RW         | Single    | Mandatory | Double  |
 * |2   | String example resource                   | RW         | Single    | Mandatory | String  |
 * |3   | Boolean example resource                  | RW         | Single    | Mandatory | Boolean |
 * |4   | Opaque example resource                   | RW         | Single    | Mandatory | Opaque  |
 * |5   | Execute example resource                  | E          | Single    | Mandatory |         |
 * |6   | Link example resource                     | RW         | Single    | Mandatory | ObjInk  |
 * |7   | Not mandatory integer example resource    | RW         | Single    | Optional  | Integer |
 * |8   | Multiple string example resource          | RW         | Multiple  | Mandatory | String  |
 * |9   | Readonly string example resource          | R          | Single    | Mandatory | String  |
 * +----+-------------------------------------------+------------+-----------+-----------+---------+
 *
 */

lwm2m_object** create_example_objects();
lwm2m_resource** create_example_resources(int object_id);


int main(int argc, char* argv[]) {
    lwm2m_context *context = lwm2m_create_context();
    context->create_objects_callback = create_example_objects;
    context->create_resources_callback = create_example_resources;

    l
}




lwm2m_object** create_example_objects() {
    lwm2m_object** objects = (lwm2m_object**) malloc(sizeof(lwm2m_object*) * 1);

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

lwm2m_resource** create_example_resources(int object_id) {
    if (object_id == 123) {
        lwm2m_resource** resources = (lwm2m_resource**) malloc(sizeof(lwm2m_resource*) * 10);
        lwm2m_resource *resource;

        resource = lwm2m_resource_new(false);
        resource->id = 0;
        resource->name = "Integer example resource";
        resource->type = INTEGER;
        resource->mandatory = true;
        resource->operations = READ & WRITE;
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
        resource->name = "Boolean example resource";
        resource->type = BOOLEAN;
        resource->mandatory = true;
        resource->operations = READ & WRITE;
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
        resource->name = "Execute example resource";
        resource->type = OPAQUE;
        resource->mandatory = true;
        resource->operations = EXECUTE;
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