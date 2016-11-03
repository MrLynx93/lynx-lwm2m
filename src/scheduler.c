#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


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


time_t find_earliest_waking_time(task_list *list) {
    task_element *curr = list->first;
    time_t earliest_time = curr->task->waking_time;

    while (curr != NULL) {
        if (difftime(curr->task->waking_time, earliest_time) < 0) {
            earliest_time = curr->task->waking_time;
        }
        curr = curr->next;
    }
    return earliest_time;
}


void await(lwm2m_scheduler *scheduler) {
    if (scheduler->queue->first == NULL) {
        pthread_cond_wait(&scheduler->condition, &scheduler->lock);
    } else {
        sem_wait(&scheduler->guard);
        time_t waking_time = find_earliest_waking_time(scheduler->queue);
        sem_post(&scheduler->guard);

        struct timespec waking_time_spec;
        waking_time_spec.tv_sec = waking_time;
        waking_time_spec.tv_nsec = 0;

        pthread_cond_timedwait(&scheduler->condition, &scheduler->lock, &waking_time_spec);
    }
}

void schedule(lwm2m_scheduler *scheduler, scheduler_task *task) {
    pthread_mutex_lock(&scheduler->lock);

    sem_wait(&scheduler->guard);
    task->waking_time = time(0) + task->period;
    list_add(scheduler->queue, task);
    sem_post(&scheduler->guard);

    pthread_cond_signal(&scheduler->condition);
    pthread_mutex_unlock(&scheduler->lock);
}

void cancel(lwm2m_scheduler *scheduler, scheduler_task *task) {
    pthread_mutex_lock(&scheduler->lock);

    sem_wait(&scheduler->guard);
    list_remove(scheduler->queue, task->id);
    sem_post(&scheduler->guard);

    pthread_cond_signal(&scheduler->condition);
    pthread_mutex_unlock(&scheduler->lock);

}

void execute(lwm2m_scheduler *scheduler, scheduler_task *task) {
    task->function(task->arg1, task->arg2);
    task->waking_time = time(0) + task->period;
    pthread_cond_signal(&scheduler->condition);
}

void process_tasks(lwm2m_scheduler *scheduler) {
    task_element *curr = scheduler->queue->first;
    time_t now = time(0);

    while (curr != NULL) {
        if (difftime(curr->task->waking_time, now) < 0) {
            // Execute without waking - we are already awake here
            curr->task->function(curr->task->arg1, curr->task->arg2);
            curr->task->waking_time = time(0) + curr->task->period;
        }
        curr = curr->next;
    }
}

void *scheduler_thread(void *scheduler_void) {
    // Init scheduler
    lwm2m_scheduler *scheduler = (lwm2m_scheduler *) scheduler_void;


    while (true) {
        pthread_mutex_lock(&scheduler->lock);
        await(scheduler);

        sem_wait(&scheduler->guard);
        process_tasks(scheduler);
        sem_post(&scheduler->guard);

        pthread_mutex_unlock(&scheduler->lock);
    }
    return NULL;
}

void scheduler_start(lwm2m_scheduler *scheduler) {
    scheduler->queue = (task_list *) malloc(sizeof(task_list));
    scheduler->queue->first = NULL;
    scheduler->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    scheduler->condition = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    sem_init(&scheduler->guard, 0, 1);

    if (pthread_create(&scheduler->thread, NULL, scheduler_thread, scheduler)) {
        printf("Error creating scheduler thread");
    }
}