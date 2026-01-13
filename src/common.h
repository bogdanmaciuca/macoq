#pragma once
#include <stdatomic.h>
#include <unistd.h>
#include <linux/futex.h>
#include <syscall.h>
#include <limits.h>

// Sleeps until `*addr != value`
static inline void syscall_wait(atomic_int* addr, int value) {
    syscall(SYS_futex, addr, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, value, NULL, NULL, 0);
}

// Wakes up a single thread
static inline void syscall_wake(atomic_int* addr) {
    syscall(SYS_futex, addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0);
}

// Wakes up all threads
// Wakes up all threads
static inline void syscall_wake_all(atomic_int* addr) {
    syscall(SYS_futex, addr, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, INT_MAX, NULL, NULL, 0);
}


