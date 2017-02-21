#include <paho/MQTTAsync.h>
#include <paho/MQTTClient.h>
#include <lwm2m_device_management.h>
#include <unistd.h>
#include "lwm2m.h"
#include "lwm2m_transport_mqtt.h"
#include "lwm2m_transport.h"


pthread_mutex_t started_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t started_condition = PTHREAD_COND_INITIALIZER;

static void __print_time() {
    time_t timer;
    char buffer[26];
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  //  printf("[%s] ", buffer);
}

////////////////////////// INTERNAL //////////////////////////////

static lwm2m_topic parse_topic(char *topic_string) {
    char *buf = strtok(topic_string, "/");

    buf = strtok(NULL, "/");
    char *operation = (char *) malloc(strlen(buf) + 1);
    strcpy(operation, buf);

    buf = strtok(NULL, "/");
    char *type = (char *) malloc(strlen(buf) + 1);
    strcpy(type, buf);

    buf = strtok(NULL, "/");
    char *token = (char *) malloc(strlen(buf) + 1);
    strcpy(token, buf);

    buf = strtok(NULL, "/");
    char *client_id = (char *) malloc(strlen(buf) + 1);
    strcpy(client_id, buf);

    buf = strtok(NULL, "/");
    char *server_id = (char *) malloc(strlen(buf) + 1);
    strcpy(server_id, buf);

    int object_id = -1;
    int instance_id = -1;
    int resource_id = -1;

    buf = strtok(NULL, "/");
    if (buf != NULL) {
        object_id = atoi(buf);

        buf = strtok(NULL, "/");
        if (buf != NULL) {
            instance_id = atoi(buf);

            buf = strtok(NULL, "/");
            if (buf != NULL) {
                resource_id = atoi(buf);
            }
        }
    }

    lwm2m_topic topic = {
            .operation   = operation,
            .type        = type,
            .client_id   = client_id,
            .server_id   = server_id,
            .token       = token,
            .object_id   = object_id,
            .instance_id = instance_id,
            .resource_id = resource_id
    };
    return topic;
}

static lwm2m_request parse_request(char *payload, int payload_len) {
    lwm2m_request request = {
            .content_type = payload[0] & 0b00000111,
            .payload = payload + 1,
            .payload_len = (size_t) (payload_len - 1)
    }; // TODO payload NULL if paylod_len is 0
    return request;
}

static lwm2m_response parse_response(char *payload, int payload_len) {
    int response_code = 0;
    if (payload[1] & 0b10000000) {
        response_code += 400;
    } else {
        response_code += 200;
    }
    response_code += payload[1] & 0b01111111;

    lwm2m_response response = {
            .content_type = payload[0] & 0b00000111,
            .response_code = response_code,
            .success = payload[1] & 0b10000000 ? 0 : 1,
            .payload = payload + 1,
            .payload_len = payload_len - 1,
    };
    return response;
}

static void subscribe_init(lwm2m_context *context) {
    char topic_server[100];
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = NULL;
    opts.onFailure = NULL;
    opts.context = context;

    // Boostrap request
    sprintf(topic_server, "lynx/br/res/+/%s/+", context->client_id);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Boostrap write
    sprintf(topic_server, "lynx/bw/req/+/%s/+/#", context->client_id);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Bootstrap delete
    sprintf(topic_server, "lynx/bd/req/+/%s/+/#", context->client_id);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Bootstrap delete all
    sprintf(topic_server, "lynx/bd/req/+/%s/+", context->client_id);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Bootstrap finish
    sprintf(topic_server, "lynx/bf/req/+/%s/+", context->client_id);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Register
    sprintf(topic_server, "lynx/rr/res/+/%s/+", context->client_id);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);
}

static void on_publish_success(void *context, MQTTAsync_successData *response) {
//    printf("Success\n"); // TODO
}

static void on_publish_failure(void* context,  MQTTAsync_failureData* response) {
    printf("Publish failed\n");
}

static void on_connection_lost(void* context, char* cause) {
    printf("Connection lost. Reason: %s", cause);
}

static void on_delivery_complete(void* context, MQTTAsync_token token) {

}

void publish_connected(lwm2m_context *context) {
    time_t timer;
    char buffer[26];
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    MQTTAsync_responseOptions opts = {
            .onSuccess = on_publish_success,
            .onFailure = on_publish_failure,
            .context = context
    };
    char topic[100];
    sprintf(topic, "lynx/clients/%s", context->client_id);

    char message[2];
    sprintf(message, "1");

    MQTTAsync_send(*(context->mqtt_client), topic, 1, message, 1, 0, &opts);
    //printf("[%s] Received  %s\n", buffer, topic);
}

static void on_connect(void *context, MQTTAsync_successData *response) {
    pthread_mutex_lock(&started_lock);

    subscribe_init(context);
//    publish_connected((lwm2m_context*) context); todo move back?

    pthread_cond_signal(&started_condition);
    pthread_mutex_unlock(&started_lock);
}

