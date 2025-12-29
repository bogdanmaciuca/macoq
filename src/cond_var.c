#include "macoq/macoq.h"
#include <stdatomic.h>
#include "common.h"

void macoq_cond_var_create(macoq_cond_var* cv) {
    cv->seq = 0;
}

void macoq_cond_var_signal(macoq_cond_var* cv) {
    atomic_fetch_add(&cv->seq, 1);
    syscall_wake(&cv->seq);
}

void macoq_cond_var_signal_all(macoq_cond_var* cv) {
    atomic_fetch_add(&cv->seq, 1);
    syscall_wake_all(&cv->seq);
}

void macoq_cond_var_wait(macoq_cond_var* cv, macoq_mutex* mutex) {
    int seq = atomic_load(&cv->seq);

    macoq_mutex_unlock(mutex);

    // Only wait if seq hasn't changed
    syscall_wait(&cv->seq, seq);

    macoq_mutex_lock(mutex);
}

