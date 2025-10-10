#include "lab3.h"
#include "queue.h"
#include "kvstore.h"

#ifndef __LAB3_WORKER_H__
#define __LAB3_WORKER_H__

typedef struct args {
    queue_t *q;
    kvstore_t *kv;
} args_t;


void *worker(void *args);


#endif
