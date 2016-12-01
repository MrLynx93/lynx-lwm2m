#include <lwm2m_transport_mqtt.h>
#include "lwm2m.h"
#include "lwm2m_object.h"
#include "lwm2m_register.h"
#include "lwm2m_transport.h"
#include "lwm2m_parser.h"
#include "scheduler.h"

static bool __has_server_instances(lwm2m_context *context) { // todo extract?
    lwm2m_object *server_object = lfind(context->object_tree, SERVER_OBJECT_ID);
    return server_object->instances->size > 0;
}

static void wait_for_registered(lwm2m_context *context) {
    pthread_mutex_lock(&context->register_mutex);
    while (context->state != REGISTERED) {
        pthread_cond_wait(&context->register_finished_condition, &context->register_mutex);
    }
    pthread_mutex_unlock(&context->register_mutex);
}

static int get_lifetime(lwm2m_instance *server_instance) {
    return ((lwm2m_resource*) lfind(server_instance->resources, LIFETIME_RESOURCE_ID))->value->int_value;
}

static char *get_binding_mode(lwm2m_instance *server_instance) {
    return ((lwm2m_resource*) lfind(server_instance->resources, BINDING_RESOURCE_ID))->value->string_value;
}

static char *serialize_registration_params(lwm2m_server *server) {
    char *params = (char *) malloc(sizeof(char *) * 100);
    strcat(params, "ep=");
    strcat(params, server->context->endpoint_client_name);
    if (get_lifetime(server->server_instance) > 0) {
        strcat(params, "&lt=");
        char *lt = itoa(get_lifetime(server->server_instance));
        strcat(params, lt);
        free(lt);
    }
    if (get_binding_mode(server->server_instance) != NULL) {
        strcat(params, "&b=");
        strcat(params, get_binding_mode(server->server_instance));
    }
    return params;
}

static char *serialize_update_params(lwm2m_server *server) {
    char *params = (char *) malloc(sizeof(char *) * 100);
    bool firstParam = true;

    if (strcmp(server->context->endpoint_client_name, server->last_update_data.endpoint_client_name)) {
        strcat(params, "ep=");
        strcat(params, server->context->endpoint_client_name);
        firstParam = false;
    }
    if (get_lifetime(server->server_instance) != server->last_update_data.lifetime) {
        strcat(params, firstParam ? "ep=" : "&ep=");
        strcat(params, server->context->endpoint_client_name);
        firstParam = false;
    }
    if (strcmp(get_binding_mode(server->server_instance), server->last_update_data.binding_mode)) {
        strcat(params, firstParam ? "b=" : "&b=");
        strcat(params, get_binding_mode(server->server_instance));
    }
    return params;
}


void update_func(void *task, void *server, void *context, void *nothing, void *nothing2) {
    update_on_server((lwm2m_context *) context, (lwm2m_server *) server);
}

///////////////////////// REGISTER ////////////////////////////

void deregister_on_server(lwm2m_context *context, lwm2m_server *server) {
    lwm2m_topic topic = {
            .operation   = LWM2M_OPERATION_DEREGISTER,
            .type        = "req",
            .client_id   = context->client_id,
            .server_id   = itoa(server->short_server_id),
            .token       = generate_token(),
            .object_id   = -1,
            .instance_id = -1,
            .resource_id = -1,
    };
    lwm2m_request request = {
            .content_type = CONTENT_TYPE_NO_FORMAT,
            .payload      = "",
            .payload_len  = 0,
    };

    perform_deregister_request(context, topic, request);
}
void register_on_server(lwm2m_context *context, lwm2m_instance *server_instance) {
    lwm2m_resource *id_resource = lfind(server_instance->resources, SHORT_SERVER_ID_RESOURCE_ID);

    lwm2m_server *server = (lwm2m_server *) malloc(sizeof(lwm2m_server));
    server->context = context;
    server->server_instance = server_instance;
    server->short_server_id = id_resource->value->int_value;
    subscribe_server(context, server);

    char *objects_and_instances = serialize_lwm2m_objects_and_instances(context);
    char *objects_and_instances_copy = (char *) calloc(200, sizeof(char));
    strcpy(objects_and_instances_copy, objects_and_instances);

    char *registration_params = serialize_registration_params(server);

    server->last_update_data = (lwm2m_register_data) {
            .endpoint_client_name = context->endpoint_client_name,
            .object_and_instances = objects_and_instances,
            .binding_mode = get_binding_mode(server->server_instance),
            .lifetime = get_lifetime(server->server_instance),
    };

    lwm2m_topic topic = {
            .operation   = LWM2M_OPERATION_REGISTER,
            .type        = "req",
            .client_id   = context->client_id,
            .server_id   = itoa(server->short_server_id),
            .token       = generate_token(),
            .object_id   = -1,
            .instance_id = -1,
            .resource_id = -1,
    };
    lwm2m_register_request request = {
            .content_type = CONTENT_TYPE_TEXT,
            .header       = registration_params,
            .payload      = objects_and_instances_copy,
            .payload_len  = strlen(objects_and_instances_copy),
    };

    perform_register_request(context, topic, request);
    ladd(context->servers, server->short_server_id, server);
};

