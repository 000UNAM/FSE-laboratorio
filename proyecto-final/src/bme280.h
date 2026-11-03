#ifndef PROYECTO_BME280_H
#define PROYECTO_BME280_H

#include <stdint.h>

/* Datos de una medicion completa, expresados en estas unidades. */
typedef struct {
  double temperatura_c;
  double humedad_pct;
  double presion_hpa;
} bme280_medicion;

/* Los detalles internos del sensor se definiran en bme280.c. */
typedef struct bme280 bme280;

/*
 * Abre el bus, verifica la identidad y configura el sensor.
 * Devuelve NULL si ocurre un error.
 */
bme280 *bme280_abrir(const char *bus, uint8_t direccion);

/*
 * Obtiene una medicion nueva.
 * Devuelve 0 si tiene exito o -1 si falla.
 * Si falla, el contenido de salida no debe utilizarse.
 */
int bme280_leer(bme280 *sensor, bme280_medicion *salida);

/*
 * Cierra el descriptor y libera la memoria del sensor.
 * Devuelve 0 si tiene exito o -1 si falla el cierre.
 * El puntero deja de ser valido en ambos casos.
 */
int bme280_cerrar(bme280 *sensor);

#endif
