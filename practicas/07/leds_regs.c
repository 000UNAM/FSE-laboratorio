#include <inttypes.h>
#include <signal.h>
#include "gpio_regs.h"

#define BIT(n) (UINT32_C(1) << (n))

static volatile sig_atomic_t terminar = 0;
static const unsigned int leds[] = {17, 27, 22, 23};

static void detener(int senal)
{
    (void)senal;
    terminar = 1;
}

static void configurar(gpio_regs *gpio, unsigned int pin,
                       uint32_t funcion)
{
    unsigned int registro = pin / 10u;
    unsigned int desplazamiento = (pin % 10u) * 3u;
    uint32_t mascara = UINT32_C(7) << desplazamiento;

    gpio->GPFSEL[registro] =
        (gpio->GPFSEL[registro] & ~mascara) |
        ((funcion & UINT32_C(7)) << desplazamiento);
}

static void mostrar(gpio_regs *gpio, uint32_t mascara)
{
    printf("GPLEV0 (solo LEDs): 0x%08" PRIX32 "\n",
           gpio->GPLEV[0] & mascara);
}

int main(void)
{
    gpio_regs *gpio = gpio_abrir();
    if (gpio == NULL) {
        return 1;
    }

    struct sigaction accion = {0};
    accion.sa_handler = detener;
    sigemptyset(&accion.sa_mask);

    if (sigaction(SIGINT, &accion, NULL) < 0 ||
        sigaction(SIGTERM, &accion, NULL) < 0) {
        perror("sigaction");
        gpio_cerrar(gpio);
        return 1;
    }

    const uint32_t todos = BIT(17) | BIT(27) | BIT(22) | BIT(23);
    const uint32_t pareja = BIT(17) | BIT(22);

    gpio->GPCLR[0] = todos;

    for (size_t i = 0; i < sizeof(leds) / sizeof(leds[0]); ++i) {
        configurar(gpio, leds[i], UINT32_C(1));
    }

    puts("=== Los cuatro LEDs: tres parpadeos ===");
    for (int vuelta = 0; vuelta < 3 && !terminar; ++vuelta) {
        gpio->GPSET[0] = todos;
        usleep(400000);
        mostrar(gpio, todos);

        gpio->GPCLR[0] = todos;
        usleep(400000);
        mostrar(gpio, todos);
    }

    puts("=== Secuencia individual: tres vueltas ===");
    for (int vuelta = 0; vuelta < 3 && !terminar; ++vuelta) {
        for (size_t i = 0;
             i < sizeof(leds) / sizeof(leds[0]) && !terminar;
             ++i) {
            printf("GPIO%u\n", leds[i]);
            gpio->GPSET[0] = BIT(leds[i]);
            usleep(150000);
            mostrar(gpio, todos);
            gpio->GPCLR[0] = BIT(leds[i]);
            usleep(150000);
        }
    }

    puts("=== Solo GPIO17 y GPIO22: tres parpadeos ===");
    for (int vuelta = 0; vuelta < 3 && !terminar; ++vuelta) {
        gpio->GPSET[0] = pareja;
        usleep(400000);
        mostrar(gpio, todos);

        gpio->GPCLR[0] = todos;
        usleep(400000);
    }

    gpio->GPCLR[0] = todos;
    for (size_t i = 0; i < sizeof(leds) / sizeof(leds[0]); ++i) {
        configurar(gpio, leds[i], UINT32_C(0));
    }

    gpio_cerrar(gpio);
    puts("Fin: LEDs apagados y sus pines como entradas.");
    return 0;
}
