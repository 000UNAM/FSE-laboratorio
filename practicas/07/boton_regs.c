#include "gpio_regs.h"

#define BOTON (UINT32_C(1) << 5)

int main(void)
{
    gpio_regs *gpio = gpio_abrir();
    if (gpio == NULL) {
        return 1;
    }

    /* GPIO5 como entrada: borrar únicamente su campo. */
    gpio->GPFSEL[0] &= ~(UINT32_C(7) << 15);

    puts("Esperando el boton GPIO5...");
    fflush(stdout);

    while ((gpio->GPLEV[0] & BOTON) == 0) {
        /* Espera intencional sin llamadas dentro del ciclo. */
    }

    puts("Boton detectado.");
    gpio_cerrar(gpio);
    return 0;
}
