#include "lwm2m_transport_mqtt.h"
#include <paho/MQTTClient.h>
#include <stdbool.h>

#define READ_BUFFER_SIZE 100
#define SEND_BUFFER_SIZE 100
#define COMMAND_TIMEOUT 500
#define MQTT_YIELD_TIME 100

static char *get_address(lwm2m_context *context);
static int get_port(lwm2m_context *context);
static char *get_client_id(lwm2m_context *pContext);


void on_bootstrap_message(MessageData* data)
{
    printf("Message arrived on topic %s: %s\n", data->topicName->lenstring.data, (char *) data->message->payload);
}

char* bootstrap_topic(lwm2m_context* context) {
    char* buf = (char*) malloc(sizeof(char) * 100);
    strcat(buf, get_client_id(context));
    strcat(buf, "/+/b");
    return buf;
}

int start_transport_layer(lwm2m_context *context) {
    unsigned char sendbuf[SEND_BUFFER_SIZE], readbuf[READ_BUFFER_SIZE];
    MQTTClient mqttClient;
    Network network;
    int res;

    // Set MQTT Connect data
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    connectData.clientID.cstring = get_client_id(context);
    connectData.MQTTVersion = 4; // 3.1.1

    // Init structures
    NetworkInit(&network);
    MQTTClientInit(&mqttClient, &network, COMMAND_TIMEOUT, sendbuf, SEND_BUFFER_SIZE, readbuf, READ_BUFFER_SIZE);

    // Connect
    if (!(res = NetworkConnect(&network, get_address(context), get_port(context)))) {
        return res;
    }
    if (!(res = MQTTConnect(&mqttClient, &connectData))) {
        return res;
    }

    // Subscribe to important topics
    MQTTSubscribe(&mqttClient, bootstrap_topic(context), QOS0, on_bootstrap_message);

    // Run
    while(true) {
        MQTTYield(&mqttClient, MQTT_YIELD_TIME);
    }
}

static char *get_client_id(lwm2m_context *context) {
    return "lynx"; // TODO implement
}

static int get_port(lwm2m_context *context) {
    return 1883; // TODO implement
}

static char *get_address(lwm2m_context *context) {
    return "broker.hivemq.com"; // TODO implement
}
