#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static volatile sig_atomic_t terminar = 0;

static void detener(int senal)
{
    (void)senal;
    terminar = 1;
}

int main(void)
{
    struct sigaction accion = {0};
    accion.sa_handler = detener;
    if (sigemptyset(&accion.sa_mask) == -1 ||
        sigaction(SIGTERM, &accion, NULL) == -1 ||
        sigaction(SIGINT, &accion, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    volatile unsigned long contador = 0;
    while (!terminar)
        ++contador;

    return EXIT_SUCCESS;
}
