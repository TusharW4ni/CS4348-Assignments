#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

int validateInput(char *argv[], int n, int m, FILE *A) {
  if (n < 1 || m < 1) {
    printf("n and m must be greater than 0\n");
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }
  if (n > 10000 || m > 10000) {
    printf("n and m must be less than 10,000\n");
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }
  if (n < m) {
    printf("n (number of elements) must be greater than or equal to m (number of processes)\n");
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }
  if (A == NULL) {
    printf("Input file does not exist\n");
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }
  int count = 0;
  int temp;
  while (fscanf(A, "%d", &temp) == 1) {
    count++;
  }
  rewind(A);
  if (n > count ) {
    printf("n (number of elements) must be less than or equal to the number of elements in the input file\n");
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }
  return 0;
}

void wait_on_barrier(int *barrier, int index, int m) {
  int currIter = barrier[index];
  int done = 0;
  barrier[index] ++;
  while(1) {
    for(int i = 0; i < m; i++) {
      if(barrier[i] == currIter) {
        break;
      }
      // done = 1;
    }
    // // if (done) {
    //   break;
    // }
  }
}

void prefix_sum(int *barrier_shmem, int *shmem, int n, int m) {
  int chunk_size = n/m;
  printf("Chunk Size: %d\n", chunk_size);

  for(int i = 0; i < (((log(n))/(log(2)))+1); i++) {
    printf("i: %d\n", i);
    if(fork() == 0) {
      printf("---------\n");
      int start = i * chunk_size;
      printf("start: %d\n", start);
      int end = start + chunk_size;
      printf("end: %d\n", end);
      if(i == m-1) end = n;

      for(int j = start; j < end; j++) {
        printf("---\n");
        printf("j: %d\n", j);
        if(j > start) shmem[j] += shmem[j-1];
        printf("shmem[j]: %d\n", shmem[j]);
        printf("---\n");
      }
      //print the array
      for(int j = start; j < end; j++) {
        printf("%d ", shmem[j]);
      }
      printf("\n");
      printf("--------\n"); 

      wait_on_barrier(barrier_shmem, i, m);

      exit(0);
    } else {
      // printf("------\n");
      // printf("Parent Waiting\n");
      // //print the array
      // for(int j = 0; j < n; j++) {
      //   printf("%d ", shmem[j]);
      // }
      // printf("\n");
      // printf("------\n");
      wait(NULL);
    }
  }

  printf("------\n");
  printf("Final Array\n");
  for(int i = 1; i < m; i++) {
    printf("shmem[chunk_size * i]: %d\n", shmem[chunk_size * i]);
    printf("shmem[chunk_size * (i-1)]: %d\n", shmem[chunk_size * (i-1)]);
    shmem[chunk_size * i] += shmem[chunk_size * (i-1)]; 
  }
  //print the array
  for(int j = 0; j < n; j++) {
    printf("%d ", shmem[j]);
  }
  printf("\n");
  printf("------\n");
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }

  int n = atoi(argv[1]);
  int m = atoi(argv[2]);  
  FILE *A = fopen(argv[3], "r");
  FILE *B = fopen(argv[4], "r");
  if (B) {
    fclose(B);
    B = fopen(argv[4], "w");
  } else {
    printf("Output file does not exist\n");
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }
  if (validateInput(argv, n, m, A) == 1) {
    return 1;
  }
  printf("You entered: \nn = %d  \nm = %d  \nInput file = %s \nOutput file = %s\n", n, m, argv[3], argv[4]);

  int barrier[m];
  for (int i = 0; i < m; i++) {
    barrier[i] = 0;
  }

  int inputArr[n];
  for (int i = 0; i < n; i++) {
    fscanf(A, "%d", &inputArr[i]);
  }
  fclose(A);

  int *shmem = mmap(NULL, n*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  int *barrier_shmem = mmap(NULL, m*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shmem == MAP_FAILED || barrier_shmem == MAP_FAILED) {
    printf("mmap failed\n");
    return 1;
  }

  for (int i = 0; i < n; i++) {
    shmem[i] = inputArr[i];
  }

  prefix_sum(barrier_shmem, shmem, n, m);

  for (int i = 0; i < n; i++) {
    fprintf(B, "%d ", shmem[i]);
  }
  fclose(B);

  munmap(shmem, n*sizeof(int));

  return 0;
}