#include "lwm2m_transport_mqtt.h"
#include "lwm2m.h"
#include <paho/MQTTClient.h>
#include <paho/MQTTAsync.h>
#include <pthread.h>

int on_message(void* context, char* topicName, int topicLen, MQTTAsync_message* message) {
    return 0;
}


int start_bootstrap_networking(lwm2m_context* context) {
    MQTTAsync client;
    int res;

    if ((res = MQTTAsync_create(&client, context->broker_address, context->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL))) {
        return res;
    }
    if ((res = MQTTAsync_setCallbacks(client, context, NULL, on_message, NULL))) {
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
}

int wait_for_server_bootstrap(lwm2m_context *context, int time) {
    pthread_mutex_lock(context->bootstrap_mutex);
    while (!context->state == BOOTSTRAPPED) {
        pthread_cond_timedwait(context->bootstrap_finished_condition, context->bootstrap_mutex, time);
    }
    pthread_mutex_unlock(context->bootstrap_mutex);
}

