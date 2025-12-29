#pragma once
#include <stdatomic.h>
#include <stdbool.h>

typedef struct {
    atomic_int state;
} macoq_mutex;

// Initialize mutex state as UNLOCKED
void macoq_mutex_create(macoq_mutex* mutex);

// Does nothing in release builds but checks if the state is UNLOCKED in debug builds
void macoq_mutex_release(macoq_mutex* mutex);

// Blocks until the mutex is unlocked then locks it
void macoq_mutex_lock(macoq_mutex* mutex);

// Unlocks the mutex
void macoq_mutex_unlock(macoq_mutex* mutex);

// Tries to lock the mutex; returns true if it succeeds and false otherwise
bool macoq_mutex_trylock(macoq_mutex* mutex);

