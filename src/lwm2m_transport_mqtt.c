#include "lwm2m_transport_mqtt.h"
#include <stdbool.h>
#include <paho/MQTTClient.h>
#include <paho/MQTTAsync.h>

static char *get_address(lwm2m_context *context);
static char *get_client_id(lwm2m_context *pContext);



void on_connection_lost(void* context, char* cause) {}

void on_delivery_complete(void* context, MQTTAsync_token token) {}

int on_message(void* context, char* topicName, int topicLen, MQTTAsync_message* message) { return 0;}



int start_transport_layer(lwm2m_context *context) {
    MQTTAsync client;
    int res;

//    Connect
    if ((res = MQTTAsync_create(&client, get_address(context), get_client_id(context), MQTTCLIENT_PERSISTENCE_NONE, NULL))) {
        return res;
    }
    if ((res = MQTTAsync_setCallbacks(client, context, on_connection_lost, on_message, on_delivery_complete))) {
        return res;
    }

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = 10;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = NULL;
    conn_opts.onFailure = NULL;
    conn_opts.context = client;
    res = MQTTAsync_connect(client, &conn_opts);


    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = NULL;
    opts.onFailure = NULL;
    opts.context = client;
    res = MQTTAsync_subscribe(client, "br/", 0, &opts);

    while(true) {
//        MQTTYield(&mqttClient, MQTT_YIELD_TIME);
        continue;
    }
    return 0;
}

static char *get_client_id(lwm2m_context *context) {
    return "lynx"; // TODO implement
}

//static int get_port(lwm2m_context *context) {
//    return 1883; // TODO implement
//}

static char *get_address(lwm2m_context *context) {
    return "broker.hivemq.com:1883"; // TODO implement
}
