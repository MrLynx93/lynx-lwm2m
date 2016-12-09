#include <lwm2m_register.h>
#include <lwm2m_information_reporting.h>
#include "lwm2m_bootstrap.h"
#include "lwm2m_transport_mqtt.h"
#include "lwm2m_device_management.h"
#include "lwm2m_attribute.h"


#define CHARSET "abcdefghijklmnopqrstuvwxyz1234567890"
#define CHARSET_LENGTH 36
#define TOKEN_LENGTH 8

list *__parse_args(lwm2m_request request) {
    list *args = list_new();
    if (request.payload_len == 0) {
        return args;
    }

    int count = 0;
    char *elems[10];
    char *buf = strtok(request.payload, ",");

    while (buf != NULL) {
        elems[count++] = buf;
        buf = strtok(NULL, ",");
    }

    for (int i = 0; i < count; i++) {
        execute_param *param = param_new();
        buf = strtok(elems[i], "=");
        param->key = atoi(buf);
        ladd(args, param->key, param);

        buf = strtok(NULL, "=");
        /**** String parameter - begins and ends with ' ****/
        if (buf[0] == '\'' && buf[strlen(buf) - 1] == '\'') {
            param->string_value = (char *) malloc(sizeof(char) * strlen(buf));
            memcpy(param->string_value, buf + 1, strlen(buf) - 2);
        }
        /**** Float parameter - contains '.' ****/
        else if(strchr(buf, '.') != NULL) {
            param->float_value = (float) atof(buf);
        }
        /**** Integer parameter ****/
        else {
            param->int_value = atoi(buf);
        }
    }
    return args;
}

char *serialize_response(lwm2m_response response, char* message, int *message_len) {
    message[0] = (char) response.content_type; // TODO int to format in pdf
    message[1] = 0;
    if (response.response_code / 100 == 200)  {
        message[1] &= 0b10000000;
    }
    message[1] += response.response_code % 100;

    memcpy(message + 2, response.payload, (size_t) response.payload_len);
    *message_len = response.payload_len + 2;
    return message;
}

static lwm2m_server *__resolve_server(lwm2m_context *context, lwm2m_topic topic) {
    return (lwm2m_server *) lfind(context->servers, atoi(topic.server_id));
}

static lwm2m_object *__resolve_object(lwm2m_context *context, lwm2m_topic topic) {
    return lfind(context->object_tree, topic.object_id);
}

static lwm2m_instance *__resolve_instance(lwm2m_object *object, lwm2m_topic topic) {
    if (object == NULL || topic.instance_id == -1) {
        return NULL;
    }
    return lfind(object->instances, topic.instance_id);
}

static lwm2m_resource *__resolve_resource(lwm2m_instance *instance, lwm2m_topic topic) {
    if (instance == NULL || topic.resource_id == -1) {
        return NULL;
    }
    return lfind(instance->resources, topic.resource_id);
}

static char *serialize_request(lwm2m_request request, char *message, int *message_len) {
    message[0] = (char) request.content_type;
    memcpy(message + 1, request.payload, request.payload_len);
    *message_len = (int) strlen(message + 1) + 1;
    return message;
}

static char *serialize_register_request(lwm2m_register_request request, char *message, int *message_len) {
    sprintf(message + 1, "%s?%s", request.header, request.payload);
    message[0] = (char) request.content_type;
    *message_len = (int) (strlen(message + 1) + 1);
    return message;
}

char *serialize_topic(lwm2m_topic topic, char *message) {
    char *id;
    sprintf(message, "lynx/%s/%s/%s/%s/%s", topic.operation, topic.type, topic.token, topic.client_id, topic.server_id);
    if (topic.object_id != -1) {
        strcat(message, "/");
        id = itoa(topic.object_id);
        strcat(message, id);
        free(id);
    }
    if (topic.instance_id != -1) {
        strcat(message, "/");
        id = itoa(topic.instance_id);
        strcat(message, id);
        free(id);
    }
    if (topic.resource_id != -1) {
        strcat(message, "/");
        id = itoa(topic.resource_id);
        strcat(message, id);
        free(id);
    }
    return message;
}

char *generate_token() {
    srand(time(0));
    char* token = (char *) malloc(TOKEN_LENGTH + 1);
    for (int i = 0; i < TOKEN_LENGTH; ++i) {
        token[i] = CHARSET[rand() % CHARSET_LENGTH];
    }
    token[TOKEN_LENGTH] = 0;
    return token;
}

