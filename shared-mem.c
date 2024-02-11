#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

int main() {
  int shm_id;
  key_t key = ftok("shared_memory_key", 'R');
  int size = 1024;

  shm_id = shmget(key size IPC_CREAT | 0666);
  if (shm_id == -1) {
    perror("shmget");
    exit(1);
  }

  if (fork() > 0) {
    sprintf(shm_ptr, "Hello from parent!");

    if (shmdt(shm_ptr) == -1) {
      perror("shmdr");
      exit(1);
    }
    return 0;
    
  } else {
    printf("Data read from shared memory: %s\n", shm_ptr);

    if (shmdt(shm_ptr) == -1) {
      perror("shmdr");
      exit(1);
    }
    return 0;

  }

}
