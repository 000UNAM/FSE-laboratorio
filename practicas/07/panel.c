#include "gpio_regs.h"
#include <errno.h>
#include <inttypes.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define BIT(n) (UINT32_C(1) << (n))
#define LEDS (BIT(17) | BIT(27) | BIT(22) | BIT(23))

static const unsigned int pines[] = {17, 27, 22, 23};
static gpio_regs *gpio = NULL;
static int spi = -1;
static int i2c = -1;
static volatile sig_atomic_t terminar = 0;

static uint16_t t1;
static int32_t t2, t3;

/* Modifica únicamente los tres bits correspondientes al pin. */
static void configurar(unsigned int pin, uint32_t funcion)
{
    unsigned int indice = pin / 10u;
    unsigned int shift = (pin % 10u) * 3u;
    uint32_t valor = gpio->GPFSEL[indice];

    valor &= ~(UINT32_C(7) << shift);
    valor |= (funcion & UINT32_C(7)) << shift;
    gpio->GPFSEL[indice] = valor;
}

static void limpiar(void)
{
    if (gpio != NULL) {
        gpio->GPCLR[0] = LEDS;

        for (unsigned int n = 0; n < 4; ++n)
            configurar(pines[n], 0);

        gpio_cerrar(gpio);
        gpio = NULL;
    }

    if (spi >= 0)
        close(spi);

    if (i2c >= 0) {
        const uint8_t dormir[2] = {0xF4, 0x24};
        (void)write(i2c, dormir, sizeof dormir);
        close(i2c);
    }
}

static void detener(int signo)
{
    (void)signo;
    terminar = 1;
}

static void fallo(const char *mensaje)
{
    perror(mensaje);
    exit(EXIT_FAILURE);
}

static void comprobar(ssize_t recibido, size_t esperado,
                      const char *mensaje)
{
    if (recibido < 0)
        fallo(mensaje);

    if ((size_t)recibido != esperado) {
        errno = EIO;
        fallo(mensaje);
    }
}

static void leer_registros(uint8_t reg, uint8_t *datos,
                           size_t cantidad)
{
    comprobar(write(i2c, &reg, 1), 1, "Seleccion I2C");
    comprobar(read(i2c, datos, cantidad), cantidad, "Lectura I2C");
}

static uint16_t u16_le(const uint8_t *b)
{
    return (uint16_t)((uint16_t)b[0] |
                      ((uint16_t)b[1] << 8));
}

static int32_t s16_le(const uint8_t *b)
{
    uint16_t valor = u16_le(b);

    return (valor & 0x8000u)
        ? (int32_t)valor - 65536
        : (int32_t)valor;
}

static int32_t temperatura(void)
{
    uint8_t b[3];
    leer_registros(0xFA, b, sizeof b);

    uint32_t cruda = ((uint32_t)b[0] << 12)
                   | ((uint32_t)b[1] << 4)
                   | ((uint32_t)b[2] >> 4);

    int64_t a = (int64_t)(cruda >> 3) - (int64_t)t1 * 2;
    int64_t v1 = (a * t2) >> 11;
    int64_t c = (int64_t)(cruda >> 4) - t1;
    int64_t v2 = (((c * c) >> 12) * t3) >> 14;

    return (int32_t)(((v1 + v2) * 5 + 128) >> 8);
}

static unsigned int potenciometro(void)
{
    uint8_t tx[3] = {0x01, 0x80, 0x00};
    uint8_t rx[3] = {0};
    struct spi_ioc_transfer transferencia = {0};

    transferencia.tx_buf = (uintptr_t)tx;
    transferencia.rx_buf = (uintptr_t)rx;
    transferencia.len = 3;
    transferencia.speed_hz = 1000000;
    transferencia.bits_per_word = 8;

    int resultado = ioctl(spi, SPI_IOC_MESSAGE(1), &transferencia);
    comprobar(resultado, 3, "Transferencia SPI");

    return (((unsigned int)rx[1] & 3u) << 8) | rx[2];
}

