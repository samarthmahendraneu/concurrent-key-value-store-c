/*
 * db client for CS5600 Lab3
 *
 * this is a really ugly piece of code, need to clean it up before the
 * next time I assign it.
 *
 * Peter Desnoyers, 2020
 *
 * 2021 fall: modified by CS5600 staff
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zlib.h>
#include <pthread.h>
#include <argp.h>
#include <assert.h>

#include "lab3.h"

/* --------- argument parsing ---------- */

static struct argp_option options[] = {
    {"set",          'S', "KEY",  0, "set KEY to VALUE"},
    {"get",          'G', "KEY",  0, "get value for KEY"},
    {"delete",       'D', "KEY",  0, "delete KEY"},
    {"quit",         'q',  0,     0, "send QUIT command"},
    {"test",         'T', "#threads",  0, "run your concurrency test with given number of threads"},
    {0}
};

enum {OP_SET = 1, OP_GET = 2, OP_DELETE = 3, OP_QUIT = 4, OP_TEST = 5};

struct args {
    int nthreads;
    int count;
    int port;
    int max;
    int op;
    char *key;
    char *val;
    char *logfile;
    FILE *logfp;
    struct sockaddr_in addr;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct args *a = state->input;
    switch (key) {
    case ARGP_KEY_INIT:
        a->nthreads = 1;
        a->count = 1000;
        a->port = 5600;
        a->max = 200;
        a->logfp = NULL;
        break;

    case 'l':
        a->logfile = arg;
        if ((a->logfp = fopen(arg, "w")) == NULL)
            fprintf(stderr, "Error opening logfile : %s : %s\n", arg,
                    strerror(errno)), exit(1);
        break;

    case 'q':
        a->op = OP_QUIT;
        break;

    case 'G':
        a->op = OP_GET;
        if (strlen(arg) > 30)
            printf("key must be <= 30 chars\n"), argp_usage(state);
        a->key = arg;
        break;

    case 'S':
        a->op = OP_SET;
        if (strlen(arg) > 30)
            printf("key must be <= 30 chars\n"), argp_usage(state);
        a->key = arg;
        break;

    case 'T':
        a->op = OP_TEST;
        a->nthreads = atoi(arg);
        if (a->nthreads == 0) {
            printf("#threads must be a number\n"), argp_usage(state);
        }
        break;

    case 'D':
        a->op = OP_DELETE;
        if (strlen(arg) > 30)
            printf("key must be <= 30 chars\n"), argp_usage(state);
        a->key = arg;
        break;

    case 'n':
        a->count = atoi(arg); break;

    case 'p':
        a->port = atoi(arg);
        break;

    case ARGP_KEY_ARG:
        if (state->arg_num == 0 && a->op == OP_SET) {
            a->val = arg;
        printf("val set to %s\n", arg);
    }
        else
            argp_usage(state);
        break;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, NULL, NULL};

/* --------- everything else ---------- */

int do_connect(struct sockaddr_in *addr)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0 || connect(sock, (struct sockaddr*)addr, sizeof(*addr)) < 0)
        fprintf(stderr, "can't connect: %s\n", strerror(errno)), exit(0);
    return sock;
}


void do_del(struct args *args, char *name, char *result, int quiet)
{
    int sock = do_connect(&args->addr);

    struct request rq;
    snprintf(rq.name, sizeof(rq.name), "%s", name);

    rq.op_status = 'D';
    int val = write(sock, &rq, sizeof(rq));
    if ((val = read(sock, &rq, sizeof(rq))) < 0)
        printf("DEL: REPLY: READ ERROR: %s\n", strerror(errno));
    else if (val < sizeof(rq))
        printf("DEL: REPLY: SHORT READ: %d\n", val);
    else if (rq.op_status != 'K' && !quiet)
        printf("DEL: FAILED (%c)\n", rq.op_status);
    else if (!quiet)
        printf("ok\n");

    if (result != NULL)
        *result = rq.op_status;

    close(sock);
}

void do_set(struct args *args, char *name, void *data, int len, char *result, int quiet)
{
    int sock = do_connect(&args->addr);

    struct request rq;
    snprintf(rq.name, sizeof(rq.name), "%s", name);
    int val;

    rq.op_status = 'W';
    sprintf(rq.len, "%d", len);
    write(sock, &rq, sizeof(rq));
    write(sock, data, len);
    if ((val = read(sock, &rq, sizeof(rq))) < 0)
        printf("WRITE: REPLY: READ ERROR: %s\n", strerror(errno));
    else if (val < sizeof(rq))
        printf("WRITE: REPLY: SHORT READ: %d\n", val);
    else if (rq.op_status != 'K' && !quiet)
        printf("WRITE: FAILED (%c)\n", rq.op_status);
    else if (!quiet)
        printf("ok\n");

    if (result != NULL)
        *result = rq.op_status;
    close(sock);
}

