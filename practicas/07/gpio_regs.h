#ifndef GPIO_REGS_H
#define GPIO_REGS_H

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef SIN_VOLATILE
#define VOL
#else
#define VOL volatile
#endif

#define GPIO_MAP_SIZE 4096u

typedef struct {
    VOL uint32_t GPFSEL[6];
    uint32_t reservado0;
    VOL uint32_t GPSET[2];
    uint32_t reservado1;
    VOL uint32_t GPCLR[2];
    uint32_t reservado2;
    VOL uint32_t GPLEV[2];
} gpio_regs;

_Static_assert(offsetof(gpio_regs, GPFSEL) == 0x00,
               "Offset incorrecto de GPFSEL");
_Static_assert(offsetof(gpio_regs, GPSET) == 0x1C,
               "Offset incorrecto de GPSET");
_Static_assert(offsetof(gpio_regs, GPCLR) == 0x28,
               "Offset incorrecto de GPCLR");
_Static_assert(offsetof(gpio_regs, GPLEV) == 0x34,
               "Offset incorrecto de GPLEV");

static inline gpio_regs *gpio_abrir(void)
{
    int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open /dev/gpiomem");
        return NULL;
    }

    void *memoria = mmap(NULL, GPIO_MAP_SIZE,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);

    if (memoria == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return NULL;
    }

    close(fd);
    return (gpio_regs *)memoria;
}

static inline void gpio_cerrar(gpio_regs *gpio)
{
    if (munmap(gpio, GPIO_MAP_SIZE) < 0) {
        perror("munmap");
    }
}

#endif
