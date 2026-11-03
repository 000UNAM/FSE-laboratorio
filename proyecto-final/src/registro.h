#ifndef PROYECTO_REGISTRO_H
#define PROYECTO_REGISTRO_H

#include "bme280.h"
#include <time.h>

/* Estado privado del archivo CSV, definido en registro.c. */
typedef struct registro registro;

/*
 * Crea un archivo CSV nuevo y escribe su encabezado.
 * Si la ruta ya existe, falla para no sobrescribir evidencias.
 * La carpeta de destino debe existir.
 * Devuelve NULL si ocurre un error.
 */
registro *registro_abrir(const char *ruta);

/*
 * Guarda una medicion y vacia el buffer de la biblioteca C.
 *
 * instante: fecha y hora de adquisicion.
 * transcurrido_s: segundos desde el inicio, medidos con
 * un reloj monotono, independiente de ajustes de fecha.
 *
 * Devuelve 0 si tiene exito o -1 si falla.
 * Si falla, el programa debe dejar de escribir en ese registro.
 */
int registro_guardar(registro *archivo,
                     const struct timespec *instante,
                     double transcurrido_s,
                     const bme280_medicion *medicion);

/*
 * Cierra el archivo y libera sus recursos.
 * Devuelve 0 si tiene exito o -1 si falla.
 * El puntero deja de ser valido en ambos casos.
 */
int registro_cerrar(registro *archivo);

#endif
