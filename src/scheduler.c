#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>

// todo is it needed?
#define _XOPEN_SOURCE 500

/****************** LIST IMPLEMENTATION *********************/

void list_add(task_list *list, scheduler_task *task) {
    task_element *new_element = (task_element *) malloc(sizeof(task_element));
    new_element->next = list->first;
    new_element->task = task;
    list->first = new_element;
}

void list_remove(task_list *list, int id_to_remove) {
    task_element *prev = NULL;
    task_element *curr = list->first;

    while (curr->task->id != id_to_remove) {
        prev = curr;
        curr = curr->next;
        if (curr == NULL) {
            return; // Did not find
        }
    }

    if (prev == NULL) {
        list->first = curr->next;
    } else {
        prev->next = curr->next;
    }
    free(curr);
}

/*********************** SCHEDULER **************************/

int generate_task_id() {
    srand(time(0));
    return rand();
}


time_t find_earliest_waking_time(list *l) {
    time_t *earliest_time = NULL;
    for (list_elem *elem = l->first; elem != NULL; elem = elem->next) {
        scheduler_task* task = elem->value;
        if (earliest_time == NULL) {
            earliest_time = malloc(sizeof(time_t));
            *earliest_time = task->waking_time;
        }
        if (difftime(*earliest_time, task->waking_time) > 0) {
            *earliest_time = task->waking_time;
        }
    }
    time_t time = *earliest_time;
    free(earliest_time);
    return time;
}


void await(lwm2m_scheduler *scheduler) {
    if (scheduler->l->size == 0) {
        pthread_cond_wait(&scheduler->condition, &scheduler->lock);
    } else {
        sem_wait(&scheduler->guard);
        time_t waking_time = find_earliest_waking_time(scheduler->l);
        sem_post(&scheduler->guard);

        struct timespec waking_time_spec;
        waking_time_spec.tv_sec = waking_time + 1; // hack, because sometimes pthread_cond_timedwait wake up one second too early
        waking_time_spec.tv_nsec = 0;

        pthread_cond_timedwait(&scheduler->condition, &scheduler->lock, &waking_time_spec);
    }
}

void schedule(lwm2m_scheduler *scheduler, scheduler_task *task) {
    pthread_mutex_lock(&scheduler->lock);

    sem_wait(&scheduler->guard);
    task->waking_time = time(0) + task->period;
    ladd(scheduler->l, task->id, task);
    sem_post(&scheduler->guard);

    pthread_cond_signal(&scheduler->condition);
    pthread_mutex_unlock(&scheduler->lock);
}

void cancel(lwm2m_scheduler *scheduler, scheduler_task *task) {
    pthread_mutex_lock(&scheduler->lock);

    printf("Cancel\n");
    fflush(stdout);

    sem_wait(&scheduler->guard);
    lremove(scheduler->l, task->id);
    sem_post(&scheduler->guard);

    pthread_cond_signal(&scheduler->condition);
    pthread_mutex_unlock(&scheduler->lock);
}

void execute(lwm2m_scheduler *scheduler, scheduler_task *task, void* arg) {
    pthread_mutex_lock(&scheduler->lock);

    task->function(task->arg0, task->arg1, task->arg2, task->arg3, arg);
    task->waking_time = time(0) + task->period;
    task->last_waking_time = time(0);

    pthread_cond_signal(&scheduler->condition);
    pthread_mutex_unlock(&scheduler->lock);
}

void process_tasks(lwm2m_scheduler *scheduler) {
    time_t now = time(0);

    for (list_elem *elem = scheduler->l->first; elem != NULL; elem = elem->next) {
        scheduler_task *task = elem->value;
        if (difftime(task->waking_time, now) <= 0) {
            // Execute without waking - we are already awake here
            task->function(task->arg0, task->arg1, task->arg2, task->arg3, NULL);
            task->waking_time = time(0) + task->period;
        }
    }
}

void stop_scheduler(lwm2m_scheduler *scheduler) {
    pthread_mutex_lock(&scheduler->lock);
    scheduler->stop = true;
    pthread_cond_signal(&scheduler->condition);
    pthread_mutex_unlock(&scheduler->lock);
}

void *scheduler_thread(void *scheduler_void) {
    // Init scheduler
    lwm2m_scheduler *scheduler = (lwm2m_scheduler *) scheduler_void;

    while (true) {
        pthread_mutex_lock(&scheduler->lock);
        await(scheduler);
        if (scheduler->stop) {
            return NULL;
        }
        sem_wait(&scheduler->guard);
        process_tasks(scheduler);
        sem_post(&scheduler->guard);

        pthread_cond_signal(&scheduler->condition);
        pthread_mutex_unlock(&scheduler->lock);
    }
    return NULL;
}

void scheduler_start(lwm2m_scheduler *scheduler) {
    scheduler->l = list_new();
    scheduler->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    scheduler->condition = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    scheduler->stop = false;
    sem_init(&scheduler->guard, 0, 1);

    if (pthread_create(&scheduler->thread, NULL, scheduler_thread, scheduler)) {
        printf("Error creating scheduler thread");
    }
}