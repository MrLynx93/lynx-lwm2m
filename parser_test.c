#include "include/lwm2m_parser.h"
#include <assert.h>

char* message;
int length;


int main() {
    TLV_INT_TEST();
}

void TLV_INT_TEST() {
    lwm2m_resource int_resource;

    //// 8-bit
    int_resource.resource.single.value.int_value = 5;
    serialize_lwm2m_resource(int_resource, &message, &length);
    assert(message[0] == blabla);
    assert(length == 1);

    int_resource.resource.single.value.int_value = -5;
    serialize_lwm2m_resource(int_resource, &message, &length);
    assert(message[0] == blabla);
    assert(length == 1);

    //// 16-bit
    int_resource.resource.single.value.int_value = 130;
    serialize_lwm2m_resource(int_resource, &message, &length);
    assert(message[0] == blabla);
    assert(message[1] == blabla);
    assert(length == 2);

    int_resource.resource.single.value.int_value = -130;
    serialize_lwm2m_resource(int_resource, &message, &length);
    assert(message[0] == blabla);
    assert(message[1] == blabla);
    assert(length == 2);

    //// 32-bit
    int_resource.resource.single.value.int_value = 30000;
    serialize_lwm2m_resource(int_resource, &message, &length);
    assert(message[0] == blabla);
    assert(message[1] == blabla);
    assert(message[2] == blabla);
    assert(message[3] == blabla);
    assert(length == 4);

    int_resource.resource.single.value.int_value = -30000;
    serialize_lwm2m_resource(int_resource, &message, &length);
    assert(message[0] == blabla);
    assert(message[1] == blabla);
    assert(message[2] == blabla);
    assert(message[3] == blabla);
    assert(length == 4);

    //// 64-bit
    int_resource.resource.single.value.int_value = 5000000000;
    serialize_lwm2m_resource(int_resource, &message, &length);
    assert(message[0] == blabla);
    assert(message[1] == blabla);
    assert(message[2] == blabla);
    assert(message[3] == blabla);
    assert(message[4] == blabla);
    assert(message[5] == blabla);
    assert(message[6] == blabla);
    assert(message[7] == blabla);
    assert(length == 8);

    int_resource.resource.single.value.int_value = -5000000000;
    serialize_lwm2m_resource(int_resource, &message, &length);
    assert(message[0] == blabla);
    assert(message[1] == blabla);
    assert(message[2] == blabla);
    assert(message[3] == blabla);
    assert(message[4] == blabla);
    assert(message[5] == blabla);
    assert(message[6] == blabla);
    assert(message[7] == blabla);
    assert(length == 8);
}

void TLV_FLOAT_TEST() {
    lwm2m_resource float_resource;

    //// 32-bit
    float_resource.resource.single.value.double_value = 1.1;
    serialize_lwm2m_resource(float_resource, &message, &length);
    assert(message[0] == blabla);
    assert(message[1] == blabla);
    assert(message[2] == blabla);
    assert(message[3] == blabla);
    assert(length == 4);

    float_resource.resource.single.value.double_value = -1.1;
    serialize_lwm2m_resource(float_resource, &message, &length);
    assert(message[0] == blabla);
    assert(message[1] == blabla);
    assert(message[2] == blabla);
    assert(message[3] == blabla);
    assert(length == 4);

    //// 64-bit
    float_resource.resource.single.value.double_value = FLOAT_MAX + 5.5;
    serialize_lwm2m_resource(float_resource, &message, &length);
    assert(message[0] == blabla);
    assert(message[1] == blabla);
    assert(message[2] == blabla);
    assert(message[3] == blabla);
    assert(message[4] == blabla);
    assert(message[5] == blabla);
    assert(message[6] == blabla);
    assert(message[7] == blabla);
    assert(length == 8);

    float_resource.resource.single.value.double_value = -(FLOAT_MAX + 5.5);
    serialize_lwm2m_resource(float_resource, &message, &length);
    assert(message[0] == blabla);
    assert(message[1] == blabla);
    assert(message[2] == blabla);
    assert(message[3] == blabla);
    assert(message[4] == blabla);
    assert(message[5] == blabla);
    assert(message[6] == blabla);
    assert(message[7] == blabla);
    assert(length == 8);
}
