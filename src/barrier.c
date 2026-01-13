#include "macoq/macoq.h"
#include <stdatomic.h>
#include "common.h"

void macoq_barrier_create(macoq_barrier* barrier, int total) {
    macoq_cond_var_create(&barrier->cond_var);
    macoq_mutex_create(&barrier->mutex);
    atomic_store(&barrier->total, total);
}

void macoq_barrier_wait(macoq_barrier* barrier) {
    macoq_mutex_lock(&barrier->mutex);

    // Decrement the count of threads yet to arrive
    while (atomic_load(&barrier->total) > 0) {
        atomic_fetch_add(&barrier->total, -1);
    }

    // Once the last thread arrives, wake all waiting threads
    macoq_cond_var_signal_all(&barrier->cond_var);
    macoq_mutex_unlock(&barrier->mutex);
}