static void iniciar_sensores(void)
{
    i2c = open("/dev/i2c-1", O_RDWR);
    if (i2c < 0)
        fallo("Abrir I2C");

    if (ioctl(i2c, I2C_SLAVE, 0x76) < 0)
        fallo("Direccion I2C");

    uint8_t id;
    leer_registros(0xD0, &id, 1);

    if (id != 0x60 && id != 0x58) {
        fprintf(stderr, "ID de sensor inesperado: 0x%02X\n",
                (unsigned int)id);
        exit(EXIT_FAILURE);
    }

    uint8_t calibracion[6];
    leer_registros(0x88, calibracion, sizeof calibracion);
    t1 = u16_le(calibracion);
    t2 = s16_le(calibracion + 2);
    t3 = s16_le(calibracion + 4);

    const uint8_t activar[2] = {0xF4, 0x27};
    comprobar(write(i2c, activar, sizeof activar),
              sizeof activar, "Configurar sensor");

    spi = open("/dev/spidev0.0", O_RDWR);
    if (spi < 0)
        fallo("Abrir SPI");

    uint8_t modo = 0;
    uint8_t bits = 8;
    uint32_t velocidad = 1000000;

    if (ioctl(spi, SPI_IOC_WR_MODE, &modo) < 0 ||
        ioctl(spi, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
        ioctl(spi, SPI_IOC_WR_MAX_SPEED_HZ, &velocidad) < 0)
        fallo("Configurar SPI");

    usleep(50000);
}

/* Una pulsación se acepta tras unos 30 ms de estabilidad.
   Mantener el botón presionado no genera eventos repetidos. */
typedef struct {
    unsigned int candidato;
    unsigned int estable;
    unsigned int repeticiones;
} boton;

static int pulsado(boton *b, unsigned int lectura)
{
    if (lectura != b->candidato) {
        b->candidato = lectura;
        b->repeticiones = 0;
        return 0;
    }

    if (b->repeticiones < 3u)
        ++b->repeticiones;

    if (b->repeticiones == 3u && b->estable != lectura) {
        b->estable = lectura;
        return lectura != 0u;
    }

    return 0;
}

static void mostrar_barra(unsigned int nivel)
{
    uint32_t encendidos = 0;

    for (unsigned int n = 0; n < nivel; ++n)
        encendidos |= BIT(pines[n]);

    gpio->GPCLR[0] = LEDS & ~encendidos;
    gpio->GPSET[0] = encendidos;
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    if (atexit(limpiar) != 0) {
        fputs("No se pudo registrar la limpieza.\n", stderr);
        return EXIT_FAILURE;
    }

    struct sigaction accion = {0};
    accion.sa_handler = detener;
    accion.sa_flags = SA_RESTART;
    sigemptyset(&accion.sa_mask);

    if (sigaction(SIGINT, &accion, NULL) < 0 ||
        sigaction(SIGTERM, &accion, NULL) < 0)
        fallo("Configurar señales");

    iniciar_sensores();

    gpio = gpio_abrir();
    if (gpio == NULL)
        return EXIT_FAILURE;

    gpio->GPCLR[0] = LEDS;
    for (unsigned int n = 0; n < 4; ++n)
        configurar(pines[n], 1);

    configurar(5, 0);
    configurar(6, 0);

    uint32_t inicial = gpio->GPLEV[0];
    boton b5 = {(inicial >> 5) & 1u, (inicial >> 5) & 1u, 0};
    boton b6 = {(inicial >> 6) & 1u, (inicial >> 6) & 1u, 0};

    int congelado = 0;
    unsigned int ultimo_nivel = 5;

    puts("Panel activo.");
    puts("GPIO5: congelar/reactivar. GPIO6: imprimir sensores.");
    puts("Gira el potenciometro. Ctrl+C para terminar.");

    while (!terminar) {
        unsigned int pot = potenciometro();
        uint32_t entradas = gpio->GPLEV[0];

        if (pulsado(&b5, (entradas >> 5) & 1u)) {
            congelado = !congelado;
            puts(congelado ? "Barra CONGELADA." : "Barra ACTIVA.");
        }

        if (!congelado) {
            unsigned int nivel = pot / 205u;
            if (nivel > 4u)
                nivel = 4u;

            if (nivel != ultimo_nivel) {
                mostrar_barra(nivel);
                printf("Pot=%u | LEDs=%u\n", pot, nivel);
                ultimo_nivel = nivel;
            }
        }

        if (pulsado(&b6, (entradas >> 6) & 1u)) {
            int32_t temp = temperatura();
            int64_t magnitud = temp;
            if (magnitud < 0)
                magnitud = -magnitud;

            printf("Lectura: Pot=%u | T=%s%" PRId64
                   ".%02" PRId64 " C | Barra=%s\n",
                   pot, temp < 0 ? "-" : "",
                   magnitud / 100, magnitud % 100,
                   congelado ? "congelada" : "activa");
        }

        usleep(10000);
    }

    puts("Panel terminado. Apagando LEDs y liberando recursos.");
    return EXIT_SUCCESS;
}
