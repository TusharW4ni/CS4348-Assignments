#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

void waitOnBarrier(int *barrierShmem, int *counterShmem, int n) {
  while (1) {
    int ready = 1;
    for (int i = 0; i < n; i++) {
      if (barrierShmem[i] < *counterShmem) {
        ready = 0;
        break;
      }
    }
    if (ready) {
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  int m = atoi(argv[1]);
  int n = atoi(argv[2]);
  FILE *inputFile = fopen(argv[3], "r");
  FILE *outputFile = fopen(argv[4], "w");

  int permissions = PROT_READ | PROT_WRITE;
  int flags = MAP_SHARED | MAP_ANONYMOUS;
  int *inputShmem = mmap(NULL, n*sizeof(int), permissions, flags, -1, 0);
  int *outputShmem = mmap(NULL, n*sizeof(int), permissions, flags, -1, 0);
  int *barrierShmem = mmap(NULL, m*sizeof(int), permissions, flags, -1, 0);
  int *counterShmem = mmap(NULL, sizeof(int), permissions, flags, -1, 0);
  counterShmem = 0;

  for (int i = 0; i < n; i++) {
    fscanf(inputFile, "%d", &inputShmem[i]);
  }
  fclose(inputFile);
  for (int i = 0; i < m; i++) {
    barrierShmem[i] = 0;
  }

  for (int i = 0; i <= floor(log2(n)); i++) {
    for (int j = 0; j < n; j++) {
      if (fork() == 0) {
        if (j < pow(2, i)) {
          outputShmem[j] = inputShmem[j];
        } else {
          outputShmem[j] = inputShmem[j] + inputShmem[j - (int)pow(2, i)];
        }
        barrierShmem[j]++;
        waitOnBarrier(barrierShmem, counterShmem, n);
        exit(0);
      }
    }
    // Wait for all child processes to finish
    for (int j = 0; j < n; j++) {
      wait(NULL);
    }
    // Copy the output back to the input for the next iteration
    for (int j = 0; j < n; j++) {
      inputShmem[j] = outputShmem[j];
    }
  }

  for (int i = 0; i < n; i++) {
    fprintf(outputFile, "%d ", outputShmem[i]);
  }
  fclose(outputFile);

  munmap(inputShmem, n*sizeof(int));
  munmap(outputShmem, n*sizeof(int));
  munmap(barrierShmem, m*sizeof(int));
}