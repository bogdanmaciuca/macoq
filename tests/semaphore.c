#include <pthread.h>
#include <unistd.h>
#include "macoq/macoq.h"
#include "common.h"

#define NUM_ITEMS 1000
#define BUFFER_SIZE 5

macoq_semaphore empty_slots;
macoq_semaphore full_slots;
int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;

// Test 1: Producer-Consumer
void* producer(void* arg) {
    for (int i = 0; i < NUM_ITEMS; i++) {
        // Wait for space in buffer
        macoq_semaphore_wait(&empty_slots);

        buffer[in] = i;
        in = (in + 1) % BUFFER_SIZE;

        // Signal that new data is available
        macoq_semaphore_post(&full_slots);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0; i < NUM_ITEMS; i++) {
        // Wait for data to be available
        macoq_semaphore_wait(&full_slots);

        int item = buffer[out];
        TEST(item == i); // Verify order integrity
        out = (out + 1) % BUFFER_SIZE;

        // Signal that a slot is freed
        macoq_semaphore_post(&empty_slots);
    }
    return NULL;
}

void test_producer_consumer() {
    printf("Running Test 1: Producer-Consumer (Blocking logic)...\n");

    pthread_t prod, cons;

    // Buffer has 'BUFFER_SIZE' empty slots initially
    macoq_semaphore_create(&empty_slots, BUFFER_SIZE); 
    // Buffer has 0 full slots initially
    macoq_semaphore_create(&full_slots, 0);

    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    printf("  [PASS] Producer-Consumer successful.\n");
}

// Test 2: TryWait
void test_trywait() {
    printf("Running Test 2: TryWait (Non-blocking logic)...\n");

    macoq_semaphore sem;
    macoq_semaphore_create(&sem, 1); // Start with 1 resource

    // 1. Should succeed (1 -> 0)
    TEST(macoq_semaphore_trywait(&sem) == true);

    // 2. Should fail (0 -> 0) - must NOT block
    TEST(macoq_semaphore_trywait(&sem) == false);

    // 3. Post (0 -> 1)
    macoq_semaphore_post(&sem);

    // 4. Should succeed again (1 -> 0)
    TEST(macoq_semaphore_trywait(&sem) == true);

    printf("  [PASS] TryWait logic successful.\n");
}

int main() {
    test_producer_consumer();
    test_trywait();
    printf("\nAll Semaphore tests passed!\n");
    return 0;
}
