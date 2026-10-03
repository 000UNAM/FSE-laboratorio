#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  setvbuf(stdout, NULL, _IOLBF, 0);

  pid_t hijo = fork();
  if (hijo < 0) {
    perror("fork");
    return EXIT_FAILURE;
  }
  if (hijo == 0) {
    _exit(EXIT_SUCCESS);
  }

  printf("Padre: PID %ld\n", (long)getpid());
  printf("Hijo: PID %ld\n", (long)hijo);
  puts("Padre: duermo 30 s antes de llamar a waitpid()");
  sleep(30);

  int estado;
  pid_t recogido;
  do {
    recogido = waitpid(hijo, &estado, 0);
  } while (recogido < 0 && errno == EINTR);

  if (recogido < 0) {
    perror("waitpid");
    return EXIT_FAILURE;
  }
  if (!WIFEXITED(estado)) {
    fputs("El hijo no termino normalmente\n", stderr);
    return EXIT_FAILURE;
  }

  printf("Padre: waitpid recogio al hijo %ld; salida %d\n",
         (long)recogido, WEXITSTATUS(estado));
  return WEXITSTATUS(estado) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
