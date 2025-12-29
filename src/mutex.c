#include "macoq/macoq.h"
#include <assert.h>
#include <unistd.h>
#include <linux/futex.h>
#include <syscall.h>

#define UNLOCKED  0
#define LOCKED    1 // No waiters
#define CONTENDED 2 // With waiters

// Sleeps until *addr != value
void syscall_wait(atomic_int* addr, int value);
// Wakes up a single thread
void syscall_wake(atomic_int* addr);

void macoq_mutex_create(macoq_mutex* mutex) {
    mutex->state = UNLOCKED;
}

void macoq_mutex_release(macoq_mutex* mutex) {
    assert(mutex->state == UNLOCKED);
}

void macoq_mutex_lock(macoq_mutex* mutex) {
    // If unlocked, lock it and early return
    int expected = UNLOCKED;
    if (atomic_compare_exchange_strong(&mutex->state, &expected, LOCKED)) {
        return;
    }

    // If locked, loop until we can lock it
    while (true) {
        // We don't know if there are any waiters, but it's easier to just assume
        // there are and have a "useless" wake syscall once in a while
        int old_state = atomic_exchange(&mutex->state, CONTENDED);

        // If the mutex was unlocked, we just locked it so we can return
        if (old_state == UNLOCKED) {
            return;
        }

        // If the mutex was locked, we must sleep
        syscall_wait(&mutex->state, CONTENDED);
    }
}

void macoq_mutex_unlock(macoq_mutex* mutex) {
    int old_state = atomic_exchange(&mutex->state, UNLOCKED);

    // If the mutex had waiters, we need to wake them
    if (old_state == CONTENDED)
        syscall_wake(&mutex->state);
}


void syscall_wait(atomic_int* addr, int value) {
    syscall(SYS_futex, addr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, value, NULL, NULL, 0);
}

void syscall_wake(atomic_int* addr) {
    syscall(SYS_futex, addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0);
}
