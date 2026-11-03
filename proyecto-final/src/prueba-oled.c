/*
 * Prueba de OLED SSD1306 128x64 por I2C.
 * Elaborado con asistencia de IA; pendiente de validacion fisica.
 */
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

enum {
  OLED_ADDR = 0x3C,
  WIDTH = 128,
  PAGES = 8,
  CHUNK = 16,
  COMMAND = 0x00,
  DATA = 0x40
};

/* Cada write corresponde a una transaccion I2C completa. */
static int enviar(int fd, const uint8_t *datos, size_t cantidad)
{
  ssize_t n = write(fd, datos, cantidad);
  if (n < 0) {
    perror("Error al escribir en OLED");
    return -1;
  }
  if ((size_t)n != cantidad) {
    fprintf(stderr, "Transferencia incompleta: %zd de %zu bytes\n",
            n, cantidad);
    return -1;
  }
  return 0;
}

int main(void)
{
  int resultado = EXIT_FAILURE;
  int fd = open("/dev/i2c-1", O_RDWR);
  if (fd < 0) {
    perror("No se pudo abrir /dev/i2c-1");
    return EXIT_FAILURE;
  }

  if (ioctl(fd, I2C_SLAVE, OLED_ADDR) < 0) {
    perror("No se pudo seleccionar la OLED");
    goto terminar;
  }

  const uint8_t inicio[] = {
    COMMAND,
    0xAE,             /* Pantalla apagada mientras se configura. */
    0xD5, 0x80,       /* Reloj interno. */
    0xA8, 0x3F,       /* 64 filas. */
    0xD3, 0x00,       /* Sin desplazamiento vertical. */
    0x40,             /* Primera linea de inicio. */
    0x8D, 0x14,       /* Bomba de carga interna habilitada. */
    0x20, 0x00,       /* Direccionamiento horizontal. */
    0xA1, 0xC8,       /* Orientacion de columnas y filas. */
    0xDA, 0x12,       /* Configuracion COM para 128x64. */
    0x81, 0x40,       /* Contraste moderado. */
    0xD9, 0xF1,       /* Precarga. */
    0xDB, 0x40,       /* Nivel VCOMH. */
    0xA4, 0xA6,       /* Mostrar RAM, sin inversion. */
    0x2E,             /* Desactivar desplazamiento automatico. */
    0x21, 0, WIDTH-1, /* Columnas 0 a 127. */
    0x22, 0, PAGES-1  /* Paginas 0 a 7. */
  };

  if (enviar(fd, inicio, sizeof inicio) < 0)
    goto terminar;

  for (int pagina = 0; pagina < PAGES; ++pagina) {
    for (int columna = 0; columna < WIDTH; columna += CHUNK) {
      uint8_t bloque[CHUNK + 1];
      bloque[0] = DATA;
      for (int i = 0; i < CHUNK; ++i) {
        /* Cada byte representa ocho pixeles verticales. */
        bloque[i + 1] =
          (((columna + i) / 8 + pagina) % 2) ? 0xFF : 0x00;
      }
      if (enviar(fd, bloque, sizeof bloque) < 0)
        goto terminar;
    }
  }

  const uint8_t encender[] = { COMMAND, 0xAF };
  if (enviar(fd, encender, sizeof encender) < 0)
    goto terminar;

  puts("Patron enviado. Comprueba visualmente la pantalla.");
  resultado = EXIT_SUCCESS;

terminar:
  if (close(fd) < 0) {
    perror("Error al cerrar I2C");
    resultado = EXIT_FAILURE;
  }
  return resultado;
}
