#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
  int *counter = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1 , 0);
  for (int i = 0; i < atoi(argv[1]); i++) {
    if (fork() == 0) {
      printf("Child %d\n", *counter);
      (*counter)++;
      exit(0);
    }
  }
  for (int i = 0; i < atoi(argv[1]); i++) {
    wait(NULL);
  }
  munmap(NULL, *counter);
}