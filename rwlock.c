#include "rwlock.h"
#include "shared.h"
#include <stdio.h>
#include <time.h>

// Get current timestamp in milliseconds - defined in chash.c

void rwlock_init(rwlock_t *lock) {
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_cond_init(&lock->readers, NULL);
    pthread_cond_init(&lock->writers, NULL);
    lock->active_readers = 0;
    lock->waiting_writers = 0;
    lock->active_writers = 0;
}

void rwlock_cleanup(rwlock_t *lock) {
    pthread_mutex_destroy(&lock->mutex);
    pthread_cond_destroy(&lock->readers);
    pthread_cond_destroy(&lock->writers);
}

void rwlock_acquire_readlock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    while (lock->active_writers > 0 || lock->waiting_writers > 0) {
        fprintf(output_file, "%llu,WAITING ON WRITERS\n", get_timestamp());
        pthread_cond_wait(&lock->readers, &lock->mutex);
    }
    lock->active_readers++;
    fprintf(output_file, "%llu,READ LOCK ACQUIRED\n", get_timestamp());
    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_release_readlock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->active_readers--;
    fprintf(output_file, "%llu,READ LOCK RELEASED\n", get_timestamp());
    if (lock->active_readers == 0 && lock->waiting_writers > 0) {
        pthread_cond_signal(&lock->writers);
    }
    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_acquire_writelock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->waiting_writers++;
    while (lock->active_readers > 0 || lock->active_writers > 0) {
        fprintf(output_file, "%llu,WAITING ON INSERTS\n", get_timestamp());
        pthread_cond_wait(&lock->writers, &lock->mutex);
    }
    lock->waiting_writers--;
    lock->active_writers++;
    fprintf(output_file, "%llu,WRITE LOCK ACQUIRED\n", get_timestamp());
    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_release_writelock(rwlock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->active_writers--;
    fprintf(output_file, "%llu,WRITE LOCK RELEASED\n", get_timestamp());
    if (lock->waiting_writers > 0) {
        fprintf(output_file, "%llu,DELETE AWAKENED\n", get_timestamp());
        pthread_cond_signal(&lock->writers);
    } else {
        pthread_cond_broadcast(&lock->readers);
    }
    pthread_mutex_unlock(&lock->mutex);
} 