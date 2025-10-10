#include "lab3.h"

#ifndef __LAB3_QUEUE_H__
#define __LAB3_QUEUE_H__

typedef struct task task_t;

struct task {
    task_t *next;
    int fd;
};


typedef struct queue {
    task_t *head;    /* remove from head */
    task_t *tail;    /* append to tail */
    int count;
} queue_t;


// add a new task to the end of the queue
void enqueue(queue_t *q, task_t *t);

// fetch a task from the head of the queue.
// if the queue is empty, the thread should wait.
task_t* dequeue(queue_t *q);

// get number of tasks in the queue
int queue_count(queue_t *q);

#endif
