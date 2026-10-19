#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int firmado = INT_MAX - 2;

    if (argc > 2) {
        fprintf(stderr, "Uso: %s [entero inicial]\n", argv[0]);
        return 1;
    }

    if (argc == 2) {
        char *fin;
        errno = 0;
        long entrada = strtol(argv[1], &fin, 10);

        if (errno != 0 || fin == argv[1] || *fin != '\0' ||
            entrada < INT_MIN || entrada > INT_MAX) {
            fprintf(stderr, "El valor debe caber en un int.\n");
            return 1;
        }

        firmado = (int)entrada;
    }

    uint32_t sin_signo = (uint32_t)firmado;

    /* Facilita ver la salida en orden junto al diagnóstico. */
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("Inicial: int=%d, uint32_t=%" PRIu32 "\n",
           firmado, sin_signo);

    for (int paso = 1; paso <= 10; ++paso) {
        firmado++; /* Desbordamiento intencional en la prueba. */
        sin_signo++;

        printf("Paso %2d: int=%12d, uint32_t=%" PRIu32 "\n",
               paso, firmado, sin_signo);
    }

    return 0;
}
