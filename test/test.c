#include <lwm2m_client.h>
#include <lwm2m_transport_mqtt.h>

#define TEST_OBJECT_ID 20004

volatile int reads_executed = 0;
volatile int finished = 0;
char *client_id;
int times;

pthread_t exit_thread;
pthread_mutex_t lock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;;
pthread_cond_t condition = (pthread_cond_t) PTHREAD_COND_INITIALIZER;;

static lwm2m_resource *create_test_object_resources() {
    lwm2m_resource *resources = (lwm2m_resource *) malloc(3 * sizeof(lwm2m_resource));

    resources[0].multiple = false;
    resources[0].id = 0;
    resources[0].name = "Client ID";
    resources[0].type = STRING;
    resources[0].mandatory = true;
    resources[0].operations = READ;
    resources[0].read_callback = NULL;
    resources[0].write_callback = NULL;

    resources[1].multiple = false;
    resources[1].id = 1;
    resources[1].name = "Server ID";
    resources[1].type = STRING;
    resources[1].mandatory = true;
    resources[1].operations = READ;
    resources[1].read_callback = NULL;
    resources[1].write_callback = NULL;

    resources[2].multiple = false;
    resources[2].id = 2;
    resources[2].name = "Payload";
    resources[2].type = STRING;
    resources[2].mandatory = false;
    resources[2].operations = READ | WRITE;
    resources[2].read_callback = NULL;
    resources[2].write_callback = NULL;

    return resources;
}

static list *create_objects() {
    lwm2m_object *test_object = lwm2m_object_new();
    test_object->id = TEST_OBJECT_ID;
    test_object->mandatory = false;
    test_object->multiple = true;
    test_object->object_urn = "lynx:lwm2m:4";
    test_object->attributes = list_new();
    test_object->resource_def = create_test_object_resources();
    test_object->resource_def_len = 3;

    list *objects = list_new();
    ladd(objects, test_object->id, (void *) test_object);
    return objects;
}

void on_read(lwm2m_resource *resource) {
    pthread_mutex_lock(&lock);
    reads_executed += 1;
    if (reads_executed >= times) {
        pthread_cond_signal(&condition);
    }
    pthread_mutex_unlock(&lock);
}

// TODO FACTORY BOOTSTRAP IN COAP TEST (AND INCREASE COUNTER)
int perform_factory_bootstrap(lwm2m_context *context) {
    lwm2m_object *test_object = lfind(context->objects, TEST_OBJECT_ID);
    lwm2m_instance *instance = lwm2m_instance_new_with_id(test_object, 0);
    lwm2m_resource *client_id_resource = lfind(instance->resources, 0);
    lwm2m_resource *server_id_resource = lfind(instance->resources, 1);
    __set_value_string(client_id_resource, client_id);
    __set_value_string(server_id_resource, "global");
    client_id_resource->read_callback = on_read;
    return 0;
}

void deregister_all(lwm2m_context *context) {
    printf("Reads executed: %d\n", reads_executed);
    fflush(stdout);
    if (reads_executed >= times || finished) {
        /** Deregister from all servers **/
        for (list_elem *elem = context->servers->first; elem != NULL; elem = elem->next) {
            lwm2m_server *server = elem->value;
            deregister(server);
        }
        /** Stop transport **/
        stop_mqtt(context);
        stop_scheduler(context->scheduler);
    }
}

void *exit_func(void *context_void) {
    pthread_mutex_lock(&lock);
    while (reads_executed < times && !finished) {
        pthread_cond_wait(&condition, &lock);
    }
    deregister_all(context_void);
    pthread_mutex_unlock(&lock);
    printf("Stopped test for client %s\n", client_id);
    exit(0);
}

/**
 * Usage:
 * ./test local_test_1 1 42.12.2.1:1883    <- test with TLS on port 1883
 * ./test local_test_2 0 localhost:8883    <- test without TLS on port 8883
 *
 * Factory bootstrap creates instance of TestObject. When it was read 1000 times, test stops.
 */
int main(int argc, char *argv[]) {
    client_id = argc > 1 ? argv[1] : "local_test_1";
    char *tls = argc > 2 ? argv[2] : "0";
    char *broker = argc > 3 ? argv[3] : "ec2-34-250-196-139.eu-west-1.compute.amazonaws.com:1883";
    times = argc > 4 ? atoi(argv[4]) : 20;


    printf("hello\n");
    fflush(stdout);

    /** Configure client from arguments **/
    lwm2m_context *context = lwm2m_create_context();
    context->factory_bootstrap_callback = perform_factory_bootstrap;
    context->objects = create_objects();
    context->client_id = client_id;
    context->endpoint_client_name = client_id;
    context->tls = !strcmp(tls, "1");
    context->broker_address = broker;
    context->qos = 0;

    /** Start client from arguments **/
    lwm2m_start_client(context);

    /** Wait for reads_executed **/
    pthread_create(&exit_thread, NULL, exit_func, context);

    /** Wait for cancel **/
    getchar();
    finished = 1;
    printf("Canceled test\n");
    fflush(stdout);
    deregister_all(context);
    printf("Stopped test for client %s\n", client_id);
}