#define _POSIX_C_SOURCE 200809L

#include "oled.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>

/*
 * Modulo para OLED SSD1306 de 128 x 64 pixeles.
 * Elaborado con asistencia de IA.
 *
 * Fuente de 5 x 7 pixeles, con una columna de separacion.
 * Las minusculas se muestran como mayusculas.
 * Los simbolos no incluidos se representan con '?'.
 */

enum {
  OLED_ANCHO = 128,
  OLED_PAGINAS = 8,
  OLED_BYTES = OLED_ANCHO * OLED_PAGINAS,
  OLED_COLUMNAS_LETRA = 5,
  OLED_PASO_LETRA = 6,
  OLED_MAX_CARACTERES = 21,
  OLED_BLOQUE = 16,
  OLED_CONTROL_COMANDO = 0x00,
  OLED_CONTROL_DATOS = 0x40
};

struct oled {
  int fd;
  uint8_t imagen[OLED_BYTES];
};

typedef struct {
  char caracter;
  uint8_t columnas[OLED_COLUMNAS_LETRA];
} glifo;

/* Cada byte representa una columna vertical de pixeles. */
static const glifo fuente[] = {
  {' ', {0x00, 0x00, 0x00, 0x00, 0x00}},
  {'!', {0x00, 0x00, 0x5F, 0x00, 0x00}},
  {'%', {0x23, 0x13, 0x08, 0x64, 0x62}},
  {'(', {0x00, 0x1C, 0x22, 0x41, 0x00}},
  {')', {0x00, 0x41, 0x22, 0x1C, 0x00}},
  {'+', {0x08, 0x08, 0x3E, 0x08, 0x08}},
  {'-', {0x08, 0x08, 0x08, 0x08, 0x08}},
  {'.', {0x00, 0x60, 0x60, 0x00, 0x00}},
  {'/', {0x20, 0x10, 0x08, 0x04, 0x02}},
  {'0', {0x3E, 0x51, 0x49, 0x45, 0x3E}},
  {'1', {0x00, 0x42, 0x7F, 0x40, 0x00}},
  {'2', {0x42, 0x61, 0x51, 0x49, 0x46}},
  {'3', {0x21, 0x41, 0x45, 0x4B, 0x31}},
  {'4', {0x18, 0x14, 0x12, 0x7F, 0x10}},
  {'5', {0x27, 0x45, 0x45, 0x45, 0x39}},
  {'6', {0x3C, 0x4A, 0x49, 0x49, 0x30}},
  {'7', {0x01, 0x71, 0x09, 0x05, 0x03}},
  {'8', {0x36, 0x49, 0x49, 0x49, 0x36}},
  {'9', {0x06, 0x49, 0x49, 0x29, 0x1E}},
  {':', {0x00, 0x36, 0x36, 0x00, 0x00}},
  {'=', {0x14, 0x14, 0x14, 0x14, 0x14}},
  {'?', {0x02, 0x01, 0x51, 0x09, 0x06}},
  {'A', {0x7E, 0x11, 0x11, 0x11, 0x7E}},
  {'B', {0x7F, 0x49, 0x49, 0x49, 0x36}},
  {'C', {0x3E, 0x41, 0x41, 0x41, 0x22}},
  {'D', {0x7F, 0x41, 0x41, 0x22, 0x1C}},
  {'E', {0x7F, 0x49, 0x49, 0x49, 0x41}},
  {'F', {0x7F, 0x09, 0x09, 0x09, 0x01}},
  {'G', {0x3E, 0x41, 0x49, 0x49, 0x7A}},
  {'H', {0x7F, 0x08, 0x08, 0x08, 0x7F}},
  {'I', {0x00, 0x41, 0x7F, 0x41, 0x00}},
  {'J', {0x20, 0x40, 0x41, 0x3F, 0x01}},
  {'K', {0x7F, 0x08, 0x14, 0x22, 0x41}},
  {'L', {0x7F, 0x40, 0x40, 0x40, 0x40}},
  {'M', {0x7F, 0x02, 0x0C, 0x02, 0x7F}},
  {'N', {0x7F, 0x04, 0x08, 0x10, 0x7F}},
  {'O', {0x3E, 0x41, 0x41, 0x41, 0x3E}},
  {'P', {0x7F, 0x09, 0x09, 0x09, 0x06}},
  {'Q', {0x3E, 0x41, 0x51, 0x21, 0x5E}},
  {'R', {0x7F, 0x09, 0x19, 0x29, 0x46}},
  {'S', {0x46, 0x49, 0x49, 0x49, 0x31}},
  {'T', {0x01, 0x01, 0x7F, 0x01, 0x01}},
  {'U', {0x3F, 0x40, 0x40, 0x40, 0x3F}},
  {'V', {0x1F, 0x20, 0x40, 0x20, 0x1F}},
  {'W', {0x3F, 0x40, 0x38, 0x40, 0x3F}},
  {'X', {0x63, 0x14, 0x08, 0x14, 0x63}},
  {'Y', {0x07, 0x08, 0x70, 0x08, 0x07}},
  {'Z', {0x61, 0x51, 0x49, 0x45, 0x43}},
  {'_', {0x40, 0x40, 0x40, 0x40, 0x40}}
};

