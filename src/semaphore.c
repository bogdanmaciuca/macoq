#include "macoq/macoq.h"
#include <stdatomic.h>
#include "common.h"

void macoq_semaphore_create(macoq_semaphore* sem, int initial_count) {
    atomic_store(&sem->max_count, initial_count);
    atomic_store(&sem->count, initial_count);
}

void macoq_semaphore_wait(macoq_semaphore* sem) {
    while (true) {
        int count = atomic_load(&sem->count);

        // If there are no open spots, wait for when one opens up and try again
        if (count == 0) {
            syscall_wait(&sem->count, 0);
            continue;
        }

        // If there open spots, try and claim one
        // (weak is fine because we're in a loop anyway)
        if (atomic_compare_exchange_weak(&sem->count, &count, count - 1)) {
            return; // On success, return
            // Otherwise, start over
        }
    }
}

// TODO(maybe): implement optimization so that when there are no waiters no syscall gets used
void macoq_semaphore_post(macoq_semaphore* sem) {
    atomic_fetch_add(&sem->count, 1);

    if (atomic_load(&sem->count) != atomic_load(&sem->max_count)) {
        syscall_wake(&sem->count);
    }
}

bool macoq_semaphore_trywait(macoq_semaphore* sem) {
    int count = atomic_load(&sem->count);
    while (count > 0) {
        if (atomic_compare_exchange_strong(&sem->count, &count, count - 1))
            return true;
    }
    return false;
}

