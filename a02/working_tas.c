#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<stdatomic.h>
#include<stdbool.h>

atomic_bool lock = ATOMIC_VAR_INIT(false); // Atomic boolean variable for the lock
int counter = 0; // Shared counter variable

void* critical_section(void* arg) {
    int thread_id = *(int*)arg;

    // Try to acquire the lock using TAS
    while (atomic_exchange(&lock, true)) {
        // If lock was already true, keep trying until it becomes false
    }

    // Inside the critical section
    printf("Thread %d is inside the critical section.\n", thread_id);

    // Increment the counter for five times
    for (int i = 0; i < 5; ++i) {
        counter++;
        printf("Thread %d incremented counter to: %d\n", thread_id, counter);
    }

    // Exiting the critical section, release the lock
    atomic_store(&lock, false);

    return NULL;
}

int main(int argc, char *argv[]){
  if (argc != 3){
    fprintf(stderr, "Useage: %s <0-TT, 1-TAS, or 2-FAI> <# of threads>\n", argv[0]);
    return 1;
  }

  int algoOpt = atoi(argv[1]);
  int numThreads = atoi(argv[2]);

  pthread_t threads[numThreads];
  int threadIds[numThreads];

  //Create threads
  for (int i = 0; i < numThreads; i++){
    threadIds[i] = i;
    if (pthread_create(& threads[i], NULL, critical_section, &threadIds[i]) != 0) {
      perror("pthread_create");
      return 1;
    }
  }


  //Join threads
  for (int i = 0; i < numThreads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      perror("pthread_join");
      return 1;
    }
  }

  printf("Final counter value: %d\n", counter);

  return 0;
}
