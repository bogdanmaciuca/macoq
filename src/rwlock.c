#include "macoq/macoq.h"
#include <stdatomic.h>
#include "common.h"

// state = 0  -> unlocked
// state = -1 -> locked by a writer
// state > 0  -> locked by readers (e.g. state = 3 -> 3 readers)
// writers_waiting = number of writers waiting to acquire the lock

void macoq_rwlock_create(macoq_rwlock* rwlock) {
    atomic_store(&rwlock->state, 0);
    atomic_store(&rwlock->writers_waiting, 0);
}

void macoq_rwlock_read_lock(macoq_rwlock* rwlock) {
    while (true) {
        int state = atomic_load(&rwlock->state);

        // If a writer holds the lock or writers are waiting, wait and then try again
        if (state < 0 || atomic_load(&rwlock->writers_waiting) > 0) {
            syscall_wait(&rwlock->state, state);
            continue;
        }

        // If there is no writer, attempt to increment reader
        if (atomic_compare_exchange_weak(&rwlock->state, &state, state + 1))
            return;
        // If it fails, try again from the start
    }
}

void macoq_rwlock_write_lock(macoq_rwlock* rwlock) {
    atomic_fetch_add(&rwlock->writers_waiting, 1);
    
    while (true) {
        int state = atomic_load(&rwlock->state);

        // If anyone holds the lock, wait and then try again
        if (state != 0) {
            syscall_wait(&rwlock->state, state);
            continue;
        }

        // If the rwlock is unlocked, try and lock it as the writer
        int expected = 0;
        if (atomic_compare_exchange_weak(&rwlock->state, &expected, -1)) {
            atomic_fetch_sub(&rwlock->writers_waiting, 1);
            return;
        }
        // If not, try again from the start
    }
}

void macoq_rwlock_unlock(macoq_rwlock* rwlock) {
    int state = atomic_load(&rwlock->state);

    if (state == -1) {
        // Writer -> notify all readers and writers waiting
        atomic_store(&rwlock->state, 0);
        syscall_wake_all(&rwlock->state);
    }
    else if (state > 0) {
        // Reader
        while (true) {
            if (atomic_compare_exchange_weak(&rwlock->state, &state, state - 1)) {
                // If we were the last reader, notify all writers
                // (the remaining threads that are waiting)
                if (state == 1)
                    syscall_wake_all(&rwlock->state);
                return;
            }
        }
    }
}
