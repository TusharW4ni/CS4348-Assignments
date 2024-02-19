#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

void waitOnBarrier(int *barrierShmem, int index, int n) {
  printf("waiting for barrier; index = %d\n", index);
  int oldValue = barrierShmem[index];
  barrierShmem[index]++;
  while (1) {
    int ready = 1;
    for (int i = 0; i < n; i++) {
      if (barrierShmem[i] < oldValue) {
        ready = 0;
        break;
      }
    }
    if (ready) {
      printf("done waiting for barrier; index = %d\n", index);
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
  int *tempShmem;

  for (int i = 0; i < n; i++) {
    fscanf(inputFile, "%d", &inputShmem[i]);
  }
  printf("\n");
  fclose(inputFile);
  for (int i = 0; i < m; i++) {
    barrierShmem[i] = 0;
  }

  int groupSize = n/m;
  for (int i = 0; i <= floor(log2(n)); i++) {
    for (int j = 0; j < m; j++) {
        // printf("outside forked child");
      if (fork() == 0) {
        int start = j * groupSize;
        int end = (j == (m - 1)) ? n : start + groupSize;
        for (int k = start; k < end; k++) {
          if (j < pow(2, i)) {
            outputShmem[j] = inputShmem[j];
          } else {
            outputShmem[j] = inputShmem[j] + inputShmem[j - (int)pow(2, i)];
          }
          // barrierShmem[j]++;
          waitOnBarrier(barrierShmem, j, n);
          exit(0);
        }
      }
    }
    for (int j = 0; j < n; j++) {
      wait(NULL);
    }
        for (int l = 0; l < n; l++) {
      printf("%d ", outputShmem[l]);
    }
    printf("\n");
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
  munmap(barrierShmem, m*sizeof(int));
}