lwm2m_response handle_bootstrap_delete_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    if (topic.instance_id == -1) {
        on_bootstrap_delete_all(context);
    } else {
        lwm2m_object *object = lfind(context->object_tree, topic.object_id);
        lwm2m_instance *instance = lfind(object->instances, topic.instance_id);
        on_bootstrap_delete(context, instance);
    }

    lwm2m_response response = {
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .response_code = RESPONSE_CODE_DELETED,
            .payload = "",
            .payload_len = 0
    };
    return response;
}

lwm2m_response handle_bootstrap_write_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    lwm2m_object *object = lfind(context->object_tree, topic.object_id);
    if (topic.instance_id != -1 && topic.resource_id != -1) {
        lwm2m_instance *instance = lfind(object->instances, topic.instance_id);
        lwm2m_resource *resource = lfind(instance->resources, topic.resource_id);
        on_bootstrap_resource_write(resource, request.payload, (int) request.payload_len);
    }
    else if (topic.instance_id != -1) {
        on_bootstrap_instance_write(object, topic.instance_id, request.payload, (int) request.payload_len);
    }
    else {
        on_bootstrap_object_write(object, request.payload, (int) request.payload_len);
    }

    lwm2m_response response = {
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .response_code = RESPONSE_CODE_CREATED, // TODO CREATED or CHANGED?
            .payload = "",
            .payload_len = 0,
    };
    return response;
}

lwm2m_response handle_bootstrap_finish_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    on_bootstrap_finish(context);

    lwm2m_response response = {
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .response_code = RESPONSE_CODE_CREATED, // TODO what here ?
            .payload = "",
            .payload_len = 0,
    };
    return response;
}

lwm2m_response handle_write_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    lwm2m_response response = {
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload = "",
            .payload_len = 0,
    };

    lwm2m_server *server = (lwm2m_server *) lfind(context->servers, atoi(topic.server_id));
    lwm2m_object *object = lfind(context->object_tree, topic.object_id);
    lwm2m_instance *instance = lfind(object->instances, topic.instance_id);

    if (topic.resource_id != -1) {
        lwm2m_resource *resource = lfind(instance->resources, topic.resource_id);
        response.response_code = on_resource_write(server, resource, request.payload, (int) request.payload_len);
    } else {
        response.response_code = on_instance_write(server, instance, request.payload, (int) request.payload_len);
    }
    return response;
}

lwm2m_response handle_read_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    lwm2m_server *server = (lwm2m_server *) lfind(context->servers, atoi(topic.server_id));
    lwm2m_object *object = lfind(context->object_tree, topic.object_id);

    if (topic.instance_id != -1 && topic.resource_id != -1) {
        lwm2m_instance *instance = lfind(object->instances, topic.instance_id);
        lwm2m_resource *resource = lfind(instance->resources, topic.resource_id);
        return on_resource_read(server, resource);
    }
    else if (topic.instance_id != -1) {
        lwm2m_instance *instance = lfind(object->instances, topic.instance_id);
        return on_instance_read(server, instance);
    } else {
        return on_object_read(server, object);
    }
}

lwm2m_response handle_create_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    lwm2m_server *server = __resolve_server(context, topic);
    lwm2m_object *object = __resolve_object(context, topic);
    return on_instance_create(server, object, topic.instance_id, request.payload, (int) request.payload_len);
}

lwm2m_response handle_delete_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    lwm2m_server *server = __resolve_server(context, topic);
    lwm2m_object *object = __resolve_object(context, topic);
    lwm2m_instance *instance = __resolve_instance(object, topic);
    return on_instance_delete(server, instance);
}

lwm2m_response handle_observe_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    lwm2m_server *server = __resolve_server(context, topic);
    lwm2m_object *object = __resolve_object(context, topic);
    lwm2m_instance *instance = __resolve_instance(object, topic);
    lwm2m_resource *resource = __resolve_resource(instance, topic);

    if (resource != NULL) {
        return on_resource_observe(server, resource, topic.token);
    }
    if (instance != NULL) {
        return on_instance_observe(server, instance, topic.token);
    }
    return on_object_observe(server, object, topic.token);
}

