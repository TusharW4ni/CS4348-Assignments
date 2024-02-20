#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_N 10000

int validateInput(char *argv[], int n, int m, FILE *inputFile) 
{
  if (n < 1 || m < 1) {
    printf("n and m must be greater than 0\n");
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }
  // if (n > 10000 || m > 10000) {
  //   printf("n and m must be less than 10,000\n");
  //   printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
  //   return 1;
  // }
  if (n < m) {
    printf("n (number of elements) must be greater than or equal to m (number of processes)\n");
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }
  int count = 0;
  int temp;
  while (fscanf(inputFile, "%d", &temp) == 1) {
    count++;
  }
  rewind(inputFile);
  if (n > count ) {
    printf("n (number of elements) must be less than or equal to the number of elements in the input file\n");
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }
  return 0;
}

void initBarrier(int* Barrier, int m)
{
    for(int i= 0; i < m; i++)
    {
        Barrier[i]=-1;
    }
}

void checkBarrier(int* Barrier, int i, int m)
{
    int incomplete = 1;
    while(incomplete) {
        incomplete = 0;
        for(int j=0; j< m; j++) {
            if(Barrier[j]<i) {
                incomplete = 1;
            }
        }
    }
}

void worker(int processId, int begin, int end, int m, int n, int* Barrier, int* X)
{
    //i+1*n is accessing the new array
    //i*n is accessing the previous array
    for (int i = 0; i < ceil(log2(n)); i++) 
    {   
        for (int j = begin; j < end; j++) 
        {
            if (j < pow(2, i)) //The case where it does not have a left neighbor
            {
                X[((i+1)*n) + j] = X[(i*n)+j];
            } 
            else 
            {
                X[((i+1)*n) + j] = X[(i*n)+j] + X[(i*n) + (j - (int)pow(2, i))];
            }
        }
        //Update Barrier and Check other Process Statuses
        Barrier[processId]++;
        checkBarrier(Barrier, i, m);
    }

    exit(0);
}

int main(int argc, char* argv[]) {
    clock_t start = clock();
    if (argc != 5) 
    {
        printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    FILE* inputFilePtr = fopen(argv[3], "r");
    FILE* outFile = fopen(argv[4], "r");
    if (outFile) {
        fclose(outFile);
        outFile = fopen(argv[4], "w");
    } else {
        printf("Output file does not exist\n");
        printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
        return 1;
    } if (inputFilePtr == NULL) {
        printf("Input file does not exist\n");
        printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
        return 1;
    }

    if (validateInput(argv, n, m, inputFilePtr) != 0) {
        return 1;
    }
    printf("You entered: \nn = %d  \nm = %d  \nInput file = %s \nOutput file = %s\n", n, m, argv[3], argv[4]);

    int numIterations = ceil(log2(n));
    int* X = mmap(NULL, (numIterations+1)*n*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1 , 0);
    int* Barrier = mmap(NULL, m*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS , -1, 0);

    //Read in A.txt to shared X array and Initialize Barrier
    for (int i = 0; i < n; i++) 
    {
        fscanf(inputFilePtr, "%d", &X[i]);
    }
    fclose(inputFilePtr);

    initBarrier(Barrier, m);

    int problemSize = n/m;
    for (int i = 0; i < m; i++) 
    {
        if (fork() == 0) // Child processes
        {
            if(i+1==m) //Last Child Process recieves all extra elements in the case of unequal division of n/m
                {
                    worker(i, (i*problemSize), n , m , n , Barrier, X);
                }
                else
                {
                    worker(i, (i*problemSize), ((i*problemSize) + problemSize), m , n, Barrier, X);
                }
        }
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);

    //Write X to B.txt
    int j = 0;
    for (; j < n; j++) 
    {
        fprintf(outFile, "%d\n", X[(numIterations*n) + j]);
    }
    fclose(outFile);

    // Clean up and release resources
    if(munmap(X, (numIterations+1)*n*sizeof(int)) <0) {
        perror("Error dealloacating shared Memory: X");
        return 1;
    }

    if(munmap(Barrier, m*sizeof(int)) <0) {
        perror("Error dealloacating shared Memory: Barrier");
        return 1;
    }

    printf("Written to %s\n", argv[4]);

    double elapsedTime = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("Elapsed time: %.2f seconds\n", elapsedTime);

    return 0;
}

