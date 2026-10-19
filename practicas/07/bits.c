#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define BIT(n) (UINT32_C(1) << (n))
#define SET(r, n) ((r) |= BIT(n))
#define CLEAR(r, n) ((r) &= ~BIT(n))
#define TOGGLE(r, n) ((r) ^= BIT(n))
#define TEST(r, n) (((r) & BIT(n)) != 0)

static void mostrar(const char *nombre, uint32_t valor)
{
    printf("%s = 0x%08" PRIX32 "  ", nombre, valor);

    for (int i = 31; i >= 0; --i) {
        putchar((valor & BIT(i)) != 0 ? '1' : '0');

        if (i != 0 && i % 8 == 0) {
            putchar('_');
        }
    }
    putchar('\n');
}

static void demostrar_campo(unsigned int pin)
{
    unsigned int registro = pin / 10u;
    unsigned int desplazamiento = (pin % 10u) * 3u;
    uint32_t mascara = UINT32_C(7) << desplazamiento;
    uint32_t inicial = UINT32_MAX;

    uint32_t correcto =
        (inicial & ~mascara) | (UINT32_C(1) << desplazamiento);

    uint32_t solo_or =
        inicial | (UINT32_C(1) << desplazamiento);

    printf("\nGPIO%u: GPFSEL%u, desplazamiento=%u\n",
           pin, registro, desplazamiento);
    mostrar("Mascara", mascara);
    mostrar("Inicial", inicial);
    mostrar("Borrar campo y poner 001", correcto);

    printf("Campo resultante: %" PRIu32 "\n",
           (correcto >> desplazamiento) & UINT32_C(7));

    mostrar("Solo OR, sin borrar", solo_or);
    printf("Campo con solo OR: %" PRIu32 "\n",
           (solo_or >> desplazamiento) & UINT32_C(7));
}

int main(void)
{
    uint32_t r = 0;

    puts("=== Operaciones sobre bits ===");
    mostrar("Inicial", r);

    SET(r, 17);
    mostrar("SET 17", r);

    SET(r, 3);
    mostrar("SET 3", r);

    TOGGLE(r, 17);
    mostrar("TOGGLE 17", r);

    CLEAR(r, 3);
    mostrar("CLEAR 3", r);

    printf("TEST 17: %d\n", TEST(r, 17));

    puts("\n=== Campos de configuracion GPIO ===");
    const unsigned int pines[] = {17, 27, 22, 23, 5, 6};

    for (size_t i = 0; i < sizeof(pines) / sizeof(pines[0]); ++i) {
        unsigned int pin = pines[i];
        unsigned int desplazamiento = (pin % 10u) * 3u;

        printf("GPIO%2u: GPFSEL%u, desplazamiento=%2u, "
               "mascara=0x%08" PRIX32 "\n",
               pin, pin / 10u, desplazamiento,
               UINT32_C(7) << desplazamiento);
    }

    demostrar_campo(17);
    demostrar_campo(22);

    return 0;
}
