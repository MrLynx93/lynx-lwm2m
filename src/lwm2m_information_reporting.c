#include "../include/lwm2m_information_reporting.h"
#include "../include/lwm2m_errors.h"
#include <pthread.h>
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
//pthread_mutex mutex;
//list<session> sessions;
//session waking_session;
//
//typedef struct lwm2m_notify_context {
//    lwm2m_notify_session waking_session;
//    lwm2m_notify_session sessions[MAX_SESSIONS];
//    int sessions_count;
//
//    pthread_mutex_t sessions_mutex;
//    pthread_mutex_t mutex;
//    pthread_cond_t wake_condition;
//
//};
//
//
//static void notify_thread (void *context) {
//    lwm2m_notify_context* notify_context = (lwm2m_notify_context*) context;
//    pthread_mutex_t *mutex = &notify_context.mutex;
//    pthread_cond_t *wake_condition = &notify_context.wake_condition;
//
//    long start_sleep_timestamp = now();
//    long last_sleep_time;
//    long minimum_time = 0L;
//
//    while (true) {
//        // Start sleeping
//        waking_session = NULL;
//        start_sleep_timestamp = now();
//        pthread_mutex_lock(mutex);
//        pthread_cond_timedwait(*mutex, *wake_condition, minimum_time);
//
//        // Wake up
//        last_sleep_time = now() - start_sleep_timestamp;
//        minimum_time = LONG_MAX;
//
//        for (session : sessions) {
//            if (should_notify(session, session == waking_session)) {
//                do_notify(session);
//                session.time_left = maximum_time;
//            }
//            else {
//                session.time_left -= last_sleep_time;
//            }
//            minimum_time = session.time_left < minimum_time ? session.time_left : minimum_time;
//        }
//
//
//        sleep_mutex(mutex, minimum_time);
//    }
//}
//
//bool should_notify(session session, bool is_waking_session) {
//
//}
//
//do_notify(session session) {
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
