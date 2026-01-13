#pragma once
#include <stdatomic.h>
#include <stdbool.h>

// Mutex
typedef struct {
    atomic_int state;
} macoq_mutex;
// Initialize mutex state as UNLOCKED
void macoq_mutex_create(macoq_mutex* mutex);
// Blocks until the mutex is unlocked then locks it
void macoq_mutex_lock(macoq_mutex* mutex);
// Unlocks the mutex
void macoq_mutex_unlock(macoq_mutex* mutex);
// Tries to lock the mutex; returns true if it succeeds and false otherwise
bool macoq_mutex_trylock(macoq_mutex* mutex);

// Conditional variable
typedef struct {
    atomic_int seq;
} macoq_cond_var;
// Initalize conditional variable
void macoq_cond_var_create(macoq_cond_var* cv);
// Signal a single thread
void macoq_cond_var_signal(macoq_cond_var* cv);
// Signal all threads
void macoq_cond_var_signal_all(macoq_cond_var* cv);
// Block until the thread is signaled
void macoq_cond_var_wait(macoq_cond_var* cv, macoq_mutex* mutex);

// Semaphore
typedef struct {
    atomic_int count;
} macoq_semaphore;
// Initialize semaphore
void macoq_semaphore_create(macoq_semaphore* sem, int initial_count);
// If the number of threads which hold the resource is greater than
// initial_count, the thread blocks; otherwise it passes right through
void macoq_semaphore_wait(macoq_semaphore* sem);
// Release the resource
void macoq_semaphore_post(macoq_semaphore* sem);
// Try to acquire resource but don't block if unable
bool macoq_semaphore_trywait(macoq_semaphore* sem);

// RW lock
typedef struct {
    atomic_int state;
    atomic_int writers_waiting;
} macoq_rwlock;
// Initialize rwlock
void macoq_rwlock_create(macoq_rwlock* rwlock);
// Block until no writer holds the lock, then lock for reader
void macoq_rwlock_read_lock(macoq_rwlock* rwlock);
// Block until no writer OR reader holds the lock, then lock for writer
void macoq_rwlock_write_lock(macoq_rwlock* rwlock);
// Unlock readers or writers, if any
void macoq_rwlock_unlock(macoq_rwlock* rwlock);

// Barrier
typedef struct {
    macoq_cond_var cond_var;
    macoq_mutex mutex;
    atomic_int total;
} macoq_barrier;
// Initialize barrier
void macoq_barrier_create(macoq_barrier* barrier, int total);
// Block until total threads have called this function
void macoq_barrier_wait(macoq_barrier* barrier);
