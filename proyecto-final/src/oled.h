#ifndef PROYECTO_OLED_H
#define PROYECTO_OLED_H

#include <stdint.h>

/* Estado interno de la pantalla, definido en oled.c. */
typedef struct oled oled;

/*
 * Abre y configura una OLED SSD1306 de 128 x 64 pixeles.
 * Devuelve NULL si ocurre un error.
 */
oled *oled_abrir(const char *bus, uint8_t direccion);

/*
 * Muestra cuatro lineas de texto.
 * Cada linea admite hasta 21 caracteres ASCII.
 * Usar texto sin acentos; por ejemplo, "Temperatura".
 *
 * Devuelve 0 si tiene exito o -1 si falla.
 */
int oled_mostrar(oled *pantalla,
                 const char *linea1,
                 const char *linea2,
                 const char *linea3,
                 const char *linea4);

/*
 * Cierra la conexion y libera los recursos.
 * No apaga ni borra la pantalla.
 * Devuelve 0 si tiene exito o -1 si falla el cierre.
 * El puntero deja de ser valido en ambos casos.
 */
int oled_cerrar(oled *pantalla);

#endif
