/*
 * Peter Desnoyers, 2020
 *
 * 2021 fall: modified by CS5600 staff
 */

#include "lab3.h"
#include "worker.h"
#include "stats.h"
#include "queue.h"
#include "kvstore.h"


// === global variables ===
int listen_sock;
int port = 5600;

queue_t wq;  // the working queue
kvstore_t kv;  // the kv-store

// a trace log for debugging
int debug = 1;  // 0: stop debug tracing; 1: open debug
FILE *trace_fp;
pthread_mutex_t trm = PTHREAD_MUTEX_INITIALIZER;

// if workers will sleep
int if_sleep = 1;

// === helper functions ===

// close the resources when quitting
void cleanup(int sig)
{
    int ret = close(listen_sock);
    if (ret != 0) {
        perror("close socket failure:");
        exit(1);
    }
    if (trace_fp != NULL) {
        fclose(trace_fp);
    }
    exit(0);
}

void init(void) {
    if (signal(SIGINT, cleanup) == SIG_ERR) {
        perror("catch SIGINT:");
    }
    for (int i = 0; i < TABLE_MAX; i++) {
        kv.keys[i].stat = 0; // set to FREE
    }
    if (debug) {
        trace_fp = fopen("./dbserver.trace", "w");
    }
}


void _trace(char *msg) {
    pthread_mutex_lock(&trm);
    fprintf(trace_fp, "%s\n", msg);
    pthread_mutex_unlock(&trm);
}

void trace(char *msg) {
    if (!debug) {return;}
    _trace(msg);
}


// === listener thread and control (main) thread ===

void *listen_thread(void *ignore)
{
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {.sin_family = AF_INET,
                                 .sin_port = htons(port),
                                 .sin_addr.s_addr = 0};
    if (bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("can't bind"), exit(1);
    }
    if (listen(listen_sock, 2) < 0) {
        perror("listen"), exit(1);
    }

    while (1) {
        int fd = accept(listen_sock, NULL, NULL);
        task_t *newtask = (task_t*) malloc(sizeof(task_t));
        newtask->fd = fd;
        enqueue(&wq, newtask);  // this function should be thread-safe
    }
}


int main(int argc, char **argv)
{

    init();

    if (argc == 2) {
        port = atoi(argv[1]); // otherwise, port is 5600
    }

    // number of threads
    int num_thread = 4;
    char *str_nthr = getenv("LAB3_NUM_THREAD");
    if (str_nthr != NULL) {
        num_thread = atoi(str_nthr);
        printf("[INFO] set num_thread = %d\n", num_thread);
    }

    // if workers sleep
    char *str_ifsleep = getenv("LAB3_IF_SLEEP");
    if (str_ifsleep != NULL) {
        if_sleep = atoi(str_ifsleep);
        printf("[INFO] set if_sleep = %d\n", if_sleep);
    }

    // set up the arguments for worker threads
    args_t args;
    args.q = &wq;
    args.kv = &kv;
    (void) args; // cancel compiler warnings

    // Exercise 1: create threads
    //
    //  (1) create one listener thread
    //      -- the linstener thread should run function "listen_thread". (defined above)
    //      -- listener thread does not need argument.
    //      -- we only need 1 listener thread.
    //
    //  (2) create multiple worker threads
    //      -- the worker thread should run function "worker".
    //         (defined in worker.h; implemented in worker.c)
    //      -- each worker thread needs the argument "args" (defined above)
    //         (how to pass arguments to a thread? read pthread tutorials)
    //      -- you should create multiple worker threads
    //         (create "num_thread" of them)
    //
    //  (hint: you only need to write a few lines of code here)

    /* TODO: your code here */

    // Create listener thread
    pthread_t listener;
    pthread_create(&listener, NULL, listen_thread, NULL);

    // Create worker threads
    pthread_t workers[num_thread];
    for (int i = 0; i < num_thread; i++) {
        pthread_create(&workers[i], NULL, worker, &args);
    }




    // main thread: getting cmd from admin
    for (;;) {
        char line[80], cmd[8];
        printf("cs5600> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            exit(0);

        int n = sscanf(line, "%7s", cmd);
        if (n < 1)
            continue;

        if (strcmp(cmd, "quit") == 0) {
            printf("quitting...\n");
            cleanup(0);
        }
        else if (strcmp(cmd, "stats") == 0) {
            printf("queued:   %d\n", queue_count(&wq));
            printf("writes:   %d\n", get_writes());
            printf("reads:    %d\n", get_reads());
            printf("deletes:  %d\n", get_deletes());
            printf("failures: %d\n", get_fails());
        }
        else if (strcmp(cmd, "list") == 0) {
            kv_dump(&kv);
        }
        else if (strcmp(cmd, "help") == 0) {
            printf("stats:  display the status of dbserver\n");
            printf("list:   list all existing kv-pairs in dbserver\n");
            printf("quit:   terminiate dbserver\n");
        }
        else {
            printf("UNKNOWN cmd: <%s>\n", cmd);
        }
    }

    cleanup(0);
}

