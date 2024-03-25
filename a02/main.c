#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>
#include <stdbool.h>

atomic_bool lockB = ATOMIC_VAR_INIT(false);
atomic_int lockF = 0;
int counter = 0;
int num_threads = 0;
int sleepFor = 0;

// typedef struct {
//     volatile bool flag[2];
//     volatile int victim;
// } PetersonsLock;

// PetersonsLock lockP;

// void peterson_lock(int thread_id) {
//     int other_thread = 1 - thread_id; // get the other thread's ID
//     lockP.flag[thread_id] = true; // show that this thread is interested in entering the critical section
//     lockP.victim = thread_id; // give priority to the other thread
//     while (lockP.flag[other_thread] && lockP.victim == thread_id); // wait until the other thread is not interested or it gives priority to this thread
// }

// void peterson_unlock(int thread_id) {
//     lockP.flag[thread_id] = false; // show that this thread is not interested in entering the critical section
// }

// void* peterson_critical_section(void* arg) {
//   int thread_id = *(int*)arg;

//   peterson_lock(thread_id);
//   printf("Peterson: Thread %d is inside the critical section.\n", thread_id);

//   for (int i = 0; i < 1; i++) {
//     counter++;
//     printf("Peterson: Thread %d incremented counter to: %d\n", thread_id, counter);
//   }

//   peterson_unlock(thread_id);

//   return NULL;
// }

typedef struct {
  volatile bool flag[2];
  volatile int victim;
} PetersonsLock;

typedef struct {
  PetersonsLock* nodes;
  int num_threads;
} TournamentTree;

TournamentTree* tree;

void peterson_lock(PetersonsLock* lock, int thread_id) {
  int other_thread = 1 - thread_id;
  lock->flag[thread_id] = true;
  lock->victim = thread_id;
  while (lock->flag[other_thread] && lock->victim == thread_id);
}

void peterson_unlock(PetersonsLock* lock, int thread_id) {
  lock->flag[thread_id] = false;
}

void tt_lock(int thread_id) {
  for (int i = thread_id; i > 0; i /= 2) {
    peterson_lock(&tree->nodes[i], thread_id % 2);
  }
}

void tt_unlock(int thread_id) {
  for (int i = 1; i < tree->num_threads; i *= 2) {
    peterson_unlock(&tree->nodes[i], thread_id % 2);
  }
}

void* tt_critical_section(void* arg) {
  int thread_id = *(int*)arg;

  tt_lock(thread_id);
  printf("TT: Thread %d is inside the critical section.\n", thread_id);

  for (int i = 0; i < 1; i++) {
    counter++;
    printf("TT: Thread %d incremented counter to: %d\n", thread_id, counter);
  }

  tt_unlock(thread_id);

 return NULL;
}

void* tas_critical_section(void* arg) {
  int thread_id = *(int*)arg;

  // Try to acquire the lock using TAS
  while (atomic_exchange(&lockB, true)) {
    // If lock was already true, keep trying until it becomes false
  }

  printf("TAS: Thread %d is inside the critical section.\n", thread_id);
  sleep(sleepFor);
  for (int i = 0; i < 1; i++) {
    counter++;
    printf("TAS: Thread %d incremented counter to: %d\n", thread_id, counter);
    sleep(sleepFor);
  }

  // Exiting the critical section, release the lock
  atomic_store(&lockB, false);

  return NULL;
}

// void* fai_critical_section(void* arg) {
//   int thread_id = *(int*)arg;

//   while (atomic_fetch_add(&lock, 1)) {
//     atomic_fetch_sub(&lock, 1);
//   }

//   printf("FAI: Thread %d is inside the critical section.\n", thread_id);
//   sleep(sleepFor);
//   for (int i = 0; i < 1; i++) {
//     counter++;
//     printf("FAI: Thread %d incremented counter to: %d\n", thread_id, counter);
//     sleep(sleepFor);
//   }

//   atomic_fetch_sub(&lock, 1);

//   return NULL;
// }

void* fai_critical_section(void* arg) {
  int thread_id = *(int*)arg;

  while (atomic_fetch_add(&lockF, 1)) {
    atomic_fetch_sub(&lockF, 1);
  }

  printf("FAI: Thread %d is inside the critical section.\n", thread_id);
  sleep(sleepFor);
  for (int i = 0; i < 1; i++) {
    counter++;
    printf("FAI: Thread %d incremented counter to: %d\n", thread_id, counter);
    sleep(sleepFor);
  }

  atomic_fetch_sub(&lockF, 1);

  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <0-TT, 1-TAS, or 2-FAI> <number of threads>\n", argv[0]);
    return 1;
  }

  int algorithm_type = atoi(argv[1]);
  num_threads = atoi(argv[2]);

  tree = malloc(sizeof(TournamentTree));
  tree->nodes = malloc(sizeof(PetersonsLock) * num_threads);
  tree->num_threads = num_threads;

  pthread_t threads[num_threads];
  int thread_ids[num_threads];

  // for (int i = 0; i < num_threads; i++) {
  //   thread_ids[i] = i;
  // }

  for (int i = 0; i < num_threads; ++i) {
    // tree[i].wantToEnter = -1;
    thread_ids[i] = i;
    switch (algorithm_type) {
      case 0:
        pthread_create(&threads[i], NULL, tt_critical_section, &thread_ids[i]);
        // pthread_create(&threads[i], NULL, peterson_critical_section, &thread_ids[i]);
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
  } else if (algorithm_type == 1) {
    printf("Final counter value (TAS): %d\n", counter);
  } else if (algorithm_type == 2) {
    printf("Final counter value (FAI): %d\n", counter);
  } else {
    fprintf(stderr, "Invalid algorithm type. Use 0 for TT, 1 for TAS, or 2 for FAI.\n");
    return 1;
  }

  free(tree->nodes);
  free(tree);

  return 0;
}