void do_quit(struct args *args)
{
    int sock = do_connect(&args->addr);
    struct request rq;
    rq.op_status = 'Q';
    write(sock, &rq, sizeof(rq));
    /* if both sides close-on-exit you won't get TIME_WAIT */
}

void do_get(struct args *args, char *name, void *data, int *len_p, char *result)
{
    int val, sock = do_connect(&args->addr);
    struct request rq;
    snprintf(rq.name, sizeof(rq.name), "%s", name);

    rq.op_status = 'R';
    sprintf(rq.len, "%d", 0);
    write(sock, &rq, sizeof(rq));
    if ((val = read(sock, &rq, sizeof(rq))) < 0)
        printf("READ: REPLY: READ ERROR: %s\n", strerror(errno));
    else if (val < sizeof(rq))
        printf("READ: REPLY: SHORT READ: %d\n", val);
    else if (rq.op_status != 'K')
        printf("READ: FAILED (%c)\n", rq.op_status);
    else {
        int len = atoi(rq.len);
        char buf[len];

        for (void *ptr = buf, *max = ptr+len; ptr < max; ) {
            int n = read(sock, ptr, max-ptr);
            if (n < 0) {
                printf("READ DATA: READ ERROR: %s\n", strerror(errno));
                break;
            }
            ptr += n;
        }
        if (data != NULL) {
            memcpy(data, buf, len);
            *len_p = len;
        }
        else
            printf("=\"%.*s\"\n", len, buf);
    }

    if (result != NULL)
        *result = rq.op_status;

    close(sock);
}


// === concurrency test ===
// helper functions
// (feel free to modify helper functions)

// helper function: generate random string
void randstr(char *buf, int len)
{
    for (int i = 0; i < len; i++)
        buf[i] = 'A' + (random() % 25);
}

// helper function: generate random keys
void gen_randkey(char *buf) {
    int len = random() % 30;
    randstr(buf, len);
}

// helper function: generate random values
void gen_randval(char *buf) {
    int len = random() % 4096;
    randstr(buf, len);
}

// === Test implementations ===

// Shared data structures for testing
typedef struct {
    struct args *args;
    int thread_id;
    int num_ops;
} thread_arg_t;

// Test 1: Concurrent writes to different keys
void *worker_write_different_keys(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    char key[32], val[100];

    for (int i = 0; i < targ->num_ops; i++) {
        sprintf(key, "key_%d_%d", targ->thread_id, i);
        sprintf(val, "value_%d_%d", targ->thread_id, i);
        do_set(targ->args, key, val, strlen(val), NULL, 1);
    }

    return NULL;
}

void test_concurrent_writes_different_keys(struct args *a, int num_threads) {
    pthread_t threads[num_threads];
    thread_arg_t targs[num_threads];
    int ops_per_thread = 10;

    // Launch threads
    for (int i = 0; i < num_threads; i++) {
        targs[i].args = a;
        targs[i].thread_id = i;
        targs[i].num_ops = ops_per_thread;
        pthread_create(&threads[i], NULL, worker_write_different_keys, &targs[i]);
    }

    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Verify all keys exist with correct values
    char key[32], val[100], data[4096];
    int len;
    char result;
    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < ops_per_thread; j++) {
            sprintf(key, "key_%d_%d", i, j);
            sprintf(val, "value_%d_%d", i, j);

            len = sizeof(data);
            do_get(a, key, data, &len, &result);
            data[len] = '\0';

            assert(result == 'K');
            assert(strcmp(data, val) == 0);
        }
    }

    printf("PASS: All %d writes verified\n", num_threads * ops_per_thread);
}

// Test 2: Concurrent writes to the same key
void *worker_write_same_key(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    char val[100];

    for (int i = 0; i < targ->num_ops; i++) {
        sprintf(val, "value_thread_%d_op_%d", targ->thread_id, i);
        do_set(targ->args, "shared_key", val, strlen(val), NULL, 1);
    }

    return NULL;
}

