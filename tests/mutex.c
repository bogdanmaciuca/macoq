#include "common.h"
#include "macoq/macoq.h"
#include <unistd.h>

const           int N =  10000; // Number of operations on each thread
const           int T =  3;     // Threads
int             sum;            // Result
macoq_mutex     sem;
pthread_mutex_t pthread_mutex = PTHREAD_MUTEX_INITIALIZER;

void* increment(void* args) {
    for (int i = 0; i < N; i++) {
        macoq_mutex_lock(&sem);
        sum++;
        sleep_us(rand_int(0, 10));
        macoq_mutex_unlock(&sem);
    }
    return NULL;
}

void* increment_unsafe(void* args) {
    for (int i = 0; i < N; i++) {
        sum++;
        sleep_us(rand_int(0, 10));
    }
    return NULL;
}

void* increment_reference(void* args) {
    for (int i = 0; i < N; i++) {
        pthread_mutex_lock(&pthread_mutex);
        sum++;
        sleep_us(rand_int(0, 10));
        pthread_mutex_unlock(&pthread_mutex);
    }
    return NULL;
}

double time_test(void* (f)(void*)) {
    sum = 0;
    Timer timer;
    timer_start(&timer);

    pthread_t threads[T];
    for (int i = 0; i < T; i++)
        pthread_create(&threads[i], NULL, f, NULL);

    for (int i = 0; i < T; i++)
        pthread_join(threads[i], NULL);

    return timer_stop(&timer);
}

int main() {
    double macoq_avg = 0.0;
    double reference_avg = 0.0;
    double unsafe_avg = 0.0;
    int attempts = 64;

    for (int i = 0; i < attempts; i++) {
        macoq_avg += time_test(increment);
        TEST(sum == T * N);
        unsafe_avg += time_test(increment_unsafe);
        reference_avg += time_test(increment_reference);
    }
    macoq_avg /= (double)attempts;
    reference_avg /= (double)attempts;
    unsafe_avg /= (double)attempts;

    // Header
    printf("\n");
    printf("%-20s | %10s\n", "Method", "Time (ms)");
    printf("---------------------+------------\n");

    // Rows
    printf("%-20s | %10.4f\n", "Macoq", macoq_avg);
    printf("%-20s | %10.4f\n", "Unsafe", unsafe_avg);
    printf("%-20s | %10.4f\n", "Reference", reference_avg);
    printf("---------------------+------------\n");

    return 0;
}


