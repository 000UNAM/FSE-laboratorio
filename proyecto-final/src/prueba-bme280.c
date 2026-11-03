#define _POSIX_C_SOURCE 200809L

#include "bme280.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BUS_I2C "/dev/i2c-1"

enum {
  DIRECCION_SENSOR = 0x76,
  NUM_MUESTRAS = 5,
  PAUSA_SEGUNDOS = 2
};

/* Prueba del modulo BME280. Elaborada con asistencia de IA. */
int main(void)
{
  bme280 *sensor = bme280_abrir(BUS_I2C, DIRECCION_SENSOR);

  if (sensor == NULL) {
    perror("Inicializar BME280");
    return EXIT_FAILURE;
  }

  int resultado = EXIT_SUCCESS;

  for (int i = 0; i < NUM_MUESTRAS; i++) {
    bme280_medicion medicion;

    if (bme280_leer(sensor, &medicion) < 0) {
      perror("Leer BME280");
      resultado = EXIT_FAILURE;
      break;
    }

    if (printf(
          "Muestra %d | Temperatura: %.2f C | "
          "Humedad: %.2f %% | Presion: %.2f hPa\n",
          i + 1,
          medicion.temperatura_c,
          medicion.humedad_pct,
          medicion.presion_hpa) < 0 ||
        fflush(stdout) == EOF) {
      perror("Mostrar medicion");
      resultado = EXIT_FAILURE;
      break;
    }

    /* No espera despues de la ultima muestra. */
    if (i + 1 < NUM_MUESTRAS) {
      struct timespec pausa = {
        .tv_sec = PAUSA_SEGUNDOS,
        .tv_nsec = 0
      };

      while (nanosleep(&pausa, &pausa) < 0) {
        if (errno != EINTR) {
          perror("Esperar entre muestras");
          resultado = EXIT_FAILURE;
          break;
        }
      }

      if (resultado != EXIT_SUCCESS) {
        break;
      }
    }
  }

  if (bme280_cerrar(sensor) < 0) {
    perror("Cerrar BME280");
    resultado = EXIT_FAILURE;
  }

  return resultado;
}