lwm2m_response handle_cancel_observe_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    lwm2m_server *server = __resolve_server(context, topic);
    lwm2m_object *object = __resolve_object(context, topic);
    lwm2m_instance *instance = __resolve_instance(object, topic);
    lwm2m_resource *resource = __resolve_resource(instance, topic);

    if (resource != NULL) {
        return on_resource_cancel_observe(server, resource);
    }
    if (instance != NULL) {
        return on_instance_cancel_observe(server, instance);
    }
    return on_object_cancel_observe(server, object);
}

lwm2m_response handle_discover_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    lwm2m_server *server = __resolve_server(context, topic);
    lwm2m_object *object = __resolve_object(context, topic);
    lwm2m_instance *instance = __resolve_instance(object, topic);
    lwm2m_resource *resource = __resolve_resource(instance, topic);

    if (resource != NULL) {
        return on_resource_discover(server, resource);
    }
    if (instance != NULL) {
        return on_instance_discover(server, instance);
    }
    return on_object_discover(server, object);
}

lwm2m_response handle_write_attributes_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    lwm2m_server *server = __resolve_server(context, topic);
    lwm2m_object *object = __resolve_object(context, topic);
    lwm2m_instance *instance = __resolve_instance(object, topic);
    lwm2m_resource *resource = __resolve_resource(instance, topic);

    if (resource != NULL) {
        return on_resource_write_attributes(server, resource, request);
    }
    if (instance != NULL) {
        return on_instance_write_attributes(server, instance, request);
    }
    return on_object_write_attributes(server, object, request);
}

lwm2m_response handle_execute_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    lwm2m_server *server = __resolve_server(context, topic);
    lwm2m_object *object = __resolve_object(context, topic);
    lwm2m_instance *instance = __resolve_instance(object, topic);
    lwm2m_resource *resource = __resolve_resource(instance, topic);
    list *args = __parse_args(request);

    return on_resource_execute(server, resource, args);
}

///////// PERFORM

static void __free_request(lwm2m_request *request) {
    if (request->payload_len > 0) {
        free(request->payload);
    }
}

static void __free_register_request(lwm2m_register_request *request) {
    if (request->payload_len > 0) {
        free(request->payload);
    }
    if (request->header != NULL) {
        free(request->header);
    }
}

static void __free_response(lwm2m_response *response) {
    free(response->payload);
}

static void __free_topic(lwm2m_topic *topic) {
    free(topic->token);
    free(topic->server_id);
}

void perform_bootstrap_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    char message[1000];
    char topic_str[100];
    int message_len;

    serialize_topic(topic, topic_str);
    serialize_request(request, message, &message_len);
    __free_request(&request);
    __free_topic(&topic);

    publish(context, topic_str, message, message_len);
}

void perform_deregister_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_request request) {
    char message[1000];
    char topic_str[100];
    int message_len;

    serialize_topic(topic, topic_str);
    serialize_request(request, message, &message_len);
    __free_request(&request);
    __free_topic(&topic);

    publish(context, topic_str, message, message_len);
}

void perform_register_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_register_request request) {
    char message[1000];
    char topic_str[100];
    int message_len;

    serialize_topic(topic, topic_str);
    serialize_register_request(request, message, &message_len);
    __free_register_request(&request);
    __free_topic(&topic);

    publish(context, topic_str, message, message_len);
}

void perform_update_request(lwm2m_context *context, lwm2m_topic topic, lwm2m_register_request request) {
    char message[1000];
    char topic_str[100];
    int message_len;

    serialize_topic(topic, topic_str);
    serialize_register_request(request, message, &message_len);
    __free_register_request(&request);
    __free_topic(&topic);

    publish(context, topic_str, message, message_len);
}

void perform_notify_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response) {
    char message[1000];
    char topic_str[100];
    int message_len;

    serialize_topic(topic, topic_str);
    serialize_response(response, message, &message_len);
    __free_response(&response);

    publish(context, topic_str, message, message_len);
}

void handle_deregister_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response) {
    lwm2m_server *server = lfind(context->servers, atoi(topic.server_id));
    on_server_deregister(server, response.response_code);
}

void handle_register_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response) {
    lwm2m_server *server = lfind(context->servers, atoi(topic.server_id));
    on_server_register(server, response.response_code);
}

void handle_update_response(lwm2m_context *context, lwm2m_topic topic, lwm2m_response response) {
    lwm2m_server *server = lfind(context->servers, atoi(topic.server_id));
    on_server_update(server, response.response_code);
}