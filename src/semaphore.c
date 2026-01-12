#include "macoq/macoq.h"
#include <stdatomic.h>
#include "common.h"

// If sem->count == -1 then the semaphore has waiters

void macoq_semaphore_create(macoq_semaphore* sem, int initial_count) {
    atomic_store(&sem->count, initial_count);
}

void macoq_semaphore_wait(macoq_semaphore* sem) {
    while (true) {
        int count = atomic_load(&sem->count);

        // If there are no open slots AND no waiters
        if (count == 0) {
            // If this is still the case, set to -1 and then wait
            if (atomic_compare_exchange_strong(&sem->count, &count, -1)) {
                syscall_wait(&sem->count, -1);
                continue;
            }
            // If not the case, test the rest of the cases
        }
        // If there are waiters, simply wait
        else if (count == -1) {
            syscall_wait(&sem->count, -1);
            continue;
        }

        // If there are open spots, try and claim one
        // (weak is fine because we're in a loop anyway)
        if (atomic_compare_exchange_weak(&sem->count, &count, count - 1)) {
            return;
        }
    }
}

void macoq_semaphore_post(macoq_semaphore* sem) {
    while (true) {
        int count = atomic_load(&sem->count);
        // If there are no waiters, open a slot up and return
        if (count >= 0) {
            if (atomic_compare_exchange_weak(&sem->count, &count, count + 1))
                return;
        }
        // If there are waiters, set one slot as open and wake them
        else {
            if (atomic_compare_exchange_weak(&sem->count, &count, 1)) {
                syscall_wake_all(&sem->count);
                return;
            }
        }
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

