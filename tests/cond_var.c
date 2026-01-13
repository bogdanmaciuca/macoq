#include <pthread.h>
#include "macoq/macoq.h"
#include "common.h"

#define MAX_ITEMS 100000

macoq_mutex sem;
macoq_cond_var cond_var;

int buffer = 0;
int done_producing = 0;

void* producer(void* arg) {
    for (int i = 0; i < MAX_ITEMS; ++i) {
        macoq_mutex_lock(&sem);
        while (buffer == 1) {
            macoq_cond_var_wait(&cond_var, &sem);
        }
        buffer = 1;
        macoq_cond_var_signal(&cond_var);
        macoq_mutex_unlock(&sem);
    }

    macoq_mutex_lock(&sem);
    done_producing = 1;
    macoq_cond_var_signal(&cond_var);
    macoq_mutex_unlock(&sem);

    return NULL;
}

void* consumer(void* arg) {
    int consumed_count = 0;
    while (1) {
        macoq_mutex_lock(&sem);
        while (buffer == 0 && !done_producing) {
            macoq_cond_var_wait(&cond_var, &sem);
        }

        if (buffer == 0 && done_producing) {
            macoq_mutex_unlock(&sem);
            break;
        }

        buffer = 0;
        consumed_count++;
        macoq_cond_var_signal(&cond_var);
        macoq_mutex_unlock(&sem);
    }

    TEST(consumed_count == MAX_ITEMS);
    return NULL;
}

int main() {
    pthread_t p_thread, c_thread;

    macoq_mutex_create(&sem);
    macoq_cond_var_create(&cond_var);

    pthread_create(&p_thread, NULL, producer, NULL);
    pthread_create(&c_thread, NULL, consumer, NULL);

    pthread_join(p_thread, NULL);
    pthread_join(c_thread, NULL);

    printf("Test Passed: Produced and Consumed %d items successfully.\n", MAX_ITEMS);
    return 0;
}