static const uint8_t *buscar_glifo(unsigned char caracter)
{
  static const uint8_t desconocido[OLED_COLUMNAS_LETRA] = {
    0x02, 0x01, 0x51, 0x09, 0x06
  };

  if (caracter >= 'a' && caracter <= 'z') {
    caracter = (unsigned char)(caracter - 'a' + 'A');
  }

  for (size_t i = 0; i < sizeof fuente / sizeof fuente[0]; i++) {
    if ((unsigned char)fuente[i].caracter == caracter) {
      return fuente[i].columnas;
    }
  }

  return desconocido;
}

/*
 * Cada escritura debe enviar la transaccion completa.
 * Si queda incompleta, informa error; no envia el resto
 * como otra transaccion sin su byte de control.
 */
static int enviar(oled *pantalla, const uint8_t *datos,
                  size_t cantidad)
{
  ssize_t enviados = write(pantalla->fd, datos, cantidad);

  if (enviados < 0) {
    return -1;
  }

  if ((size_t)enviados != cantidad) {
    errno = EIO;
    return -1;
  }

  return 0;
}

/* Envia toda la imagen usando direccionamiento horizontal. */
static int actualizar(oled *pantalla)
{
  const uint8_t posicion[] = {
    OLED_CONTROL_COMANDO,
    0x21, 0x00, OLED_ANCHO - 1,   /* Columnas 0 a 127. */
    0x22, 0x00, OLED_PAGINAS - 1  /* Paginas 0 a 7. */
  };

  if (enviar(pantalla, posicion, sizeof posicion) < 0) {
    return -1;
  }

  for (size_t inicio = 0; inicio < OLED_BYTES;
       inicio += OLED_BLOQUE) {
    uint8_t paquete[OLED_BLOQUE + 1];

    paquete[0] = OLED_CONTROL_DATOS;
    memcpy(&paquete[1], &pantalla->imagen[inicio], OLED_BLOQUE);

    if (enviar(pantalla, paquete, sizeof paquete) < 0) {
      return -1;
    }
  }

  return 0;
}

