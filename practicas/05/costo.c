#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define N 2000

static void tomar_tiempo(struct timespec *t)
{
    if (clock_gettime(CLOCK_MONOTONIC, t) == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
}

static double milisegundos(struct timespec inicio, struct timespec fin)
{
    return (double)(fin.tv_sec - inicio.tv_sec) * 1000.0
         + (double)(fin.tv_nsec - inicio.tv_nsec) / 1000000.0;
}

static void comprobar_hilo(int error, const char *operacion)
{
    if (error != 0) {
        fprintf(stderr, "%s: %s\n", operacion, strerror(error));
        exit(EXIT_FAILURE);
    }
}

static void *hilo_vacio(void *argumento)
{
    (void)argumento;
    return NULL;
}

int main(void)
{
    struct timespec inicio, fin;
    tomar_tiempo(&inicio);
    for (int i = 0; i < N; ++i) {
        pid_t hijo = fork();
        if (hijo == -1) {
            perror("fork");
            return EXIT_FAILURE;
        }
        if (hijo == 0)
            _exit(EXIT_SUCCESS);

        int estado;
        pid_t recogido;
        do {
            recogido = waitpid(hijo, &estado, 0);
        } while (recogido == -1 && errno == EINTR);
        if (recogido == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }
        if (!WIFEXITED(estado) || WEXITSTATUS(estado) != EXIT_SUCCESS) {
            fprintf(stderr, "El hijo no termino correctamente.\n");
            return EXIT_FAILURE;
        }
    }
    tomar_tiempo(&fin);
    double procesos_ms = milisegundos(inicio, fin);

    tomar_tiempo(&inicio);
    for (int i = 0; i < N; ++i) {
        pthread_t hilo;
        comprobar_hilo(pthread_create(&hilo, NULL, hilo_vacio, NULL),
                       "pthread_create");
        comprobar_hilo(pthread_join(hilo, NULL), "pthread_join");
    }
    tomar_tiempo(&fin);
    double hilos_ms = milisegundos(inicio, fin);

    if (procesos_ms <= 0.0 || hilos_ms <= 0.0) {
        fprintf(stderr, "Tiempo no positivo; no se puede calcular la relacion.\n");
        return EXIT_FAILURE;
    }
    printf("%.6f %.6f %.6f\n", procesos_ms, hilos_ms,
           procesos_ms / hilos_ms);
    return EXIT_SUCCESS;
}
