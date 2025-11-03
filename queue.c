#include "queue.h"

// Exercise 2: implement a concurrent queue

// TODO: define your synchronization variables here
// (hint: don't forget to initialize them)

// Monitor: mutex and condition variable for the queue
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;


// add a new task to the end of the queue
// NOTE: queue must be implemented as a monitor
void enqueue(queue_t *q, task_t *t) {
    /* TODO: your code here */

    // Monitor entry - acquire lock
    pthread_mutex_lock(&queue_mutex);

    // Add task to the end of the queue
    t->next = NULL;
    if (q->tail == NULL) {
        // Empty queue
        q->head = t;
        q->tail = t;
    } else {
        // Non-empty queue
        q->tail->next = t;
        q->tail = t;
    }
    q->count++;

    // Signal waiting threads that there is a new task
    pthread_cond_signal(&queue_cond);

    // Monitor exit - release lock
    pthread_mutex_unlock(&queue_mutex);
}

// fetch a task from the head of the queue.
// if the queue is empty, the thread should wait.
// NOTE: queue must be implemented as a monitor
task_t* dequeue(queue_t *q) {
    /* TODO: your code here */

    // Monitor entry - acquire lock
    pthread_mutex_lock(&queue_mutex);

    // Wait while the queue is empty
    while (q->head == NULL) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    // Dequeue from head
    task_t *task = q->head;
    q->head = task->next;
    if (q->head == NULL) {
        // Queue is now empty
        q->tail = NULL;
    }
    q->count--;

    // Monitor exit - release lock
    pthread_mutex_unlock(&queue_mutex);

    return task;
}

// return the number of tasks in the queue.
// NOTE: queue must be implemented as a monitor
int queue_count(queue_t *q) {
    /* TODO: your code here */

    // Monitor entry - acquire lock
    pthread_mutex_lock(&queue_mutex);

    int count = q->count;

    // Monitor exit - release lock
    pthread_mutex_unlock(&queue_mutex);

    return count;
}
