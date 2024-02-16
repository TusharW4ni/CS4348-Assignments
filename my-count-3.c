#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int id = fork();
  int n = 0;
  if (id != 0) {
    n = 5;
    for (int i = 0; i < n; i++) {
      printf("%d. hello from parent\n", i);
    }
  } else {
    n = 10;
    for (int i = 0; i < n; i++) {
      printf("%d. hello from child\n", i);
    }
  }
  if (id != 0) wait(NULL);
}