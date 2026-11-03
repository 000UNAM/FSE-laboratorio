#define _POSIX_C_SOURCE 200809L

#include "bme280.h"
#include "oled.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BUS_I2C "/dev/i2c-1"

enum {
  DIRECCION_BME280 = 0x76,
  DIRECCION_OLED = 0x3C,
  NUM_MUESTRAS = 10,
  PAUSA_SEGUNDOS = 2,
  TAM_LINEA = 22
};

/* Prueba de integracion. Elaborada con asistencia de IA. */

static int formar_linea(char *destino, const char *formato,
                        double valor)
{
  int longitud = snprintf(destino, TAM_LINEA, formato, valor);

  if (longitud < 0) {
    errno = EILSEQ;
    return -1;
  }

  if (longitud >= TAM_LINEA) {
    errno = EMSGSIZE;
    return -1;
  }

  return 0;
}

static int esperar(void)
{
  struct timespec pausa = {
    .tv_sec = PAUSA_SEGUNDOS,
    .tv_nsec = 0
  };

  while (nanosleep(&pausa, &pausa) < 0) {
    if (errno != EINTR) {
      return -1;
    }
  }

  return 0;
}

int main(void)
{
  int resultado = EXIT_FAILURE;

  oled *pantalla = oled_abrir(BUS_I2C, DIRECCION_OLED);

  if (pantalla == NULL) {
    perror("Inicializar OLED");
    return EXIT_FAILURE;
  }

  bme280 *sensor = bme280_abrir(BUS_I2C, DIRECCION_BME280);

  if (sensor == NULL) {
    perror("Inicializar BME280");

    if (oled_mostrar(pantalla, "ERROR SENSOR",
                     "SIN MEDICIONES", "REVISAR TERMINAL", "") < 0) {
      perror("Mostrar error en OLED");
    }

    goto cerrar_pantalla;
  }

  for (int i = 0; i < NUM_MUESTRAS; i++) {
    bme280_medicion medicion;
    char temperatura[TAM_LINEA];
    char humedad[TAM_LINEA];
    char presion[TAM_LINEA];

    if (bme280_leer(sensor, &medicion) < 0) {
      perror("Leer BME280");
      goto fallo_medicion;
    }

    if (formar_linea(temperatura, "TEMP: %.2f C",
                     medicion.temperatura_c) < 0 ||
        formar_linea(humedad, "HUM: %.2f %%",
                     medicion.humedad_pct) < 0 ||
        formar_linea(presion, "PRES: %.2f HPA",
                     medicion.presion_hpa) < 0) {
      perror("Formar texto");
      goto fallo_medicion;
    }

    if (oled_mostrar(pantalla, "ESTACION AMBIENTAL",
                     temperatura, humedad, presion) < 0) {
      perror("Actualizar OLED");
      goto cerrar_sensor;
    }

    if (printf("Muestra %d | %s | %s | %s\n",
               i + 1, temperatura, humedad, presion) < 0 ||
        fflush(stdout) == EOF) {
      perror("Escribir en terminal");
      goto cerrar_sensor;
    }

    if (i + 1 < NUM_MUESTRAS && esperar() < 0) {
      perror("Esperar entre muestras");
      goto cerrar_sensor;
    }
  }

  /*
   * Indica que termino la prueba para no dejar una medicion
   * congelada con apariencia de lectura en tiempo real.
   */
  if (oled_mostrar(pantalla, "PRUEBA TERMINADA",
                   "10 MUESTRAS", "CONSULTAR REGISTRO", "") < 0) {
    perror("Mostrar fin de prueba");
    goto cerrar_sensor;
  }

  resultado = EXIT_SUCCESS;
  goto cerrar_sensor;

fallo_medicion:
  if (oled_mostrar(pantalla, "ERROR DE MEDICION",
                   "DATOS NO VALIDOS", "REVISAR TERMINAL", "") < 0) {
    perror("Mostrar error en OLED");
  }

cerrar_sensor:
  if (bme280_cerrar(sensor) < 0) {
    perror("Cerrar BME280");
    resultado = EXIT_FAILURE;
  }

cerrar_pantalla:
  if (oled_cerrar(pantalla) < 0) {
    perror("Cerrar OLED");
    resultado = EXIT_FAILURE;
  }

  return resultado;
}
