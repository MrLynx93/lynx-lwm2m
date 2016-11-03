#ifndef LYNX_LWM2M_SCHEDULER_H
#define LYNX_LWM2M_SCHEDULER_H

#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>

/********* LIST **********/

typedef struct task_element task_element;

typedef struct task_list {
    task_element* first;
} task_list;

/********* SCHEDULER **********/

typedef void (*scheduler_func)(void *, void *);

typedef struct scheduler_task {
    int id;
    int period; /* period in seconds */
    time_t waking_time;

    scheduler_func function;
    void* arg1;
    void* arg2;

} scheduler_task;

typedef struct lwm2m_scheduler {
    pthread_t thread;
    pthread_mutex_t lock;
    pthread_cond_t condition;
    sem_t guard;

    task_list *queue;
} lwm2m_scheduler;

struct task_element {
    task_element *next;
    scheduler_task *task;
};

/** Generate random unique task id **/
int generate_task_id();

/** Initializes thread, conditions etc. **/
void scheduler_start(lwm2m_scheduler *scheduler);

/** Start executing task every {task.period} milliseconds **/
void schedule(lwm2m_scheduler *scheduler, scheduler_task *task);

/** Executes task immediately. Delays next scheduled execution to {task.period} time **/
void execute(lwm2m_scheduler *scheduler, scheduler_task *task);

/** Cancels a task **/
void cancel(lwm2m_scheduler *scheduler, scheduler_task *task);

#endif //LYNX_LWM2M_SCHEDULER_H