oled *oled_abrir(const char *bus, uint8_t direccion)
{
  if (bus == NULL ||
      (direccion != 0x3C && direccion != 0x3D)) {
    errno = EINVAL;
    return NULL;
  }

  oled *pantalla = calloc(1, sizeof *pantalla);

  if (pantalla == NULL) {
    return NULL;
  }

  pantalla->fd = open(bus, O_RDWR | O_CLOEXEC);

  if (pantalla->fd < 0) {
    int error_original = errno;
    free(pantalla);
    errno = error_original;
    return NULL;
  }

  if (ioctl(pantalla->fd, I2C_SLAVE, direccion) < 0) {
    goto fallo;
  }

  /*
   * Configuracion para el modulo de 128 x 64 probado.
   * Se mantiene apagado mientras se limpia su memoria.
   */
  const uint8_t configuracion[] = {
    OLED_CONTROL_COMANDO,
    0xAE,        /* Pantalla apagada. */
    0xD5, 0x80,  /* Reloj del controlador. */
    0xA8, 0x3F,  /* Multiplexado de 64 filas. */
    0xD3, 0x00,  /* Sin desplazamiento vertical. */
    0x40,        /* Linea inicial 0. */
    0x8D, 0x14,  /* Bomba de carga habilitada. */
    0x20, 0x00,  /* Direccionamiento horizontal. */
    0xA1,        /* Orientacion de columnas. */
    0xC8,        /* Orientacion de filas. */
    0xDA, 0x12,  /* Configuracion de terminales COM. */
    0x81, 0x40,  /* Contraste. */
    0xD9, 0xF1,  /* Periodo de precarga. */
    0xDB, 0x20,  /* VCOMH: aproximadamente 0.77 de VCC. */
    0xA4,        /* Mostrar el contenido de la memoria. */
    0xA6,        /* Imagen normal, sin inversion. */
    0x2E         /* Desplazamiento automatico desactivado. */
  };

  if (enviar(pantalla, configuracion, sizeof configuracion) < 0 ||
      actualizar(pantalla) < 0) {
    goto fallo;
  }

  const uint8_t encender[] = {
    OLED_CONTROL_COMANDO, 0xAF
  };

  if (enviar(pantalla, encender, sizeof encender) < 0) {
   goto fallo;
  }

  return pantalla;

fallo:
  {
    int error_original = errno;

    if (close(pantalla->fd) < 0) {
      perror("Cerrar OLED tras fallo de inicializacion");
    }

    free(pantalla);
    errno = error_original;
    return NULL;
  }
}

/* Dibuja una linea ya validada en la imagen en memoria. */
static void dibujar_linea(oled *pantalla, size_t pagina,
                          const char *texto)
{
  for (size_t i = 0; texto[i] != '\0'; i++) {
    const uint8_t *columnas =
      buscar_glifo((unsigned char)texto[i]);

    size_t posicion =
      pagina * OLED_ANCHO + i * OLED_PASO_LETRA;

    memcpy(&pantalla->imagen[posicion],
           columnas, OLED_COLUMNAS_LETRA);
  }
}

int oled_mostrar(oled *pantalla,
                 const char *linea1,
                 const char *linea2,
                 const char *linea3,
                 const char *linea4)
{
  const char *lineas[] = {linea1, linea2, linea3, linea4};
  enum { NUM_LINEAS = 4, PASO_PAGINAS = 2 };

  if (pantalla == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* Valida todo antes de modificar la imagen. */
  for (size_t i = 0; i < NUM_LINEAS; i++) {
    if (lineas[i] == NULL) {
      errno = EINVAL;
      return -1;
    }

    size_t longitud =
      strnlen(lineas[i], OLED_MAX_CARACTERES + 1);

    if (longitud > OLED_MAX_CARACTERES) {
      errno = EMSGSIZE;
      return -1;
    }

    for (size_t j = 0; j < longitud; j++) {
      unsigned char caracter = (unsigned char)lineas[i][j];

      /* Solo ASCII imprimible: sin saltos de linea ni acentos. */
      if (caracter < 32 || caracter > 126) {
        errno = EILSEQ;
        return -1;
      }
    }
  }

  /* Borra letras anteriores, incluso si el texto nuevo es menor. */
  memset(pantalla->imagen, 0, sizeof pantalla->imagen);

  for (size_t i = 0; i < NUM_LINEAS; i++) {
    dibujar_linea(pantalla, i * PASO_PAGINAS, lineas[i]);
  }

  return actualizar(pantalla);
}

int oled_cerrar(oled *pantalla)
{
  if (pantalla == NULL) {
    errno = EINVAL;
    return -1;
  }

  int resultado = close(pantalla->fd);
  int error_cierre = errno;

  free(pantalla);

  if (resultado < 0) {
    errno = error_cierre;
  }

  return resultado;
}
