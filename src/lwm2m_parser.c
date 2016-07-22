#include "../include/lwm2m_parser.h"

char *serialize_lwm2m_value(lwm2m_value value, lwm2m_type type, int format) {
    return format == TLV_FORMAT ? serialize_tlv(value, type) : serialize_text(value, type);
}

/////////////// PRIVATE //////////////

static char *serialize_text(lwm2m_value value, lwm2m_type type) {
    // TODO switch inside
}

static char *serialize_tlv(lwm2m_value value, lwm2m_type type) {
    // TODO switch inside
}

