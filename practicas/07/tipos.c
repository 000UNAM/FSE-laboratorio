#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct paquete_normal {
    uint8_t tipo;
    uint32_t valor;
    uint8_t crc;
};

struct __attribute__((packed)) paquete_compacto {
    uint8_t tipo;
    uint32_t valor;
    uint8_t crc;
};

int main(void)
{
    puts("=== Tamaños en bytes ===");
    printf("char:      %zu\n", sizeof(char));
    printf("short:     %zu\n", sizeof(short));
    printf("int:       %zu\n", sizeof(int));
    printf("long:      %zu\n", sizeof(long));
    printf("long long: %zu\n", sizeof(long long));
    printf("void *:    %zu\n", sizeof(void *));
    printf("uint8_t:   %zu\n", sizeof(uint8_t));
    printf("uint16_t:  %zu\n", sizeof(uint16_t));
    printf("uint32_t:  %zu\n", sizeof(uint32_t));
    printf("uint64_t:  %zu\n", sizeof(uint64_t));

    printf("\nINT_MAX:    %d\n", INT_MAX);
    printf("UINT32_MAX: %" PRIu32 "\n", UINT32_MAX);

    uint8_t a = 200;
    uint8_t b = 100;
    int amplia = a + b;
    uint8_t recortada = (uint8_t)(a + b);

    puts("\n=== Promociones y conversiones ===");
    printf("Suma como int: %d\n", amplia);
    printf("Suma convertida a uint8_t: %u\n",
           (unsigned int)recortada);

    uint8_t reg = 0x0F;

    printf("~reg: 0x%X\n", (unsigned int)~reg);
    printf("(uint8_t)~reg: 0x%X\n",
           (unsigned int)(uint8_t)~reg);
    printf("~reg == 0xF0: %d\n", ~reg == 0xF0);
    printf("(uint8_t)~reg == 0xF0: %d\n",
           (uint8_t)~reg == 0xF0);

    puts("\n=== Estructuras ===");
    printf("Normal: tamaño=%zu, valor=%zu, crc=%zu\n",
           sizeof(struct paquete_normal),
           offsetof(struct paquete_normal, valor),
           offsetof(struct paquete_normal, crc));

    printf("Compacta: tamaño=%zu, valor=%zu, crc=%zu\n",
           sizeof(struct paquete_compacto),
           offsetof(struct paquete_compacto, valor),
           offsetof(struct paquete_compacto, crc));

    uint32_t palabra = UINT32_C(0x01020304);
    const unsigned char *bytes = (const unsigned char *)&palabra;

    puts("\n=== Orden de bytes en memoria ===");
    printf("Palabra: 0x%08" PRIX32 "\n", palabra);
    printf("Bytes:");
    for (size_t i = 0; i < sizeof(palabra); ++i) {
        printf(" %02X", (unsigned int)bytes[i]);
    }
    putchar('\n');

    return 0;
}
