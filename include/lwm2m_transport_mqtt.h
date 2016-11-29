#include "lwm2m.h"
#include "lwm2m_transport.h"

#ifndef PROJECT_LWM2M_TRANSPORT_MQTT_H
#define PROJECT_LWM2M_TRANSPORT_MQTT_H

void receive_message(lwm2m_context *context, char* topic_string, char* message, int message_len);
void receive_request(lwm2m_context *context, lwm2m_topic topic, char* message, int message_len);
void receive_response(lwm2m_context *context, lwm2m_topic topic, char* message, int message_len);

void publish(lwm2m_context *context, char* topic, char* message, int message_len);
void subscribe_server(lwm2m_context *context, lwm2m_server *server);

int start_mqtt(lwm2m_context *context);
int stop_mqtt(lwm2m_context *context);
void publish_connected(lwm2m_context *context);

#endif //PROJECT_LWM2M_TRANSPORT_MQTT_H
