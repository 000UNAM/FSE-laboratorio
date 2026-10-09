/*
 * @Autor       Ivan E. Nicolas Montesinos
 * @Fecha       08/10/2026
 * @Descripcion Cliente de usuario para el driver p6buf.
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    const char *msg = argc > 1 ? argv[1] : "hola desde espacio de usuario";
    char recibido[257]; /* 256 bytes del driver mas el terminador. */
    size_t largo = strlen(msg);
    ssize_t n;
    int fd = open("/dev/p6buf", O_RDWR);

    if (fd < 0) {
        perror("open /dev/p6buf");
        return 1;
    }
    n = write(fd, msg, largo);
    if (n < 0) {
        perror("write");
        close(fd);
        return 1;
    }
    if ((size_t)n != largo) {
        fprintf(stderr, "Escritura incompleta\n");
        close(fd);
        return 1;
    }
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        close(fd);
        return 1;
    }
    n = read(fd, recibido, sizeof(recibido) - 1);
    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }
    recibido[n] = '\0';
    printf("el driver devolvio %zd bytes: %s\n", n, recibido);
    if (close(fd) < 0) {
        perror("close");
        return 1;
    }
    return 0;
}
