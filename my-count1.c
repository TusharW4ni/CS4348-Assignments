#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_N 10000  // Adjust as needed

/*
Initializes barrier with all -1

int* Barrier - Barrier to initialize
int m - size of Barrier
*/
void initBarrier(int* Barrier, int m) 
{  
    for(int i= 0; i < m; i++)
    {
        Barrier[i]=-1;
    }  
}

/*
Function that Worker Process Calls to check status of other Worker Processes
Idles until all child processes are finished with the current iteration.

int* Barrier - Barrier to check
int i - current Iteration #
int m - # of Worker Processes
*/
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

/*
Unused function 
Purpose: Validates file has enough integers

FILE* file - file we are validating
int n - # of integers required in file
*/
int intCount(FILE* file, int n)
{

  int wordCount = 0;
  int enoughIntegers = 0; //Assume we have enough integers in the file

  // Read the file character by character.
  char c;
  while ((c = fgetc(file)) != EOF) {
    // If the current character is a space or new line increment the word count.
    if (c == ' ' || c == '\n') {
      wordCount++;
    }
  }

  if(wordCount < n) {
    enoughIntegers = 1;
  }

  return enoughIntegers;
}

/* 
Worker function
Computes a portion of the array's prefix sum based on the child's user defined process Id (0 to m).
After completing current iterations, it checks the shared barrier via checkBarrier() function call.

int processId - user defined processId (A value 0 to m)
int begin - beggining of sub problem
int end - end of sub problem
int m - # of child processes
int n - # of elements in input array
int* Barrier - shared barrier
int* X - input array and all intermediate arrays
*/

void worker(int processId, int begin, int end, int m, int n, int* Barrier, int* X) 
{
    //i+1*n is accessing the new array
    //i*n is accessing the previous array
    for (int i = 0; i < ceil(log2(n)); i++) 
    {   
        int new = (i+1)*n;
        int old = (i*n);
        int shift = (int)pow(2, i);

        for (int j = begin; j < end; j++) 
        {
            if (j < shift) //The case where it does not have a left neighbor
            {
                X[new + j] = X[old +j];
            } 
            else 
            {
                X[new + j] = X[old +j] + X[old + j - shift];
            }
        }
        //Update Barrier and Check other Process Statuses
        Barrier[processId]++;
        checkBarrier(Barrier, i, m);
    }

    exit(0);
}

/*
Main Function
1. Parses and validates arguments from argv[]
2. Allocated Shared Memory X and Barrier
3. Reads in input file to X
4. Forks off m child processes
5. Waits for children processes to finish
6. Writes final array in X to output file
7.Cleans up and deallocates shared memeory X and Barrier
*/

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <n> <m> <inputFile> <outputFile>\n", argv[0]);
        return 1;
    }

    // Read in and validate arguments
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    char* inputFile = argv[3];
    char* outputFile = argv[4];

    if (n <= 0) {
        perror( "n can not be less than 1 \n");
        return 1; 
    }
    if (m <= 0) {
      perror("m can not be less than 1 \n");
      return 1;
    }
    if (n < m) 
    {
      perror("n can not be less than m\n");
      return 1;
    }

    // We need to Validate n == # of elements in file
    
    // Open input File A.txt
    FILE* inputFilePtr = fopen(inputFile, "r");
    if (inputFilePtr == NULL) {
        perror("Error opening input file");
        return 1;
    }
   
    int numIterations = ceil(log2(n));

    /*  Shared Memory Allocation
    X contains all intermediate arrays and original input array allocated together
        Indices Ranges:
        A.txt = [0,n)
        X1 = [n, 2n)
        X1 = [2n, 3n)
        etc...
    */
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
            /*
            beggining of sub problem for process m = (i*problemSize) where i is the processID
            end of sub problem for process m       = (i*problemSize) + problemSize where i is the processID
            */
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
    FILE* outFile = fopen(outputFile, "w");
    
    for (int j = 0; j < n; j++) 
    {
        fprintf(outFile, "%d ", X[(numIterations*n) + j]);
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

    return 0;
}

