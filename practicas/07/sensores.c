#include <fcntl.h>
#include <inttypes.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static void fallo(const char *mensaje)
{
    perror(mensaje);
    exit(EXIT_FAILURE);
}

static void leer_registros(int fd, uint8_t reg,
                           uint8_t *datos, size_t cantidad)
{
    if (write(fd, &reg, 1) != 1) {
        fallo("Seleccion de registro I2C");
    }
    if (read(fd, datos, cantidad) != (ssize_t)cantidad) {
        fallo("Lectura I2C");
    }
}

static void escribir_registro(int fd, uint8_t reg, uint8_t valor)
{
    uint8_t datos[2] = {reg, valor};
    if (write(fd, datos, sizeof(datos)) != (ssize_t)sizeof(datos)) {
        fallo("Escritura I2C");
    }
}

static uint16_t u16_le(const uint8_t *b)
{
    return (uint16_t)((uint16_t)b[0] | ((uint16_t)b[1] << 8));
}

static int32_t s16_le(const uint8_t *b)
{
    uint16_t valor = u16_le(b);
    return (valor & 0x8000u) != 0
        ? (int32_t)valor - 65536
        : (int32_t)valor;
}

/* Resultado en centesimas de grado, sin usar coma flotante.
   Intermedios de 64 bits para evitar desbordamientos. */
static int32_t compensar(uint32_t cruda, uint16_t t1,
                         int32_t t2, int32_t t3)
{
    int64_t a = (int64_t)(cruda >> 3) - (int64_t)t1 * 2;
    int64_t v1 = (a * t2) >> 11;

    int64_t b = (int64_t)(cruda >> 4) - t1;
    int64_t v2 = (((b * b) >> 12) * t3) >> 14;

    return (int32_t)(((v1 + v2) * 5 + 128) >> 8);
}

static unsigned int leer_pot(int fd, uint8_t rx[3])
{
    uint8_t tx[3] = {1, 0x80, 0};

    struct spi_ioc_transfer transferencia = {0};
    transferencia.tx_buf = (uintptr_t)tx;
    transferencia.rx_buf = (uintptr_t)rx;
    transferencia.len = 3;
    transferencia.speed_hz = 1000000;
    transferencia.bits_per_word = 8;

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &transferencia) != 3) {
        fallo("Transferencia SPI");
    }

    return (((unsigned int)rx[1] & 3u) << 8) | rx[2];
}

int main(int argc, char *argv[])
{
    unsigned int direccion = 0x76;

    if (argc > 2 ||
        (argc == 2 && strcmp(argv[1], "0x76") != 0 &&
                      strcmp(argv[1], "0x77") != 0)) {
        fprintf(stderr, "Uso: %s [0x76|0x77]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2 && strcmp(argv[1], "0x77") == 0) {
        direccion = 0x77;
    }

    int i2c = open("/dev/i2c-1", O_RDWR);
    if (i2c < 0) {
        fallo("Abrir I2C");
    }
    if (ioctl(i2c, I2C_SLAVE, direccion) < 0) {
        fallo("Direccion I2C");
    }

    uint8_t id;
    leer_registros(i2c, 0xD0, &id, 1);
    printf("Direccion: 0x%02X, ID: 0x%02X\n",
           direccion, (unsigned int)id);

    if (id != 0x60 && id != 0x58) {
        fprintf(stderr, "Identificador de sensor no esperado.\n");
        close(i2c);
        return EXIT_FAILURE;
    }

    uint8_t calibracion[6];
    leer_registros(i2c, 0x88, calibracion, sizeof(calibracion));
    uint16_t t1 = u16_le(calibracion);
    int32_t t2 = s16_le(calibracion + 2);
    int32_t t3 = s16_le(calibracion + 4);

    printf("Calibracion: T1=%u, T2=%" PRId32 ", T3=%" PRId32 "\n",
           (unsigned int)t1, t2, t3);
    printf("Bytes T1: %02X %02X\n",
           (unsigned int)calibracion[0],
           (unsigned int)calibracion[1]);

    int spi = open("/dev/spidev0.0", O_RDWR);
    if (spi < 0) {
        fallo("Abrir SPI");
    }

    uint8_t modo = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t velocidad = 1000000;

    if (ioctl(spi, SPI_IOC_WR_MODE, &modo) < 0 ||
        ioctl(spi, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
        ioctl(spi, SPI_IOC_WR_MAX_SPEED_HZ, &velocidad) < 0) {
        fallo("Configurar SPI");
    }

    escribir_registro(i2c, 0xF4, 0x27);
    usleep(50000);

    for (int muestra = 1; muestra <= 5; ++muestra) {
        uint8_t rx[3] = {0};
        unsigned int pot = leer_pot(spi, rx);

        uint8_t bytes[3];
        leer_registros(i2c, 0xFA, bytes, sizeof(bytes));

        uint32_t cruda = ((uint32_t)bytes[0] << 12) |
                         ((uint32_t)bytes[1] << 4) |
                         ((uint32_t)bytes[2] >> 4);

        int32_t temperatura = compensar(cruda, t1, t2, t3);
        int64_t magnitud = temperatura;
        if (magnitud < 0) {
            magnitud = -magnitud;
        }

        printf("\nMuestra %d\n", muestra);
        printf("SPI RX: %02X %02X %02X | Pot=%u | V=%.2f\n",
               (unsigned int)rx[0], (unsigned int)rx[1],
               (unsigned int)rx[2], pot, pot * 3.3 / 1023.0);

        printf("Temp bytes: %02X %02X %02X | Cruda=%" PRIu32
               " | T=%s%" PRId64 ".%02" PRId64 " C\n",
               (unsigned int)bytes[0], (unsigned int)bytes[1],
               (unsigned int)bytes[2], cruda,
               temperatura < 0 ? "-" : "",
               magnitud / 100, magnitud % 100);

        sleep(1);
    }

    escribir_registro(i2c, 0xF4, 0x24);
    close(spi);
    close(i2c);
    return EXIT_SUCCESS;
}