static int on_message(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    time_t timer;
    char buffer[26];
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    //printf("[%s] Received  %s\n", buffer, topicName);
    receive_message((lwm2m_context *) context, topicName, (char *) message->payload, message->payloadlen);
    return 1; // TODO check if payload is correct (void -> char)
}


static void __free_response(lwm2m_response* response) {
    if (response->payload != NULL && response->payload_len > 0) {
        free(response->payload);
    }
}
////////////////////////// MAIN //////////////////////////////

void publish_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response) {
    MQTTAsync_responseOptions opts = {
            .onSuccess = on_publish_success,
            .onFailure = on_publish_failure,
            .context = context
    };
    int message_len;
    char message[5000];
    char topic_str[200];
    serialize_topic(topic, topic_str);
    serialize_response(response, message, &message_len);

    MQTTAsync_send(*(context->mqtt_client), topic_str, message_len, message, 0, 0, &opts);
    __print_time();
   // printf("Published %s\n", topic_str);


}

static lwm2m_server *__resolve_server(lwm2m_context *context, lwm2m_topic topic) {
    for (list_elem *elem = context->servers->first; elem != NULL; elem = elem->next) {
        lwm2m_server *server = elem->value;
        if (!strcmp(server->name, topic.server_id)) {
            return server;
        }
    }
    return NULL;
}

void receive_request(lwm2m_context *context, lwm2m_topic topic, char *message, int message_len) {
    lwm2m_request request = parse_request(message, message_len);
    lwm2m_response response;
    bool is_bootstrap = false;

    if (!strcmp(LWM2M_OPERATION_BOOTSTRAP_DELETE, topic.operation)) {
        response = handle_bootstrap_delete_request(context, topic, request);
        is_bootstrap = true;
//        DUMP_ALL(context);
    }
    else if (!strcmp(LWM2M_OPERATION_BOOTSTRAP_WRITE, topic.operation)) {
        response = handle_bootstrap_write_request(context, topic, request);
        is_bootstrap = true;
//        DUMP_ALL(context);
    }
    else if (!strcmp(LWM2M_OPERATION_BOOTSTRAP_FINISH, topic.operation)) {
        response = handle_bootstrap_finish_request(context, topic, request);
        is_bootstrap = true;
//        DUMP_ALL(context);
    } else {
        lwm2m_server *server = __resolve_server(context, topic);
        if (server == NULL) {
            char topic_str[100];
            serialize_topic(topic, topic_str);
            printf("Received request %s for non-existing server %s. Ignoring\n", topic_str, topic.server_id);
            return;
        }

        pthread_mutex_lock(&server->server_mutex);
        server->pending_requests++;

        if (server->register_state != 1) {
            char topic_str[100];
            serialize_topic(topic, topic_str);
            printf("Received request %s for deregistering server %s. Ignoring\n", topic_str, topic.server_id);

            if (--server->pending_requests == 0) {
                pthread_cond_signal(&server->no_pending_request_condition);
            }
            pthread_mutex_unlock(&server->server_mutex);
            return;
        }

        if (!strcmp(LWM2M_OPERATION_WRITE, topic.operation)) {
            response = handle_write_request(context, topic, request);
//        DUMP_ALL(context);
        }
        if (!strcmp(LWM2M_OPERATION_READ, topic.operation)) {
            response = handle_read_request(context, topic, request);
//        DUMP_ALL(context);
        }
        if (!strcmp(LWM2M_OPERATION_CREATE, topic.operation)) {
            response = handle_create_request(context, &topic, request);
//        DUMP_ALL(context);
        }
        if (!strcmp(LWM2M_OPERATION_DELETE, topic.operation)) {
            response = handle_delete_request(context, topic, request);
//        DUMP_ALL(context);
        }
        if (!strcmp(LWM2M_OPERATION_DISCOVER, topic.operation)) {
            response = handle_discover_request(context, topic, request);
//        DUMP_ALL(context);
        }
        if (!strcmp(LWM2M_OPERATION_OBSERVE, topic.operation)) {
            response = handle_observe_request(context, topic, request);
        }
        if (!strcmp(LWM2M_OPERATION_CANCEL_OBSERVE, topic.operation)) {
            response = handle_cancel_observe_request(context, topic, request);
        }
        if (!strcmp(LWM2M_OPERATION_WRITE_ATTRIBUTES, topic.operation)) {
            response = handle_write_attributes_request(context, topic, request);
        }
        if (!strcmp(LWM2M_OPERATION_EXECUTE, topic.operation)) {
            response = handle_execute_request(context, topic, request);
        }

        /** Publish response **/
        topic.type = "res";
        publish_response(context, topic, response);
        __free_response(&response);

        /** Server can deregister only if all requests ended **/
        if (--server->pending_requests == 0) {
            pthread_cond_signal(&server->no_pending_request_condition);
        }
        pthread_mutex_unlock(&server->server_mutex);
    }

    if (is_bootstrap) {
        topic.type = "res";
        publish_response(context, topic, response);
        __free_response(&response);
    }
}

