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


// FIXME: implementation below is not thread-safe.
// Fix this by implementing a Monitor.

void inc_write() {
    n_writes++;
}

void inc_read() {
    n_reads++;
}

void inc_delete() {
    n_deletes++;
}

void inc_fail() {
    n_fails++;
}


int get_writes() {
    return n_writes;
}

int get_reads() {
    return n_reads;
}

int get_deletes() {
    return n_deletes;
}

int get_fails() {
    return n_fails;
}
