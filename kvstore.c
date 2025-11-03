#include "kvstore.h"


// Exercise 4: finish implementing kvstore
// TODO: define your synchronization variables here
// (hint: don't forget to initialize them)

// Monitor: mutex to protect key-value store
static pthread_mutex_t kvstore_mutex = PTHREAD_MUTEX_INITIALIZER;



/* read a key from the key-value store.
 *
 * if key doesn't exist, return NULL.
 *
 * NOTE: kv-store must be implemented as a monitor.
 */
char* kv_read(kvstore_t *kv, char *key) {
    /* TODO: your code here */

    // Monitor entry - acquire lock
    pthread_mutex_lock(&kvstore_mutex);

    char *result = NULL;

    // Search for the key
    for (int i = 0; i < TABLE_MAX; i++) {
        if (kv->keys[i].stat == 1 && strcmp(kv->keys[i].key, key) == 0) {
            // Key found
            result = kv->values[i];
            break;
        }
    }

    // Monitor exit - release lock
    pthread_mutex_unlock(&kvstore_mutex);

    return result;
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

    // Monitor entry - acquire lock
    pthread_mutex_lock(&kvstore_mutex);

    int result = 0;
    int key_index = -1;
    int free_index = -1;

    // Search for existing key or find a free slot
    for (int i = 0; i < TABLE_MAX; i++) {
        if (kv->keys[i].stat == 1 && strcmp(kv->keys[i].key, key) == 0) {
            // Key already exists
            key_index = i;
            break;
        }
        if (kv->keys[i].stat == 0 && free_index == -1) {
            // Found a free slot
            free_index = i;
        }
    }

    if (key_index != -1) {
        // Key exists - overwrite the value
        // Free old value
        free(kv->values[key_index]);

        // Allocate and copy new value
        int val_len = strlen(val);
        kv->values[key_index] = (char*) malloc(val_len + 1);
        strcpy(kv->values[key_index], val);

        result = 0; // Success
    } else if (free_index != -1) {
        // Key doesn't exist - insert new key-value pair
        kv->keys[free_index].stat = 1;
        strcpy(kv->keys[free_index].key, key);

        int val_len = strlen(val);
        kv->values[free_index] = (char*) malloc(val_len + 1);
        strcpy(kv->values[free_index], val);

        result = 0; // Success
    } else {
        // No free slot - table is full
        result = 1; // Failure
    }

    // Monitor exit - release lock
    pthread_mutex_unlock(&kvstore_mutex);

    return result;
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

    // Monitor entry - acquire lock
    pthread_mutex_lock(&kvstore_mutex);

    // Search for the key
    for (int i = 0; i < TABLE_MAX; i++) {
        if (kv->keys[i].stat == 1 && strcmp(kv->keys[i].key, key) == 0) {
            // Key found - delete it
            kv->keys[i].stat = 0; // Mark as FREE
            free(kv->values[i]);
            kv->values[i] = NULL;
            break;
        }
    }

    // Monitor exit - release lock
    pthread_mutex_unlock(&kvstore_mutex);
}


// print kv-store's contents to stdout
// note: use any format that you like; this is mostly for debugging purpose
void kv_dump(kvstore_t *kv) {
    /* TODO: your code here */

    // Monitor entry - acquire lock
    pthread_mutex_lock(&kvstore_mutex);

    printf("=== Key-Value Store Dump ===\n");
    int count = 0;
    for (int i = 0; i < TABLE_MAX; i++) {
        if (kv->keys[i].stat == 1) {
            printf("[%d] key=\"%s\" => value=\"%s\"\n",
                   i, kv->keys[i].key, kv->values[i]);
            count++;
        }
    }
    printf("Total entries: %d\n", count);
    printf("============================\n");

    // Monitor exit - release lock
    pthread_mutex_unlock(&kvstore_mutex);
}
