#include <pthread.h>
#include <unistd.h>
#include "macoq/macoq.h"
#include "common.h"

#define NUM_READERS 10
#define NUM_WRITERS 2
#define LOOPS 100

macoq_rwlock rwlock;
int shared_data = 0;

// This variable acts as a "trap" to detect if a reader sneaks in during a write
atomic_int writer_active = 0; 

void* reader_thread(void* arg) {
    long id = (long)arg;
    for (int i = 0; i < LOOPS; i++) {
        macoq_rwlock_read_lock(&rwlock);

        // CRITICAL SECTION (READ)

        TEST(atomic_load(&writer_active) == 1 && "FAILURE: Reader entered while writer was active!\n");

        // 2. Read value
        int val_start = shared_data;

        // 3. Sleep slightly to invite race conditions (allow time for a buggy writer to intrude)
        usleep(100); 

        // 4. Verify value hasn't changed while we held the lock
        int val_end = shared_data;
        TEST(val_start != val_end && "FAILURE: Data changed (%d -> %d) while Reader %ld held lock!\n");

        macoq_rwlock_unlock(&rwlock);
        usleep(50); // Give others a chance
    }
    return NULL;
}

void* writer_thread(void* arg) {
    long id = (long)arg;
    for (int i = 0; i < LOOPS; i++) {
        macoq_rwlock_write_lock(&rwlock);

        // CRITICAL SECTION (WRITE)
        atomic_store(&writer_active, 1); // Set trap

        // Modify data
        int val = shared_data;
        usleep(200); // Hold lock longer to block readers
        shared_data = val + 1;

        atomic_store(&writer_active, 0); // Unset trap

        macoq_rwlock_unlock(&rwlock);

        usleep(1000); // Writers should be slower/less frequent
    }
    return NULL;
}

int main() {
    pthread_t r_threads[NUM_READERS];
    pthread_t w_threads[NUM_WRITERS];

    macoq_rwlock_create(&rwlock);

    printf("Starting RWLock Test with %d Readers and %d Writers...\n", NUM_READERS, NUM_WRITERS);

    // Create Writers
    for (long i = 0; i < NUM_WRITERS; i++) {
        pthread_create(&w_threads[i], NULL, writer_thread, (void*)i);
    }

    // Create Readers
    for (long i = 0; i < NUM_READERS; i++) {
        pthread_create(&r_threads[i], NULL, reader_thread, (void*)i);
    }

    // Join all
    for (int i = 0; i < NUM_READERS; i++) pthread_join(r_threads[i], NULL);
    for (int i = 0; i < NUM_WRITERS; i++) pthread_join(w_threads[i], NULL);

    int expected = NUM_WRITERS * LOOPS;
    if (shared_data == expected) {
        printf("Test Passed: Final shared data value: %d\n", shared_data);
    } else {
        printf("Test Failed: Expected %d, got %d\n", expected, shared_data);
    }

    return 0;
}
