#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_N 10000  // Adjust as needed


/*Shared memory structure
n = # of Elements
m = # of Processors
A - input Array
B - output Array
X - intermediate Array
*/ 

void initBarrier( int* Barrier, int n) {
    for(int i=0; i< n; i++){
        Barrier[i]=-1;
    }  
}

void checkBarrier(int* Barrier, int i, int m)
{
        for(int j=0; j< m; j++)
        {
            if(Barrier[j]<i)
            {
                j=0; //Reset J to scan again;
                wait(NULL);
            }
        }
}

// Worker function
void worker(int begin, int end, int id, int n, int m, int* Barrier, int* X) 
{
    //Kill extra processes if too many forked
    if(id >= m) {
        kill(getpid(), SIGTERM);
        }

    for (int i = 0; i < ceil(log2(n)); i++) 
    {
        int* Old = X;
        X = mmap(NULL, n*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFilePtr , 0);
        //shared->X = malloc(n * sizeof(int));
        for (int j = begin; begin < end; j++) 
        {
            if (j - pow(2, i) < 0) //The case where it does not have a left neighbor
            {
                X[j] = Old[j];
            } else {
                X[j] = Old[j] + Old[j - (int)pow(2, i)];
            }
        }
        //Update Barrier and Check to See other Process Statuses
        Barrier[id]++;
        checkBarrier(Barrier, i, m);
    }
 
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <n> <m> <inputFile> <outputFile>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    char* inputFile = argv[3];
    char* outputFile = argv[4];

    //Used to Identify child processes
    int processId = 0;

    // Validate arguments (additional validation may be required)
    if (n <= 0 || m <= 0) {
        fprintf(stderr, "n or m can't be less than zero\n");
        return 1;
    } else if (m < n) {
      fprintf(stderr, "m can't be less than n\n");
      return 1;
    } 
    // We need to Validate n== # of elements in file

    // Read input array from file
    FILE* inputFilePtr = fopen(inputFile, "r");
    if (inputFilePtr == NULL) {
        perror("Error opening input file");
        return 1;
    }
    //Get File Descriptor from input File
    int inputFD = fileno(inputFilePtr);

    //SharedData* shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

    //Shared memory initialization
    //int* shared_n = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFD , 0);
    //int* shared_m = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFD , 0);
    int* A = mmap(NULL, n*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFD , 0);
    int* B = mmap(NULL, n*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFD , 0);
    int* X = mmap(NULL, n*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFD , 0);
    int* Barrier = mmap(NULL, m*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFD , 0);

    //int* processId = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFilePtr , 0);

    //Initiialize Shared Memory
    //*shared_n = n;
    //*shared_m = m;
    initBarrier(Barrier, n);

    //Read in A.txt to shared A array
    for (int i = 0; i < n; i++) 
    {
        fscanf(inputFilePtr, "%d", A[i]);
    }

    int problemSize = n/m;
    int numChildLoops = ceil(log(m)/log(2));

    for (int i = 0; i < numChildLoops; i++) 
    {
        if (fork() == 0) // Child processes
        {
            /*
            (processId)*problemSize) = beggining of sub problem for process m
            (processId)*problemSize+problemSize = end of sub problem for process m
            */
            if(processId+1==m) //Last Child Process recieves all extra elements in the case of unequal division of n/m
                {
                    worker((processId*problemSize), n, processId++, n, m, Barrier, X);
                }
                else
                {
                    worker((processId*problemSize), ((processId*problemSize) + problemSize), processId++, n, m, Barrier, X);
                }
            exit(0);
        }
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);

    // Update B array
        for (int j = 0; j < n; j++) 
        {
            B[j] = X[j];
        }

    //Write B to B.txt
     FILE* outFile = fopen(outputFile, "w");
        for (int j = 0; j < n; j++) {
            fprintf(outFile, "%d ", B[j]);
        }
        fclose(outFile);


    // Clean up and release resources
    if(munmap(A, n*sizeof(int)) <0)  {
        perror("Error dealloacating shared Memory: A");
        return 1;
    }

    if(munmap(B, n*sizeof(int)) <0) {
        perror("Error dealloacating shared Memory: B");
        return 1;
    }

    if(munmap(X, n*sizeof(int)) <0) {
        perror("Error dealloacating shared Memory: X");
        return 1;
    }

    if(munmap(Barrier, n*sizeof(int)) <0) {
        perror("Error dealloacating shared Memory: Barrier");
        return 1;
    }

    //Close Input File (Must keep open to end so we can mmap using A.txt file descriptor)
    fclose(inputFilePtr);

    return 0;
}

