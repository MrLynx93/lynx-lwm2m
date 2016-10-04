#include "lwm2m_parser.h"


/////////////// SERIALIZE //////////////

static char *serialize_text(lwm2m_value value, lwm2m_type type) {
    // TODO switch inside
    return "";
}

static char *serialize_tlv(lwm2m_value value, lwm2m_type type) {
    // TODO switch inside
    return "";
}

/////////////// DESERIALIZE //////////////

static lwm2m_value deserialize_text(char* message, lwm2m_type type) {
    lwm2m_value value;
    value.bool_value = 1;
    // TODO switch inside
    return value;
}

static lwm2m_value deserialize_tlv(char* message, lwm2m_type type) {
    lwm2m_value value;
    value.bool_value = 1;
    // TODO switch inside
    return value;
}

////////// TEXT VALUES /////////

char *serialize_lwm2m_value(lwm2m_value value, lwm2m_type type, int format) {
    return format == TLV_FORMAT ? serialize_tlv(value, type) : serialize_text(value, type);
}

lwm2m_value deserialize_lwm2m_value(char* message, lwm2m_type type, int format) {
    return format == TLV_FORMAT ? deserialize_tlv(message, type) : deserialize_text(message, type);
}