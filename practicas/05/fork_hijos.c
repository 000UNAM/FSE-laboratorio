#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  int creados = 0;
  setvbuf(stdout, NULL, _IOLBF, 0);
  printf("Padre: PID %ld\n", (long)getpid());

  for (int i = 0; i < 3; i++) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      break;
    }
    if (pid == 0) {
      printf("Hijo %d: PID %ld, PPID %ld\n", i + 1,
             (long)getpid(), (long)getppid());
      sleep(20);
      _exit(EXIT_SUCCESS);
    }
    creados++;
  }

  for (int i = 0; i < creados; i++) {
    pid_t terminado;
    do {
      terminado = wait(NULL);
    } while (terminado < 0 && errno == EINTR);
    if (terminado < 0) {
      perror("wait");
      return EXIT_FAILURE;
    }
  }

  printf("Padre: los %d hijos terminaron\n", creados);
  return creados == 3 ? EXIT_SUCCESS : EXIT_FAILURE;
}
