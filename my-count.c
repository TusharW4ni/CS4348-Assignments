//#define _POSIX_C_SOURCE 200809L
//#define GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
//#include <bits/mman.h>

#define MAX_N 10000  // Adjust as needed


/*Shared memory structure
n = # of Elements
m = # of Processors
A - input Array
B - output Array
X - intermediate Array
*/ 

void initBarrier(int* Barrier, int m) 
{  
    for(int i= 0; i < m; i++)
    {
        Barrier[i]=-1;
    }  
}

void waitBarrier(int* Barrier, int i, int m)
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

// Worker function
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
        //Update Barrier and Check to See other Process Statuses
        Barrier[processId]++;
        waitBarrier(Barrier, i, m);
    }

    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <n> <m> <inputFile> <outputFile>\n", argv[0]);
        return 1;
    }

    // Read in and validate arguments (additional validation may be required)
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

    /*Get File Descriptor from input File
    int inputFD = fileno(inputFilePtr);
    struct stat inputFile;

    if(fstat(inputFD, &inputFile) == -1){
        perror("Could not get file size.\n");
        return 1;
    }
    if(inputFile.st_size != n) {
        perror("Number of elements in file does not equal n")
    }
    */

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
            beggining of sub problem for process m = (i*problemSize)
            end of sub problem for process m       = (i*problemSize) + problemSize
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
    int j = 0;
    for (; j < n; j++) 
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

