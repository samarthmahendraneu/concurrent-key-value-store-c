#include "stats.h"

// Exercise 3: fix concurrency bugs by Monitor

// FIXME:
// These statistics should be implemented as a Monitor,
// which keeps track of dbserver's status

int n_writes = 0;  // number of writes
int n_reads = 0;   // number of reads
int n_deletes = 0; // number of deletes
int n_fails = 0;   // number of failed operations


// TODO: define your synchronization variables here
// (hint: don't forget to initialize them)

/* your code here */

// Monitor: mutex to protect statistics
static pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;


// FIXME: implementation below is not thread-safe.
// Fix this by implementing a Monitor.

void inc_write() {
    pthread_mutex_lock(&stats_mutex);
    n_writes++;
    pthread_mutex_unlock(&stats_mutex);
}

void inc_read() {
    pthread_mutex_lock(&stats_mutex);
    n_reads++;
    pthread_mutex_unlock(&stats_mutex);
}

void inc_delete() {
    pthread_mutex_lock(&stats_mutex);
    n_deletes++;
    pthread_mutex_unlock(&stats_mutex);
}

void inc_fail() {
    pthread_mutex_lock(&stats_mutex);
    n_fails++;
    pthread_mutex_unlock(&stats_mutex);
}


int get_writes() {
    pthread_mutex_lock(&stats_mutex);
    int val = n_writes;
    pthread_mutex_unlock(&stats_mutex);
    return val;
}

int get_reads() {
    pthread_mutex_lock(&stats_mutex);
    int val = n_reads;
    pthread_mutex_unlock(&stats_mutex);
    return val;
}

int get_deletes() {
    pthread_mutex_lock(&stats_mutex);
    int val = n_deletes;
    pthread_mutex_unlock(&stats_mutex);
    return val;
}

int get_fails() {
    pthread_mutex_lock(&stats_mutex);
    int val = n_fails;
    pthread_mutex_unlock(&stats_mutex);
    return val;
}
