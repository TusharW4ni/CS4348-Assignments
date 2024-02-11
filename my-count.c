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


void initBarrier( int* Barrier) 
{
    for(int i=0; i< (sizeof(Barrier)/sizeof(int)); i++)
    {
        Barrier[i]=-1;
    }
    
}

void waitBarrier(int* barrier) {
    barrier->count++;

    if (barrier->count == barrier->n) {
        barrier->count = 0;
    } else {
        while (barrier->count != 0) {
            // Wait
        }
    }
}

// Worker function
void worker(int id, int begin, int end) {
    int n = shared->n;
    int m = shared->m;
    int* A = shared->A;
    int* B = shared->B;
    int* X = shared->X;

    for (int i = 0; i < ceil(log2(n)); i++) 
    {
        int* Old = shared->X;
        shared->X = malloc(n * sizeof(int));
        for (int j = begin; begin < end; j++) {
            if (j - pow(2, i) < 0) //The case where it does not have a left neighbor
            {
                X[j] = Old[j];
            } else {
                X[j] = Old[j] + Old[j - (int)pow(2, i)];
            }
        }

        waitBarrier(barrier);

        waitBarrier(barrier);
    }

    // Worker id 0 writes the result to the output file
    if (id == 0) {
        FILE* outputFile = fopen("B.txt", "w");
        for (int j = 0; j < n; j++) {
            fprintf(outputFile, "%d ", B[j]);
        }
        fclose(outputFile);
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

    //SharedData* shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

    //Shared memory initialization
    int* A = mmap(NULL, n*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFilePtr , 0);
    int* B = mmap(NULL, n*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFilePtr , 0);
    int* X = mmap(NULL, n*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFilePtr , 0);
    int* Barrier = mmap(NULL, m*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFilePtr , 0);
    int* processId = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inputFilePtr , 0);

    //Initiialize Shared Memory
    *processId = 0;
    initBarrier(Barrier);

    //Read in A.txt to shared A array
    for (int i = 0; i < n; i++) 
    {
        fscanf(inputFilePtr, "%d", A[i]);
    }
    fclose(inputFilePtr);

    int problemSize = n/m;
    int numChildLoops = ceil(log(m)/log(2));

    for (int i = 0; i < numChildLoops; i++) 
    {
        if (fork() == 0) // Child processes
        {
            /*
            (*processId)*problemSize) = beggining of sub problem for process m
            (*processId)*problemSize+problemSize = end of sub problem for process m
            */
            if(i+1==numChildLoops) //Last Child Process recieves all extra elements in the case of unequal division of n/m
                {
                    worker(((*processId)*problemSize), n, *processId++);
                }
                else
                {
                    worker(((*processId)*problemSize), ((*processId)*problemSize+problemSize),*processId++);
                }
            exit(0);
        }
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);

    // Update B array
        for (int j = 0; j < n; j++) {
            B[j] = X[j];
        }

    //Write B to B.txt


    // Clean up and release resources
    free(A); //Use munmap instead
    free(B);
    free(X);
    munmap(shared, sizeof(SharedData));

    return 0;
}