void test_concurrent_writes_same_key(struct args *a, int num_threads) {
    pthread_t threads[num_threads];
    thread_arg_t targs[num_threads];
    int ops_per_thread = 20;

    // Launch threads
    for (int i = 0; i < num_threads; i++) {
        targs[i].args = a;
        targs[i].thread_id = i;
        targs[i].num_ops = ops_per_thread;
        pthread_create(&threads[i], NULL, worker_write_same_key, &targs[i]);
    }

    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Verify the key exists (value may be from any thread)
    char data[4096];
    int len = sizeof(data);
    char result;
    do_get(a, "shared_key", data, &len, &result);
    assert(result == 'K');

    printf("PASS: Concurrent writes to same key completed\n");
}

// Test 3: Concurrent reads and writes
void *worker_read_write(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    char key[32], val[100], data[4096];
    int len;
    char result;

    for (int i = 0; i < targ->num_ops; i++) {
        sprintf(key, "rw_key_%d", i % 5);
        sprintf(val, "rw_val_%d_%d", targ->thread_id, i);

        // Write
        do_set(targ->args, key, val, strlen(val), NULL, 1);

        // Read back immediately
        len = sizeof(data);
        do_get(targ->args, key, data, &len, &result);

        // Should succeed (may get value from this or another thread)
        assert(result == 'K');
    }

    return NULL;
}

void test_concurrent_reads_writes(struct args *a, int num_threads) {
    pthread_t threads[num_threads];
    thread_arg_t targs[num_threads];
    int ops_per_thread = 15;

    // Launch threads
    for (int i = 0; i < num_threads; i++) {
        targs[i].args = a;
        targs[i].thread_id = i;
        targs[i].num_ops = ops_per_thread;
        pthread_create(&threads[i], NULL, worker_read_write, &targs[i]);
    }

    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("PASS: Concurrent reads and writes completed\n");
}

// Test 4: Concurrent deletes
void *worker_delete(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    char key[32];

    for (int i = 0; i < targ->num_ops; i++) {
        sprintf(key, "del_key_%d", i);
        do_del(targ->args, key, NULL, 1);
    }

    return NULL;
}

void test_concurrent_deletes(struct args *a, int num_threads) {
    int num_keys = 20;

    // First, create keys
    char key[32], val[100];
    for (int i = 0; i < num_keys; i++) {
        sprintf(key, "del_key_%d", i);
        sprintf(val, "del_val_%d", i);
        do_set(a, key, val, strlen(val), NULL, 1);
    }

    pthread_t threads[num_threads];
    thread_arg_t targs[num_threads];

    // Launch threads to delete concurrently
    for (int i = 0; i < num_threads; i++) {
        targs[i].args = a;
        targs[i].thread_id = i;
        targs[i].num_ops = num_keys;
        pthread_create(&threads[i], NULL, worker_delete, &targs[i]);
    }

    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Verify all keys are deleted
    char data[4096];
    int len;
    char result;
    for (int i = 0; i < num_keys; i++) {
        sprintf(key, "del_key_%d", i);
        len = sizeof(data);
        do_get(a, key, data, &len, &result);
        assert(result == 'X'); // Should fail
    }

    printf("PASS: All deletes completed successfully\n");
}

// Test 5: Mixed random operations
void *worker_mixed(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    char key[32], val[100], data[4096];
    int len;

    for (int i = 0; i < targ->num_ops; i++) {
        int op = random() % 3;
        sprintf(key, "mix_key_%d", (int)(random() % 10));
        sprintf(val, "mix_val_%d_%d", targ->thread_id, i);

        if (op == 0) {
            // Write
            do_set(targ->args, key, val, strlen(val), NULL, 1);
        } else if (op == 1) {
            // Read
            len = sizeof(data);
            do_get(targ->args, key, data, &len, NULL);
        } else {
            // Delete
            do_del(targ->args, key, NULL, 1);
        }
    }

    return NULL;
}

void test_mixed_operations(struct args *a, int num_threads) {
    pthread_t threads[num_threads];
    thread_arg_t targs[num_threads];
    int ops_per_thread = 30;

    // Launch threads
    for (int i = 0; i < num_threads; i++) {
        targs[i].args = a;
        targs[i].thread_id = i;
        targs[i].num_ops = ops_per_thread;
        pthread_create(&threads[i], NULL, worker_mixed, &targs[i]);
    }

    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("PASS: Mixed operations completed\n");
}

// Test 6: Stress test
void *worker_stress(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    char key[32], val[200], data[4096];
    int len;
    char result;

    for (int i = 0; i < targ->num_ops; i++) {
        sprintf(key, "stress_%d", i % 50);
        sprintf(val, "stress_value_thread_%d_iteration_%d_with_more_data",
                targ->thread_id, i);

        // Write
        do_set(targ->args, key, val, strlen(val), NULL, 1);

        // Sometimes read back
        if (i % 3 == 0) {
            len = sizeof(data);
            do_get(targ->args, key, data, &len, &result);
            if (result == 'K') {
                data[len] = '\0';
                assert(strlen(data) > 0);
            }
        }

        // Sometimes delete
        if (i % 7 == 0) {
            do_del(targ->args, key, NULL, 1);
        }
    }

    return NULL;
}

