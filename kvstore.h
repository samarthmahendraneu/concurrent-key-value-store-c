#include "lab3.h"

#ifndef __LAB3_KVSTORE_H__
#define __LAB3_KVSTORE_H__

// a shared key-value store: mapping keys to entries
// NOTE: this is a horribly inefficient implementation...
// Challenge I: want better performance?
// re-implement below with better data structures


typedef struct key_entry {
    int stat; // the status of one key (or entry)
              // FREE = 0, USED = 1
    char key[32];  // key is a string within 31 characters
} key_entry_t;


// an inefficient mapping from keys to values:
//    keys[i] => values[i]  (i < TABLE_MAX)
typedef struct kvstore {
    key_entry_t keys[TABLE_MAX];
    char *values[TABLE_MAX];
} kvstore_t;


//=== key-value store APIs ===

// read a key from the key-value store.
// if key doesn't exist, return NULL.
char* kv_read(kvstore_t *kv, char *key);

// write a key-value pair into the kv-store.
// - if the key exists, overwrite the old value.
// - if key doesn't exit,
//     -- insert one if the number of keys is smaller than TABLE_MAX
//     -- return failure if the number of keys equals TABLE_MAX
// - return 0 for success; return 1 for failures.
int kv_write(kvstore_t *kv, char *key, char *val);


// delete a key-value pair from the kv-store.
// - if the key exists, delete it.
// - if the key doesn't exits, do nothing.
void kv_delete(kvstore_t *kv, char *key);


// print all the kv contents to stdout
void kv_dump(kvstore_t *kv);

#endif
