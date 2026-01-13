#include <pthread.h>
#include "macoq/macoq.h"
#include "common.h"

#define NUM_THREADS 5

macoq_barrier barrier;

void* thread_func(void* arg) {
    long id = (long)arg;
    printf("%d reached the barrier\n", id);
    macoq_barrier_wait(&barrier);

    printf("%d passed the barrier\n", id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    // Initialize barrier for NUM_THREADS
    macoq_barrier_create(&barrier, NUM_THREADS);

    printf("Starting Barrier Test...\n");

    // Create threads
    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_func, (void*)i);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    if (atomic_load(&barrier.total) == 0) {
        printf("Test Passed: All threads successfully passed the barrier.\n");
    } else {
        printf("Test Failed: Some threads did not pass the barrier.\n");
    }

    return 0;
}