void test_stress(struct args *a, int num_threads) {
    pthread_t threads[num_threads];
    thread_arg_t targs[num_threads];
    int ops_per_thread = 50;

    // Launch threads
    for (int i = 0; i < num_threads; i++) {
        targs[i].args = a;
        targs[i].thread_id = i;
        targs[i].num_ops = ops_per_thread;
        pthread_create(&threads[i], NULL, worker_stress, &targs[i]);
    }

    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("PASS: Stress test with %d total operations completed\n",
           num_threads * ops_per_thread);
}

/*
 * Exercise 5:
 * Write your own concurrency tests.
 *
 * You should (these are *suggestions* not requirements):
 *
 *  - create multiple threads and send concurrent operations to dbserver
 *
 *  - try to produce unnatural schedules and interleaving too
 *
 *  - you don't have to follow the six rules or always use monitor here
 *    because this is for debugging purpose. For example,
 *
 *    -- you can use sleep,
 *
 *    -- and feel free to use synchronization primitives (e.g., mutexes)
 *    whatever way you like
 *
 *  - to check correctness, we suggest you to compare your writes and check
 *  the return values from reads. How? remember your write values and compare
 *  the values when you read from the same keys.
 *
 *  - hints:
 *    -- read "do_get/do_set/do_del" to see how to issue these operations
 *    -- you can use the above helper functions to generate keys/values
 *    -- if you want share data between multiple threads, protect the shared data with mutex!
 *       (note: you will not lose points due to coding style in Exercise 5.)
 */
void do_test(struct args *a)
{
    // below is a simple single-threaded test
    char data[4096];
    int len = 4096;
    char result;

    do_set(a, "key0", "val0", strlen("val0"), NULL, 1);
    do_get(a, "key0", &data, &len, NULL);
    data[len] = '\0';    // why we add this? for strcmp
    assert (strcmp(data,"val0") == 0);

    do_del(a, "key0", NULL, 1);
    printf("Expected to see: \"READ: FAILED (X)\" below\n");
    do_get(a, "key0", &data, &len, &result); // should have a READ ERROR
    data[len] = '\0';
    assert (result == 'X');


    /* TODO: your code here */
    // fix random seed; easier to reproduce your bugs
    srandom(5600);

    int num_threads = a->nthreads;

    printf("=== Starting concurrency tests with %d threads ===\n", num_threads);

    // Test 1: Concurrent writes to different keys
    printf("\n[Test 1] Concurrent writes to different keys...\n");
    test_concurrent_writes_different_keys(a, num_threads);

    // Test 2: Concurrent writes to the same key
    printf("\n[Test 2] Concurrent writes to the same key...\n");
    test_concurrent_writes_same_key(a, num_threads);

    // Test 3: Concurrent reads and writes
    printf("\n[Test 3] Concurrent reads and writes...\n");
    test_concurrent_reads_writes(a, num_threads);

    // Test 4: Concurrent deletes
    printf("\n[Test 4] Concurrent deletes...\n");
    test_concurrent_deletes(a, num_threads);

    // Test 5: Mixed random operations
    printf("\n[Test 5] Mixed random operations...\n");
    test_mixed_operations(a, num_threads);

    // Test 6: Stress test with many operations
    printf("\n[Test 6] Stress test with many operations...\n");
    test_stress(a, num_threads);

    printf("\n=== All tests completed successfully! ===\n");

}



int main(int argc, char **argv)
{
    struct args args;
    memset(&args, 0, sizeof(args));

    argp_parse(&argp, argc, argv, 0, 0, &args);

    args.addr = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_port = htons(args.port),
        .sin_addr.s_addr = inet_addr("127.0.0.1")}; /* localhost */

    if (args.op == OP_SET)
        do_set(&args, args.key, args.val, strlen(args.val), NULL, 0);
    else if (args.op == OP_GET)
        do_get(&args, args.key, NULL, NULL, NULL);
    else if (args.op == OP_DELETE)
        do_del(&args, args.key, NULL, 0);
    else if (args.op == OP_QUIT)
        do_quit(&args);
    else if (args.op == OP_TEST)
        do_test(&args);
    else
        printf("See usage: \"dbclient --help\"\n");
}