void receive_response(lwm2m_context *context, lwm2m_topic topic, char *message, int message_len) {
    lwm2m_response response = parse_response(message, message_len);

    if (!strcmp(LWM2M_OPERATION_REGISTER, topic.operation)) {
        handle_register_response(context, topic, response);
    }
    if (!strcmp(LWM2M_OPERATION_UPDATE, topic.operation)) {
        handle_update_response(context, topic, response);
    }
    if (!strcmp(LWM2M_OPERATION_DEREGISTER, topic.operation)) {
        handle_deregister_response(context, topic, response);
    }
//    __free_response() TODO should I? or mqtt does free?
}

static void __free_topic(lwm2m_topic *topic) {
    free(topic->operation);
    free(topic->type);
    free(topic->client_id);
    free(topic->server_id);
    free(topic->token);
}

void receive_message(lwm2m_context *context, char* topic_string, char* message, int message_len) {
    lwm2m_topic topic = parse_topic(topic_string);

    if (!strcmp("req", topic.type)) {
        receive_request(context, topic, message, message_len);
    }
    if (!strcmp("res", topic.type)) {
        receive_response(context, topic, message, message_len);
    }
    __free_topic(&topic);
}

void subscribe_server(lwm2m_context *context, lwm2m_server *server) {
    char topic_server[100];
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = NULL;
    opts.onFailure = NULL;
    opts.context = context;

    // Update
    sprintf(topic_server, "lynx/ru/res/+/%s/%s", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Deregister
    sprintf(topic_server, "lynx/rd/res/+/%s/%s", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Read
    sprintf(topic_server, "lynx/mr/req/+/%s/%s/#", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Write
    sprintf(topic_server, "lynx/mw/req/+/%s/%s/#", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Execute
    sprintf(topic_server, "lynx/me/req/+/%s/%s/+/+/+", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Create
    sprintf(topic_server, "lynx/mc/req/+/%s/%s/#", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Delete
    sprintf(topic_server, "lynx/md/req/+/%s/%s/+/+", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Write attributes
    sprintf(topic_server, "lynx/ma/req/+/%s/%s/#", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Discover
    sprintf(topic_server, "lynx/mm/req/+/%s/%s/#", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Observe
    sprintf(topic_server, "lynx/io/req/+/%s/%s/#", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);

    // Cancel observe
    sprintf(topic_server, "lynx/ic/req/+/%s/%s/#", context->client_id, server->name);
    MQTTAsync_subscribe(*(context->mqtt_client), topic_server, 1, &opts);
}

void publish(lwm2m_context *context, char* topic, char* message, int message_len) {

    MQTTAsync_responseOptions opts = {
            .onSuccess = on_publish_success,
            .onFailure = on_publish_failure,
            .context = context
    };
    MQTTAsync_send(*(context->mqtt_client), topic, message_len, message, 0, 0, &opts);
    __print_time();
   // printf("Published %s\n", topic);
}


static void on_connect_fail(void* context,  MQTTAsync_failureData* response) {
    printf("failed to conn\n");
//    printf(response->message);
    exit(1);
}


int start_mqtt(lwm2m_context *context) {
    pthread_mutex_lock(&started_lock);

    ////////////////// TLS ////////////////////
    MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
    if (context->tls) {
        ssl_opts.trustStore = "/home/mateusz/ca.crt";
        char *url = malloc(strlen(context->broker_address) + 7);
        url[0] = 0;
        strcat(url, "ssl://");
        strcat(url, context->broker_address);
        context->broker_address = url;
    } else {
        char *url = malloc(strlen(context->broker_address) + 7);
        url[0] = 0;
        strcat(url, "tcp://");
        strcat(url, context->broker_address);
        context->broker_address = url;
    }

    context->mqtt_client = (MQTTAsync *) malloc(sizeof(MQTTAsync));
    int res;

    /* Connect */
    if ((res = MQTTAsync_create(context->mqtt_client, context->broker_address, context->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL))) {
        return res;
    }
    if ((res = MQTTAsync_setCallbacks(*(context->mqtt_client), context, on_connection_lost, on_message, on_delivery_complete))) {
        return res;
    }

    char *topic = (char*) malloc(100);
    sprintf(topic, "lynx/clients/%s", context->endpoint_client_name);

    MQTTAsync_willOptions will_opts = MQTTAsync_willOptions_initializer;
    will_opts.topicName = topic;
    will_opts.message = "0";
    will_opts.retained = 0;
    will_opts.qos = 0;

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = 10;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = on_connect;
    conn_opts.onFailure = on_connect_fail;
    conn_opts.context = context;
    conn_opts.will = &will_opts;
    if (context->tls) {
        conn_opts.ssl = &ssl_opts;
    }

    res = MQTTAsync_connect(*(context->mqtt_client), &conn_opts);
    pthread_cond_wait(&started_condition, &started_lock);

    pthread_mutex_unlock(&started_lock);
    return 0;
}

int stop_mqtt(lwm2m_context *context) {
    MQTTAsync_disconnectOptions disconn_opts = MQTTAsync_disconnectOptions_initializer;
    MQTTAsync_disconnect(context->mqtt_client, &disconn_opts);
    sleep(1);

    MQTTAsync_destroy(context->mqtt_client);
    sleep(1);
    return 0;
}

