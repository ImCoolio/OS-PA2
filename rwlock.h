#ifndef RWLOCK_H
#define RWLOCK_H

#include <pthread.h>
#include <stdint.h>

// Reader-writer lock structure
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t readers;
    pthread_cond_t writers;
    int active_readers;
    int waiting_writers;
    int active_writers;
} rwlock_t;

// Function declarations
void rwlock_init(rwlock_t *lock);
void rwlock_cleanup(rwlock_t *lock);
void rwlock_acquire_readlock(rwlock_t *lock);
void rwlock_release_readlock(rwlock_t *lock);
void rwlock_acquire_writelock(rwlock_t *lock);
void rwlock_release_writelock(rwlock_t *lock);

#endif // RWLOCK_H 