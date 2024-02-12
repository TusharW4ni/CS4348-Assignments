#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

void performHillisAndSteele(int *input_shmem, int *barrier_shmem, int *intermediate_shmem, int id, int n, int m) {
  int chunk_size = n / m;
  int start = id * chunk_size;
  int end = (id + 1) * chunk_size;
  if (id == m - 1) { end = n; }
  // Hillis and Steele algorithm
  for (int i = 0; i <= log2(n); i++) {
    for (int j = start; j < end; j++) {
      if (j < pow(2, i)) {
        intermediate_shmem[j] = input_shmem[j];
      } else {
        intermediate_shmem[j] = input_shmem[j] + input_shmem[j - (int)pow(2, i)];
      }
    }
    // Synchronize threads here if necessary
    int current_barrier = barrier_shmem[id];
    int done = 0;
    barrier_shmem[id] = barrier_shmem[id] + 1;
    while (1) {
      for (int k = 0; k < m; k++) {
        if (barrier_shmem[k] <= current_barrier) {
          break;
        } else {
          done = 1;
        }
      }
      if (done) break;
    }
    // Copy intermediate results back to input for next iteration
    for (int j = start; j < end; j++) {
      input_shmem[j] = intermediate_shmem[j];
    }
  }
}

int main(int argc, char *argv[]) {
  //--------------------
  int n = atoi(argv[1]); 
  int m = atoi(argv[2]); 
  FILE *A = fopen(argv[3], "r"); 
  FILE *B = fopen(argv[4], "w");
  //--------------------
  int sizeOfInputArray = n*sizeof(int);
  int sizeOfOutputArray = n*sizeof(int);
  int sizeOfBarrier = m*sizeof(int);
  int logValue = log2(n);
  int sizeOfIntermediateArrays = (logValue*(n*sizeof(int)));
  //--------------------
  int sizeOfSharedMem = sizeOfInputArray + sizeOfOutputArray + sizeOfBarrier + sizeOfIntermediateArrays;
  int permissions = PROT_READ | PROT_WRITE;
  int flags = MAP_SHARED | MAP_ANONYMOUS;
  int *shmem = mmap(NULL, sizeOfSharedMem, permissions, flags, -1, 0);
  //--------------------
  for (int i = 0; i < n; i++) { fscanf(A, "%d", &shmem[i]); } fclose(A);
  for (int i = 0; i < m; i++) { shmem[sizeOfInputArray + i] = 0; }
  //--------------------
  for (int i = 0; i < m; i++) {
    if (fork() > 0) {
      continue;
    } else {
      int *input_shmem = shmem;
      int *barrier_shmem = &shmem[sizeOfInputArray];
      int *intermediate_shmem = &shmem[sizeOfInputArray + sizeOfBarrier];
      int id = i;
      performHillisAndSteele(input_shmem, barrier_shmem, intermediate_shmem, id, n, m);
      break;
    }
  }

  // Write output
  for (int i = 0; i < n; i++) { fprintf(B, "%d ", shmem[i]); } fclose(B);

  // Clear shared memory
  munmap(shmem, sizeOfSharedMem);

  return 0;
}