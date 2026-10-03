#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ITERACIONES 1000000L

static long contador = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void *incrementar(void *argumento) {
  (void)argumento;
  for (long i = 0; i < ITERACIONES; i++) {
    pthread_mutex_lock(&mutex);
    contador++;
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

int main(void) {
  pthread_t hilos[2];

  for (int i = 0; i < 2; i++) {
    int error = pthread_create(&hilos[i], NULL, incrementar, NULL);
    if (error != 0) {
      fprintf(stderr, "pthread_create: %s\n", strerror(error));
      return EXIT_FAILURE;
    }
  }
  for (int i = 0; i < 2; i++) {
    int error = pthread_join(hilos[i], NULL);
    if (error != 0) {
      fprintf(stderr, "pthread_join: %s\n", strerror(error));
      return EXIT_FAILURE;
    }
  }

  int error = pthread_mutex_destroy(&mutex);
  if (error != 0) {
    fprintf(stderr, "pthread_mutex_destroy: %s\n", strerror(error));
    return EXIT_FAILURE;
  }

  printf("%ld\n", contador);
  return EXIT_SUCCESS;
}
