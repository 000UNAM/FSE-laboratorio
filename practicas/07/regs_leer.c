#include <inttypes.h>
#include "gpio_regs.h"

int main(void)
{
    gpio_regs *gpio = gpio_abrir();
    if (gpio == NULL) {
        return 1;
    }

    const unsigned int pines[] = {
        2, 3, 8, 9, 10, 11, 5, 6, 17, 27, 22, 23
    };

    const char *funciones[] = {
        "Entrada", "Salida", "ALT5", "ALT4",
        "ALT0", "ALT1", "ALT2", "ALT3"
    };

    puts("GPIO | Registro | Campo | Funcion | Nivel");

    for (size_t i = 0; i < sizeof(pines) / sizeof(pines[0]); ++i) {
        unsigned int pin = pines[i];
        unsigned int registro = pin / 10u;
        unsigned int desplazamiento = (pin % 10u) * 3u;

        uint32_t campo =
            (gpio->GPFSEL[registro] >> desplazamiento) & UINT32_C(7);

        uint32_t nivel =
            (gpio->GPLEV[pin / 32u] >> (pin % 32u)) & UINT32_C(1);

        printf("%4u | GPFSEL%u  | %5" PRIu32
               " | %-7s | %" PRIu32 "\n",
               pin, registro, campo, funciones[campo], nivel);
    }

    gpio_cerrar(gpio);
    return 0;
}
