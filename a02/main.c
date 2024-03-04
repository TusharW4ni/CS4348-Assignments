#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>
#include <stdbool.h>

atomic_bool lock = ATOMIC_VAR_INIT(false);
int counter = 0;

void* tt_critical_section(void* arg) {
    printf("TT: Placeholder for critical section\n");
    return NULL;
}

void* tas_critical_section(void* arg) {
    int thread_id = *(int*)arg;

    // Try to acquire the lock using TAS
    while (atomic_exchange(&lock, true)) {
        // If lock was already true, keep trying until it becomes false
    }

    printf("TAS: Thread %d is inside the critical section.\n", thread_id);

    for (int i = 0; i < 5; ++i) {
        counter++;
        printf("TAS: Thread %d incremented counter to: %d\n", thread_id, counter);
    }

    // Exiting the critical section, release the lock
    atomic_store(&lock, false);

    return NULL;
}

void* fai_critical_section(void* arg) {
    int thread_id = *(int*)arg;

    printf("FAI: Thread %d is inside the critical section.\n", thread_id);

    for (int i = 0; i < 5; ++i) {
        counter = atomic_fetch_add(&counter, 1); // Fetch and increment
        printf("FAI: Thread %d incremented counter to: %d\n", thread_id, counter);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <0-TT, 1-TAS, or 2-FAI> <number of threads>\n", argv[0]);
        return 1;
    }

    int algorithm_type = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        thread_ids[i] = i;
        switch (algorithm_type) {
            case 0:
                pthread_create(&threads[i], NULL, tt_critical_section, &thread_ids[i]);
                break;
            case 1:
                pthread_create(&threads[i], NULL, tas_critical_section, &thread_ids[i]);
                break;
            case 2:
                pthread_create(&threads[i], NULL, fai_critical_section, &thread_ids[i]);
                break;
            default:
                fprintf(stderr, "Invalid algorithm type. Use 0 for TT, 1 for TAS, or 2 for FAI.\n");
                return 1;
        }
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    if (algorithm_type == 1) {
        printf("Final counter value (TAS): %d\n", counter);
    }

    return 0;
}

