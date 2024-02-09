#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_N 10000  // Adjust as needed

// Shared memory structure
typedef struct {
    int n;
    int m;
    int* A;
    int* B;
    int* X;
} SharedData;

// Barrier implementation
typedef struct {
    int count;
    int n;
} Barrier;

void initBarrier(Barrier* barrier, int n) {
    barrier->count = 0;
    barrier->n = n;
}

void waitBarrier(Barrier* barrier) {
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
void worker(int id, SharedData* shared, Barrier* barrier) {
    int n = shared->n;
    int m = shared->m;
    int* A = shared->A;
    int* B = shared->B;
    int* X = shared->X;

    for (int i = 0; i < log2(n); i++) {
        for (int j = 0; j < n; j++) {
            if (j < pow(2, i)) {
                X[j] = B[j];
            } else {
                X[j] = B[j] + B[j - (int)pow(2, i)];
            }
        }

        waitBarrier(barrier);

        // Update B array
        for (int j = 0; j < n; j++) {
            B[j] = X[j];
        }

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

   

    //Shared memory initialization
    //SharedData* shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    int fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd, sizeof(SharedData));
    SharedData* shared = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    shared->n = n;
    shared->m = m;
    shared->A = malloc(n * sizeof(int));
    shared->B = malloc(n * sizeof(int));
    shared->X = malloc(n * sizeof(int));

    // Read input array from file
    FILE* inputFilePtr = fopen(inputFile, "r");
    if (inputFilePtr == NULL) {
        perror("Error opening input file");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        fscanf(inputFilePtr, "%d", &shared->A[i]);
    }

    fclose(inputFilePtr);

    // Create worker processes
    pid_t pid;
    Barrier barrier;
    initBarrier(&barrier, m);

    for (int i = 0; i < m; i++) {
        pid = fork();
        if (pid == 0) {
            // Child process
            worker(i, shared, &barrier);
            exit(0);
        }
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);

    // Clean up and release resources
    free(shared->A);
    free(shared->B);
    free(shared->X);
    munmap(shared, sizeof(SharedData));

    return 0;
}
