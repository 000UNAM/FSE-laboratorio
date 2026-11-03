#define _POSIX_C_SOURCE 200809L

#include "registro.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Registro CSV. Elaborado con asistencia de IA. */

struct registro {
  FILE *flujo;
  int fallo;
};

registro *registro_abrir(const char *ruta)
{
  if (ruta == NULL || ruta[0] == '\0') {
    errno = EINVAL;
    return NULL;
  }

  registro *archivo = calloc(1, sizeof *archivo);

  if (archivo == NULL) {
    return NULL;
  }

  /*
   * "w": escritura.
   * "x": creacion exclusiva; falla si el archivo ya existe.
   */
  archivo->flujo = fopen(ruta, "wx");

  if (archivo->flujo == NULL) {
    int error_original = errno;
    free(archivo);
    errno = error_original;
    return NULL;
  }

  const char *encabezado =
    "fecha_utc,transcurrido_s,temperatura_c,"
    "humedad_pct,presion_hpa\n";

  if (fputs(encabezado, archivo->flujo) == EOF ||
      fflush(archivo->flujo) == EOF) {
    int error_original = errno;

    if (fclose(archivo->flujo) == EOF) {
      perror("Cerrar CSV tras fallo de encabezado");
    }

    free(archivo);
    errno = error_original;
    return NULL;
  }

  return archivo;
}

int registro_guardar(registro *archivo,
                     const struct timespec *instante,
                     double transcurrido_s,
                     const bme280_medicion *medicion)
{
  if (archivo == NULL || instante == NULL || medicion == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (archivo->fallo) {
    errno = EIO;
    return -1;
  }

  if (instante->tv_nsec < 0 || instante->tv_nsec >= 1000000000L ||
      !isfinite(transcurrido_s) || transcurrido_s < 0.0 ||
      !isfinite(medicion->temperatura_c) ||
      !isfinite(medicion->humedad_pct) ||
      !isfinite(medicion->presion_hpa)) {
    errno = EINVAL;
    return -1;
  }

  /*
   * UTC evita ambiguedades de zona horaria.
   * La letra Z al final de la fecha indica UTC.
   */
  struct tm calendario;

  if (gmtime_r(&instante->tv_sec, &calendario) == NULL) {
    return -1;
  }

  char fecha[32];

  if (strftime(fecha, sizeof fecha,
               "%Y-%m-%dT%H:%M:%S", &calendario) == 0) {
    errno = EOVERFLOW;
    return -1;
  }

  long milisegundos = instante->tv_nsec / 1000000L;

  /*
   * El programa principal debe conservar el punto decimal
   * de la configuracion regional C, predeterminada al iniciar.
   */
  int escritos = fprintf(
    archivo->flujo,
    "%s.%03ldZ,%.3f,%.2f,%.2f,%.2f\n",
    fecha,
    milisegundos,
    transcurrido_s,
    medicion->temperatura_c,
    medicion->humedad_pct,
    medicion->presion_hpa
  );

  if (escritos < 0) {
    archivo->fallo = 1;
    return -1;
  }

  /*
   * Decision de almacenamiento para documentar:
   * vaciar el buffer despues de cada registro.
   * Esto permite consultar el CSV mientras sigue ejecutandose.
   */
  if (fflush(archivo->flujo) == EOF) {
    archivo->fallo = 1;
    return -1;
  }

  return 0;
}

int registro_cerrar(registro *archivo)
{
  if (archivo == NULL) {
    errno = EINVAL;
    return -1;
  }

  int fallo_previo = archivo->fallo;
  int resultado = fclose(archivo->flujo);
  int error_cierre = errno;

  free(archivo);

  if (resultado == EOF) {
    errno = error_cierre;
    return -1;
  }

  if (fallo_previo) {
    errno = EIO;
    return -1;
  }

  return 0;
}
