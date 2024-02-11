#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h> // For ceil function

#define BARRIER_FLAG 0
#define TRUE 1

// Function to validate n and m
int validate(int n, int m)
{
  if (n <= 0 || m <= 0)
  {
    printf("Error: n and m must be greater than zero.\n");
    return 0;
  }
  if (n <= m)
  {
    printf("Error: n must be greater than m.\n");
    return 0;
  }
  return 1;
}

// Barrier implementation using shared memory
void barrier(int *arr, int n, int m, int process_id)
{
  int i;
  arr[process_id]++;
  while (arr[process_id] != n)
  {
    // Wait until all processes reach the barrier
  }
  if (process_id == 0)
  {
    for (i = 0; i < m; i++)
    {
      arr[i] = BARRIER_FLAG;
    }
  }
  while (arr[process_id] != BARRIER_FLAG)
  {
    // Wait until the barrier is reset
  }
}

int main(int argc, char *argv[])
{
  // Validate command-line arguments
  if (argc != 5)
  {
    printf("Usage: %s <n> <m> <A.txt> <B.txt>\n", argv[0]);
    return 1;
  }

  // Parse n and m from command-line arguments
  int n = atoi(argv[1]);
  int m = atoi(argv[2]);

  // Validate n and m
  if (!validate(n, m))
  {
    return 1;
  }

  // Open A.txt for reading
  FILE *file_A = fopen(argv[3], "r");
  if (file_A == NULL)
  {
    printf("Error: Unable to open %s for reading.\n", argv[3]);
    return 1;
  }

  // Round up the size of the array to the nearest multiple of the number of processes
  // int array_size = (int)ceil((double)n / m) * m;
  int array_size = n;

  // Create/access shared memory for barrier
  key_t barrier_key = ftok("barrier_key", 'R');
  int barrier_shmid = shmget(barrier_key, m * sizeof(int), IPC_CREAT | 0666);
  if (barrier_shmid == -1)
  {
    perror("73 shmget for barrier");
    return 1;
  }
  int *barrier_arr = (int *)shmat(barrier_shmid, NULL, 0);
  if (barrier_arr == (int *)(-1))
  {
    perror("shmat for barrier");
    return 1;
  }

  // Create/access shared memory for array
  key_t array_key = ftok("array_key", 'R');
  int array_shmid = shmget(array_key, array_size * sizeof(int), IPC_CREAT | 0666);
  if (array_shmid == -1)
  {
    perror("86 shmget for array");
    printf("shmget error: %s\n", strerror(errno));
    return 1;
  }
  int *array = (int *)shmat(array_shmid, NULL, 0);
  if (array == (int *)(-1))
  {
    perror("shmat for array");
    return 1;
  }

  // Initialize shared memory for array with zeros
  for (int i = 0; i < array_size; i++)
  {
    array[i] = 0;
  }

  // Read integers from A.txt and store them in the shared array
  int i = 0;
  while (fscanf(file_A, "%d", &array[i]) != EOF && i < n)
  {
    i++;
  }
  fclose(file_A);

  // Fork processes to perform parallel prefix sum
  pid_t pid;
  for (int process_id = 0; process_id < m; process_id++)
  {
    pid = fork();
    if (pid == 0)
    { // Child process
      // Perform parallel prefix sum for this process
      int start = process_id * (n / m);
      int end = (process_id == m - 1) ? n : (process_id + 1) * (n / m);
      for (int step = 1; step < n; step *= 2)
      {
        for (int j = start + step; j < end; j++)
        {
          array[j] += array[j - step];
        }
        barrier(barrier_arr, m, m, process_id);
      }

      // Output the array to B.txt
      FILE *file_B = fopen(argv[4], "w");
      if (file_B == NULL)
      {
        printf("Error: Unable to open %s for writing.\n", argv[4]);
        return 1;
      }
      for (int j = 0; j < n; j++)
      {
        fprintf(file_B, "%d ", array[j]);
      }
      fclose(file_B);

      exit(0);
    }
    else if (pid < 0)
    {
      perror("fork");
      return 1;
    }
  }

  // Wait for all child processes to finish
  for (int i = 0; i < m; i++)
  {
    wait(NULL);
  }

  // Detach and remove shared memory segments
  shmdt(barrier_arr);
  shmdt(array);
  shmctl(barrier_shmid, IPC_RMID, NULL);
  shmctl(array_shmid, IPC_RMID, NULL);

  return 0;
}
