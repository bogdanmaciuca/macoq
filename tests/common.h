#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#define TEST(condition)                                                                                    \
    if (!(bool)(condition)) {                                                                              \
        printf("\n=== TEST FAILED ===\nIn %s:%d: %s()\n%s\n\n", __FILE__, __LINE__, __func__, #condition); \
        exit(1);                                                                                           \
    }

typedef struct {
    struct timespec start_time;
} Timer;

void timer_start(Timer* t) {
    clock_gettime(CLOCK_MONOTONIC, &t->start_time);
}

double timer_stop(Timer* t) {
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    long seconds = end_time.tv_sec - t->start_time.tv_sec;
    long nanoseconds = end_time.tv_nsec - t->start_time.tv_nsec;

    if (nanoseconds < 0) {
        seconds -= 1;
        nanoseconds += 1000000000;
    }

    return (seconds * 1000.0) + (nanoseconds / 1e6);
}

void sleep_us(long us) {
    #ifdef ENABLE_SLEEP
    struct timespec req;

    req.tv_sec = us / 1000000;

    req.tv_nsec = (us % 1000000) * 1000;

    nanosleep(&req, NULL);
#endif
}

static unsigned int seed = 1337;
int rand_int(int min, int max) {
    int r = rand_r(&seed); 

    return min + (r % (max - min + 1));
}

