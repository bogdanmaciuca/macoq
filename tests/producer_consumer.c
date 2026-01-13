#include "common.h"
#include "macoq/macoq.h"
#include <semaphore.h>
#include <unistd.h>

// Configuration
const int PRODUCERS = 2;
const int CONSUMERS = 2;
const int ITEMS_PER_PRODUCER = 20000;
#define BUFFER_SIZE 64

// Shared state
int buffer[BUFFER_SIZE];
int head, tail;
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

macoq_semaphore macoq_empty;
macoq_semaphore macoq_full;

sem_t posix_empty;
sem_t posix_full;

void* producer_macoq(void* arg) {
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        macoq_semaphore_wait(&macoq_empty);
        pthread_mutex_lock(&buffer_mutex);
        buffer[tail] = 1;
        tail = (tail + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&buffer_mutex);
        macoq_semaphore_post(&macoq_full);
    }
    return NULL;
}

void* consumer_macoq(void* arg) {
    int items_to_consume = (PRODUCERS * ITEMS_PER_PRODUCER) / CONSUMERS;
    int local_sum = 0;
    for (int i = 0; i < items_to_consume; i++) {
        macoq_semaphore_wait(&macoq_full);
        pthread_mutex_lock(&buffer_mutex);
        local_sum += buffer[head];
        head = (head + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&buffer_mutex);
        macoq_semaphore_post(&macoq_empty);
    }
    return (void*)(intptr_t)local_sum;
}

void* producer_posix(void* arg) {
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        sem_wait(&posix_empty);
        pthread_mutex_lock(&buffer_mutex);
        buffer[tail] = 1;
        tail = (tail + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&buffer_mutex);
        sem_post(&posix_full);
    }
    return NULL;
}

void* consumer_posix(void* arg) {
    int items_to_consume = (PRODUCERS * ITEMS_PER_PRODUCER) / CONSUMERS;
    int local_sum = 0;
    for (int i = 0; i < items_to_consume; i++) {
        sem_wait(&posix_full);
        pthread_mutex_lock(&buffer_mutex);
        local_sum += buffer[head];
        head = (head + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&buffer_mutex);
        sem_post(&posix_empty);
    }
    return (void*)(intptr_t)local_sum;
}

double run_macoq_once() {
    head = tail = 0;
    // initialize buffer to zeros
    for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = 0;

    macoq_semaphore_create(&macoq_empty, BUFFER_SIZE);
    macoq_semaphore_create(&macoq_full, 0);

    Timer timer;
    timer_start(&timer);

    pthread_t producers[PRODUCERS];
    pthread_t consumers[CONSUMERS];

    for (int i = 0; i < PRODUCERS; i++)
        pthread_create(&producers[i], NULL, producer_macoq, NULL);
    for (int i = 0; i < CONSUMERS; i++)
        pthread_create(&consumers[i], NULL, consumer_macoq, NULL);

    for (int i = 0; i < PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    int total = 0;
    for (int i = 0; i < CONSUMERS; i++) {
        void* res;
        pthread_join(consumers[i], &res);
        total += (int)(intptr_t)res;
    }

    double ms = timer_stop(&timer);

    TEST(total == PRODUCERS * ITEMS_PER_PRODUCER);

    return ms;
}

double run_posix_once() {
    head = tail = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = 0;

    sem_init(&posix_empty, 0, BUFFER_SIZE);
    sem_init(&posix_full, 0, 0);

    Timer timer;
    timer_start(&timer);

    pthread_t producers[PRODUCERS];
    pthread_t consumers[CONSUMERS];

    for (int i = 0; i < PRODUCERS; i++)
        pthread_create(&producers[i], NULL, producer_posix, NULL);
    for (int i = 0; i < CONSUMERS; i++)
        pthread_create(&consumers[i], NULL, consumer_posix, NULL);

    for (int i = 0; i < PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    int total = 0;
    for (int i = 0; i < CONSUMERS; i++) {
        void* res;
        pthread_join(consumers[i], &res);
        total += (int)(intptr_t)res;
    }

    double ms = timer_stop(&timer);

    sem_destroy(&posix_empty);
    sem_destroy(&posix_full);

    TEST(total == PRODUCERS * ITEMS_PER_PRODUCER);

    return ms;
}

int main() {
    const int attempts = 16;
    double macoq_total = 0.0;
    double posix_total = 0.0;

    for (int i = 0; i < attempts; i++) {
        macoq_total += run_macoq_once();
        posix_total += run_posix_once();
    }

    double macoq_avg = macoq_total / attempts;
    double posix_avg = posix_total / attempts;

    printf("\n");
    printf("%-20s | %10s\n", "Method", "Time (ms)");
    printf("---------------------+------------\n");
    printf("%-20s | %10.4f\n", "Macoq semaphores", macoq_avg);
    printf("%-20s | %10.4f\n", "POSIX sem_t", posix_avg);
    printf("---------------------+------------\n");

    return 0;
}
