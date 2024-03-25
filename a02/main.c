#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>
#include <stdbool.h>

atomic_bool lockB = ATOMIC_VAR_INIT(false);
atomic_int lock = 0;
int counter = 0;
int num_threads = 0;

// typedef struct {
//   int wantToEnter;
//   int turn;
// } PetersonsLock;

// PetersonsLock tree[num_threads];

// void lock(int i) {
//     int depth;
//     for (depth = 0; depth < 5; depth++) {
//         tree[i].wantToEnter = depth;
//         tree[i].turn = i / (1 << (depth + 1));
//         int j;
//         for (j = 0; j < num_threads; j++) {
//             if (j != i) {
//                 while (tree[j].wantToEnter >= depth && tree[i].turn == j / (1 << (depth + 1)));
//             }
//         }
//     }
// }

// void* tt_critical_section(void* arg) {
//   int thread_id = *(int*)arg;

//   lock(thread_id);
//   printf("TT: Thread %d is inside the critical section.\n", thread_id);

//   for (int i = 0; i < 5; i++) {
//     counter++;
//     printf("TT: Thread %d incremented counter to: %d\n", thread_id, counter);
//   }

//   unlock(thread_id);

//   return NULL;
// }

void* tas_critical_section(void* arg) {
  int thread_id = *(int*)arg;

  // Try to acquire the lock using TAS
  while (atomic_exchange(&lockB, true)) {
    // If lock was already true, keep trying until it becomes false
  }

  printf("TAS: Thread %d is inside the critical section.\n", thread_id);

  for (int i = 0; i < 1; i++) {
    counter++;
    printf("TAS: Thread %d incremented counter to: %d\n", thread_id, counter);
  }

  // Exiting the critical section, release the lock
  atomic_store(&lockB, false);

  return NULL;
}

void* fai_critical_section(void* arg) {
  int thread_id = *(int*)arg;

  // Try to acquire the lock using FAI
  while (atomic_fetch_add(&lock, 1)) {
    // If lock was already true, keep trying until it becomes false
    atomic_fetch_sub(&lock, 1);
    continue;
  }

  printf("FAI: Thread %d is inside the critical section.\n", thread_id);

  for (int i = 0; i < 1; i++) {
    counter++;
    printf("FAI: Thread %d incremented counter to: %d\n", thread_id, counter);
  }

  // Exiting the critical section, release the lock
  atomic_fetch_sub(&lock, 1);

  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <0-TT, 1-TAS, or 2-FAI> <number of threads>\n", argv[0]);
    return 1;
  }

  int algorithm_type = atoi(argv[1]);
  num_threads = atoi(argv[2]);

  pthread_t threads[num_threads];
  int thread_ids[num_threads];

  for (int i = 0; i < num_threads; ++i) {
    // tree[i].wantToEnter = -1;
    thread_ids[i] = i;
    switch (algorithm_type) {
      case 0:
        // pthread_create(&threads[i], NULL, tt_critical_section, &thread_ids[i]);
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

  if (algorithm_type == 0) {
    printf("Final counter value (TT): %d\n", counter);
  }

  if (algorithm_type == 1) {
    printf("Final counter value (TAS): %d\n", counter);
  }

  if (algorithm_type == 2) {
    printf("Final counter value (FAI): %d\n", counter);
  }

  return 0;
}