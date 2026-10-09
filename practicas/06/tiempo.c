/*
 * @Autor       Ivan E. Nicolas Montesinos
 * @Fecha       08/10/2026
 * @Descripcion Mide memoria, syscall, pread y SPI opcional.
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#define N 1000000
#define N_SPI 10000
static double ahora_ns(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) == -1) {
        perror("clock_gettime"); return -1;
    }
    return t.tv_sec * 1e9 + t.tv_nsec;
}
int main(void)
{
    static volatile char memoria = 'x';
    char c = 0;
    int fd = open("/dev/p6buf", O_RDONLY);
    double t0, t1, t2, t3;
    if (fd == -1) { perror("open /dev/p6buf"); return 1; }
    t0 = ahora_ns();
    for (int i = 0; i < N; i++) c = memoria;
    t1 = ahora_ns();
    for (int i = 0; i < N; i++) {
        if (syscall(SYS_getppid) == -1) {
            perror("getppid"); close(fd); return 1;
        }
    }
    t2 = ahora_ns();
    for (int i = 0; i < N; i++) {
        ssize_t n = pread(fd, &c, 1, 0);
        if (n != 1) {
            if (n == -1) perror("pread");
            else fprintf(stderr, "pread no devolvio un byte; revise el contenido\n");
            close(fd); return 1;
        }
    }
    t3 = ahora_ns();
    close(fd);
    if (t0 < 0 || t1 < 0 || t2 < 0 || t3 < 0) return 1;
    printf("lectura de memoria de usuario: %.1f ns por operacion\n", (t1-t0)/N);
    printf("syscall getppid:               %.1f ns por operacion\n", (t2-t1)/N);
    printf("pread sobre /dev/p6buf:        %.1f ns por operacion\n", (t3-t2)/N);
    (void)c;
    int fs = open("/dev/spidev0.0", O_RDWR);
    if (fs == -1) {
        perror("open /dev/spidev0.0");
        puts("SPI omitido: dispositivo ausente o sin acceso."); return 0;
    }
    unsigned char tx[3] = {1, 8 << 4, 0}, rx[3] = {0};
    struct spi_ioc_transfer tr = {
        .tx_buf = (uintptr_t)tx, .rx_buf = (uintptr_t)rx,
        .len = 3, .speed_hz = 1000000, .bits_per_word = 8
    };
    double inicio = ahora_ns();
    for (int i = 0; i < N_SPI; i++) {
        int n = ioctl(fs, SPI_IOC_MESSAGE(1), &tr);
        if (n != 3) {
            if (n == -1) perror("ioctl SPI_IOC_MESSAGE");
            else fprintf(stderr, "Transferencia SPI incompleta: %d bytes\n", n);
            puts("SPI sin medicion valida: no se completaron las 10000 transferencias.");
            close(fs); return 0;
        }
    }
    double fin = ahora_ns();
    close(fs);
    if (inicio < 0 || fin < 0) return 1;
    printf("transferencia SPI de 3 bytes:  %.1f ns por operacion\n", (fin-inicio)/N_SPI);
    puts("SPI mide transferencias del bus; no verifica la presencia del MCP3008.");
    return 0;
}
