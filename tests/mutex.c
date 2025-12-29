#include "common.h"
#include <pthread.h>
#include "macoq/macoq.h"

const int N = 10000;
const int T = 3;
int sum = 0;
macoq_mutex mutex;

void* increment(void* args) {
    for (int i = 0; i < N; i++) {
        macoq_mutex_lock(&mutex);
        sum++;
        macoq_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t threads[T];
    for (int i = 0; i < T; i++)
        pthread_create(&threads[i], NULL, increment, NULL);

    for (int i = 0; i < T; i++)
        pthread_join(threads[i], NULL);

    TEST(sum == T * N);
    return 0;
}


