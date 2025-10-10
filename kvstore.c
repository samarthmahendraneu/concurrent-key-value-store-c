#include "kvstore.h"


// Exercise 4: finish implementing kvstore
// TODO: define your synchronization variables here
// (hint: don't forget to initialize them)



/* read a key from the key-value store.
 *
 * if key doesn't exist, return NULL.
 *
 * NOTE: kv-store must be implemented as a monitor.
 */
char* kv_read(kvstore_t *kv, char *key) {
    /* TODO: your code here */

    printf("[INFO] read key[%s]\n", key);
    return NULL;
}


/* write a key-value pair into the kv-store.
 *
 * - if the key exists, overwrite the old value.
 * - if key doesn't exit,
 *     -- insert one if the number of keys is smaller than TABLE_MAX
 *     -- return failure if the number of keys equals TABLE_MAX
 * - return 0 for success; return 1 for failures.
 *
 * notes:
 * - the "val" locates in stack, you must copy the string to
 *   kv-store's own memory. (hint: use malloc)
 * - the "val" is a null-terminated string. Pay attention to how many bytes you
 *   need to allocate. (hint: you need an extra to store '\0').
 * - Read "man strlen" and "man strcpy" to see how they handle string length.
 *
 * NOTE: kv-store must be implemented as a monitor.
 */

int kv_write(kvstore_t *kv, char *key, char *val) {
    /* TODO: your code here */

    printf("[INFO] write key[%s]=val[%s]\n", key, val);
    return 0;
}


/* delete a key-value pair from the kv-store.
 *
 * - if the key exists, delete it.
 * - if the key doesn't exits, do nothing.
 *
 * NOTE: kv-store must be implemented as a monitor.
 */
void kv_delete(kvstore_t *kv, char *key) {
    /* TODO: your code here */

    printf("[INFO] delete key[%s]\n", key);
}


// print kv-store's contents to stdout
// note: use any format that you like; this is mostly for debugging purpose
void kv_dump(kvstore_t *kv) {
    /* TODO: your code here */

    printf("[INFO] dump key-value store\n");
}
