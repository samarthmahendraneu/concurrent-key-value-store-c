#include "worker.h"
#include "stats.h"

extern int if_sleep;          // defined in dbserver.c
extern void cleanup(int sig); // defined in dbserver.c
extern void trace(char *msg); // defined in dbserver.c


// respond a failure to dbclient
void resp_fail(int sock, struct request *rq, char *fmt, ...)
{
    inc_fail();  // see stats.c

    va_list ap;
    va_start(ap, fmt);
    if (fmt != NULL)
        vfprintf(stderr, fmt, ap);
    va_end(ap);
    rq->op_status = 'X';
    write(sock, rq, sizeof(*rq));
}

// respond a success to dbclient
void resp_okay(int sock, struct request *rq, int len, void *data)
{
    rq->op_status = 'K';
    sprintf(rq->len, "%d", len);
    write(sock, rq, sizeof(*rq));
    if (len > 0)
        write(sock, data, len);
}


// handle writes
void handle_write(int sock, struct request *rq, kvstore_t *kv)
{
    inc_write(); // see stats.c

    int n, bytes = 0, len = atoi(rq->len);
    char buf[len+1];

    for (void *ptr = buf, *max = ptr+len; ptr < max; ) {
        n = read(sock, buf, len);
        if (n <= 0)
            break;
        ptr += n;
        bytes += n;
    }
    buf[len] = '\0';

    // trace for debug
    char msg[1024];
    sprintf(msg, "W %s = %d,%d", rq->name, bytes,
            (int)crc32(-1, (unsigned char*)buf, bytes));
    trace(msg);

    if (n < 0) {
        resp_fail(sock, rq, "write(%s): receive: %s\n", rq->name, strerror(errno));
    } else if (n == 0) {
        resp_fail(sock, rq, "write(%s, %d): short receive: %d\n", rq->name, len, bytes);
    } else {
        // NOTE: buf is a local variable on stack;
        // you need to store the string in buf to kv-store's memory
        int ret = kv_write(kv, rq->name, buf);
        if (ret == 0) {
            resp_okay(sock, rq, 0, NULL);
        } else {
            resp_fail(sock, rq, "write(%s, %d): kv-store full\n", rq->name, len);
        }
    }
}

void handle_read(int sock, struct request *rq, kvstore_t *kv)
{
    inc_read(); // see stats.c

    char *key = rq->name;
    char *ret = kv_read(kv, key);

    if (ret == NULL) {
        resp_fail(sock, rq, "not found: %s\n", rq->name);
    } else {
        int size = strlen(ret);

        // trace for debug
        char msg[1024];
        sprintf(msg, "R %s = %d,%d", rq->name, size,
                (int)crc32(-1, (unsigned char*)ret, size));
        trace(msg);

        resp_okay(sock, rq, size, ret);
    }
}

void handle_delete(int sock, struct request *rq, kvstore_t *kv)
{
    inc_delete(); // see stats.c

    char *key = rq->name;
    kv_delete(kv, key);

    // trace for debug
    char msg[1024];
    sprintf(msg, "D %s \n", rq->name);
    trace(msg);

    resp_okay(sock, rq, 0, NULL);
}

void dispatch(int sock, kvstore_t *kv)
{
    struct request rq;
    int n = read(sock, &rq, sizeof(rq));
    if (n != sizeof(rq))
        resp_fail(sock, &rq, "short read: %d : %s\n", n, strerror(errno));
    else {
        // random sleep to trigger concurrency bugs
        usleep( (random() * if_sleep)  % 10000);
        if (rq.op_status == 'W')
            handle_write(sock, &rq, kv);
        else if (rq.op_status == 'R')
            handle_read(sock, &rq, kv);
        else if (rq.op_status == 'D')
            handle_delete(sock, &rq, kv);
        else if (rq.op_status == 'Q')
            cleanup(0);
        else
            resp_fail(sock, &rq, "unknown op: %c\n", rq.op_status);
    }
    close(sock);
}

// woker thread function
void *worker(void *args)
{
    queue_t *wq = ((args_t*)args)->q;
    kvstore_t *kv = ((args_t*)args)->kv;
    int first_null = 0;

    for (;;) {
        task_t *w = dequeue(wq);
        if (w != NULL) {
            dispatch(w->fd, kv);
            free(w);
        } else {
            if (first_null == 0) {
                printf("[INFO] worker thread dequeue NULL...\n");
                first_null = 1;
            }
            sleep(1);
        }
    }
}
