//#include "../include/lwm2m_information_reporting.h"
//#include "../include/lwm2m_context.h"
//#include <semaphore.h>
//#include <pthread.h>
//
//sem_t observe_semaphore;
//
//long last_sleep_time;
//long minimum_time;
//lwm2m_observe_session *minimum_time_session;
//pthread_mutex_t sleeping_mutex;
//pthread_cond_t sleeping_condition;
//
//int on_observe(lwm2m_server* server, lwm2m_resource* resource) {
//    int access_error = check_resource_access(server, resource, READ);
//    if (access_error) {
//        return access_error;
//    }
//    start_observing(server, resource);
//}
//
//int cancel_observation(lwm2m_server* server, lwm2m_resource *resource) {
//    stop_observing(server, resource);
//}
//
//static void notify(lwm2m_notify_context *context, lwm2m_observe_session *session) {
//    pthread_mutex_lock(sleeping_mutex);
//    while(context->waking_session != NULL) {
//        pthread_cond_wait(sleeping_mutex, context->sleeping_condition);
//    }
//    context->waking_session = session;
//    pthread_cond_signal(context->sleeping_condition);
//    pthread_mutex_unlock(sleeping_mutex);
//}
//
//static void add_session(lwm2m_notify_context *context, lwm2m_observe_session *session) {
//    sem_wait(observe_semaphore);
//    lwm2m_map_put(context->sessions, session.token, session);
//    sem_post(observe_semaphore);
//}
//
//static void remove_session(lwm2m_notify_context *context, lwm2m_observe_session *session) {
//    sem_wait(observe_semaphore);
//    lwm2m_map_remove(context->sessions, session.token);
//    sem_post(observe_semaphore);
//}
//
//pthread_mutex mutex;
//list<session> sessions;
//session waking_session;
//
//typedef struct lwm2m_notify_context {
//    lwm2m_notify_session waking_session;
//    lwm2m_map* sessions;
//};
//
//static void notify_thread (void *context) {
//    lwm2m_notify_context* notify_context = (lwm2m_notify_context*) context;
//    long start_sleep_timestamp;
//
//    while (true) {
//        start_sleep_timestamp = now();
//        is_waking_session = false;
//
//        // Sleep and wake up
//        pthread_mutex_lock(sleeping_mutex);
//        pthread_cond_timedwait(sleeping_mutex, sleeping_condition, minimum_time);
//        if (notify_context->waking_session == NULL) {
//            waking_session = minimum_time_session;
//            is_waking_session = true;
//        }
//        last_sleep_time = now() - start_sleep_timestamp;
//
//        // Do main work
//        do_notify(waking_session);
//        update_sleep_times(context);
//
//        // Unlock
//        is_waking_session = false;
//        notify_context->waking_session = NULL;
//        pthread_cond_signal(sleeping_condition);
//        pthread_mutex_unlock(&sleeping_mutex);
//    }
//}
//
//static void update_sleep_times(lwm2m_notify_context *notify_context) {
//    sem_wait(observe_semaphore);
//    minimum_time = LONG_MAX;
//    for (session : sessions) {
//        session.time_left = is_waking_session ? session.maximum_time : session.time_left - last_sleep_time;
//        if (session.time_left < minimum_time) {
//            minimum_time = session.time_left;
//            minimum_time_session = session;
//        }
//    }
//    sem_post(observe_semaphore);
//}
//
//static void do_notify(lwm2m_observe_session *session) {
//
//}
//
//
////////////////////////// PRIVATE ////////////////////////
//
//static int check_access(lwm2m_server* server, lwm2m_resource* resource, int operation) {
//    if (!lwm2m_check_resource_access_control(server, resource, READ)) {
//        return ACCESS_RIGHT_PERMISSON_DENIED;
//    }
//    if (!resource->operations & READ) {
//        return OPERATION_NOT_SUPPORTED;
//    }
//    return 0;
//}
//
///**
//    TODO metoda setValue w resource
//    Sytuacja:
//    Dajemy observe na sensorze temperatury (dajemy callback on_read, który odczytuje sensor)
//    Wtedy observe nie działa.
//
//    User sam musi stworzyć wątek, który będzie odczytywał temperaturę z sensora
//    i wołał setValue() na resource
// */