/*
 * @Autor       Ivan E. Nicolas Montesinos
 * @Fecha       08/10/2026
 * @Descripcion Ejemplo de comunicacion padre-hijo mediante una tuberia.
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int extremos[2], estado;
    const char mensaje[] = "hola desde espacio de usuario";
    pid_t hijo;
    ssize_t n;
    if (pipe(extremos) == -1) { perror("pipe"); return 1; }
    hijo = fork();
    if (hijo == -1) { perror("fork"); return 1; }
    if (hijo == 0) {
        char recibido[257];
        close(extremos[1]); /* El hijo solamente lee. */
        do { n = read(extremos[0], recibido, sizeof(recibido) - 1); }
        while (n == -1 && errno == EINTR);
        if (n == -1) { perror("read"); return 1; }
        recibido[n] = '\0';
        close(extremos[0]);
        printf("el hijo recibio %zd bytes: %s\n", n, recibido);
        return n == (ssize_t)strlen(mensaje) &&
               strcmp(recibido, mensaje) == 0 ? 0 : 1;
    }
    close(extremos[0]); /* El padre solamente escribe. */
    do { n = write(extremos[1], mensaje, strlen(mensaje)); }
    while (n == -1 && errno == EINTR);
    close(extremos[1]);
    if (n == -1) { perror("write"); return 1; }
    if (n != (ssize_t)strlen(mensaje)) {
        fprintf(stderr, "Escritura incompleta\n"); return 1;
    }
    while (waitpid(hijo, &estado, 0) == -1) {
        if (errno != EINTR) { perror("waitpid"); return 1; }
    }
    return WIFEXITED(estado) && WEXITSTATUS(estado) == 0 ? 0 : 1;
}