void update_on_server(lwm2m_context *context, lwm2m_server *server) {
    char *objects_and_instances = serialize_lwm2m_objects_and_instances(context);
    char *update_params = serialize_update_params(server);

    char *request_payload = NULL;

    if (!strcmp(objects_and_instances, server->last_update_data.object_and_instances)) {
        request_payload = "";
    } else {
        request_payload = malloc(strlen(objects_and_instances) + 1);
        strcpy(request_payload, objects_and_instances);
    }
    free(server->last_update_data.object_and_instances);

    server->last_update_data = (lwm2m_register_data) {
            .endpoint_client_name = context->endpoint_client_name,
            .object_and_instances = objects_and_instances,
            .binding_mode = get_binding_mode(server->server_instance),
            .lifetime = get_lifetime(server->server_instance),
    };

    lwm2m_topic topic = {
            .operation   = LWM2M_OPERATION_UPDATE,
            .type        = "req",
            .client_id   = context->client_id,
            .server_id   = itoa(server->short_server_id),
            .token       = generate_token(),
            .object_id   = -1,
            .instance_id = -1,
            .resource_id = -1,
    };
    lwm2m_register_request request = {
            .content_type = CONTENT_TYPE_TEXT,
            .header       = update_params,
            .payload      = request_payload,
            .payload_len  = strlen(request_payload),
    };

    perform_update_request(context, topic, request);
}

int lwm2m_register(lwm2m_context *context) {
    context->register_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    context->register_finished_condition = (pthread_cond_t) PTHREAD_COND_INITIALIZER;

    if (__has_server_instances(context)) {
        context->state = REGISTERING;

        // Register on all servers in context asynchronously
        lwm2m_object *server_object = lfind(context->object_tree, SERVER_OBJECT_ID);
        list *server_object_instances = server_object->instances;

        for (list_elem *elem = server_object_instances->first; elem != NULL; elem = elem->next) {
            lwm2m_instance *server_instance = elem->value;
            register_on_server(context, server_instance);
        }

        // Wait for async registration to finish TODO check bootstrap sequence
        wait_for_registered(context);
        if (context->registered_servers_num > 0) {
            return 0;
        }
        return -1;
    }
    return -1;
}

void on_server_deregister(lwm2m_server* server, int response_code) {
    lremove(server->context->servers, server->short_server_id);
    scheduler_task *update_task = lfind(server->context->update_tasks, server->short_server_id);
    cancel(server->context->scheduler, update_task);
    printf("Deregistered from server=%d\n", server->short_server_id);
    // TODO free task
    // TODO free server
}

// TODO same in observe
void on_server_register(lwm2m_server *server, int success) {
    scheduler_task *update_task = (scheduler_task *) malloc(sizeof(scheduler_task));
    update_task->id = generate_task_id();
    update_task->period = get_lifetime(server->server_instance) / 2;
    update_task->function = update_func;
    update_task->arg0 = update_task;
    update_task->arg1 = server;
    update_task->arg2 = server->context;
    update_task->arg3 = NULL;

    ladd(server->context->update_tasks, server->short_server_id, update_task);
    schedule(server->context->scheduler, update_task);

    pthread_mutex_lock(&server->context->register_mutex);
    server->context->state = REGISTERED;
    server->context->registered_servers_num += 1;
    pthread_cond_signal(&server->context->register_finished_condition);
    pthread_mutex_unlock(&server->context->register_mutex);
}

void on_server_update(lwm2m_server *server, int success) {
    printf("Updated on server=%d\n", server->short_server_id);
}