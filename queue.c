#include "queue.h"

// Exercise 2: implement a concurrent queue

// TODO: define your synchronization variables here
// (hint: don't forget to initialize them)


// add a new task to the end of the queue
// NOTE: queue must be implemented as a monitor
void enqueue(queue_t *q, task_t *t) {
    /* TODO: your code here */

}

// fetch a task from the head of the queue.
// if the queue is empty, the thread should wait.
// NOTE: queue must be implemented as a monitor
task_t* dequeue(queue_t *q) {
    /* TODO: your code here */

    return NULL;
}

// return the number of tasks in the queue.
// NOTE: queue must be implemented as a monitor
int queue_count(queue_t *q) {
    /* TODO: your code here */

    return 0;
}
