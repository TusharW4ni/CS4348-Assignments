#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

int main(int argc, char *argv[]) {
  int m = atoi(argv[1]);
  int n = atoi(argv[2]);
  FILE *inputFile = fopen(argv[3], "r");
  FILE *outputFile = fopen(argv[4], "w");

  int permissions = PROT_READ | PROT_WRITE;
  int flags = MAP_SHARED | MAP_ANONYMOUS;
  int *inputShmem = mmap(NULL, n*sizeof(int), permissions, flags, -1, 0);
  int *outputShmem = mmap(NULL, n*sizeof(int), permissions, flags, -1, 0);
  int *counterShmem = mmap(NULL, sizeof(int), permissions, flags, -1, 0);
  counterShmem = 0;
  int *tempShmem;

  for (int i = 0; i < n; i++) {
    fscanf(inputFile, "%d", &inputShmem[i]);
  }
  fclose(inputFile);

  for (int i = 0; i <= floor(log2(n)); i++) {
    for (int j = 0; j < n; j++) {
      if (fork() == 0) {
        if (j < pow(2, i)) {
          outputShmem[j] = inputShmem[j];
        } else {
          outputShmem[j] = inputShmem[j] + inputShmem[j - (int)pow(2, i)];
        }
        exit(0);
      }
    }
    for (int j = 0; j < n; j++) {
      wait(NULL);
    }
    tempShmem = inputShmem;
    inputShmem = outputShmem;
    outputShmem = tempShmem;
  }

  for (int i = 0; i < n; i++) {
    fprintf(outputFile, "%d ", inputShmem[i]);
  }
  fclose(outputFile);

  munmap(inputShmem, n*sizeof(int));
  munmap(outputShmem, n*sizeof(int));
}