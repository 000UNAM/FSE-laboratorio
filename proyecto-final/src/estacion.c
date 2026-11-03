#define _POSIX_C_SOURCE 200809L

#include "bme280.h"
#include "oled.h"
#include "registro.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUS_I2C "/dev/i2c-1"

enum {
  DIRECCION_SENSOR = 0x76,
  DIRECCION_PANTALLA = 0x3C,
  PERIODO_LECTURA_S = 2,
  PERIODO_REGISTRO_S = 10,
  TAM_LINEA = 22
};

/* Programa principal. Elaborado con asistencia de IA. */

static volatile sig_atomic_t detener = 0;

/*
 * El manejador solo cambia una bandera.
 * El cierre de archivos y dispositivos ocurre en main().
 */
static void solicitar_parada(int numero)
{
  (void)numero;
  detener = 1;
}

static int configurar_senales(void)
{
  struct sigaction accion = {0};
  accion.sa_handler = solicitar_parada;

  if (sigemptyset(&accion.sa_mask) < 0) {
    return -1;
  }

  if (sigaction(SIGINT, &accion, NULL) < 0 ||
      sigaction(SIGTERM, &accion, NULL) < 0) {
    return -1;
  }

  return 0;
}

/* Diferencia de tiempo entre dos lecturas del mismo reloj. */
static double segundos_entre(const struct timespec *fin,
                             const struct timespec *inicio)
{
  return difftime(fin->tv_sec, inicio->tv_sec) +
         (fin->tv_nsec - inicio->tv_nsec) / 1000000000.0;
}

/*
 * Espera hasta una fecha del reloj monotono.
 * Usar un plazo absoluto evita sumar el tiempo de trabajo
 * al periodo de cada ciclo.
 *
 * Devuelve 0 al alcanzar el plazo, 1 si se solicita parar
 * o -1 si ocurre un error.
 */
static int esperar_hasta(const struct timespec *plazo)
{
  while (!detener) {
    int error = clock_nanosleep(
      CLOCK_MONOTONIC, TIMER_ABSTIME, plazo, NULL
    );

    if (error == 0) {
      return detener ? 1 : 0;
    }

    /*
     * clock_nanosleep devuelve directamente el numero
     * de error; lo copiamos a errno para usar perror().
     */
    if (error != EINTR) {
      errno = error;
      return -1;
    }
  }

  return 1;
}

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

static int mostrar_medicion(oled *pantalla,
                            const bme280_medicion *medicion)
{
  char temperatura[TAM_LINEA];
  char humedad[TAM_LINEA];
  char presion[TAM_LINEA];

  if (formar_linea(temperatura, "TEMP: %.2f C",
                   medicion->temperatura_c) < 0 ||
      formar_linea(humedad, "HUM: %.2f %%",
                   medicion->humedad_pct) < 0 ||
      formar_linea(presion, "PRES: %.2f HPA",
                   medicion->presion_hpa) < 0) {
    return -1;
  }

  return oled_mostrar(pantalla, "ESTACION AMBIENTAL",
                      temperatura, humedad, presion);
}

int main(int argc, char *argv[])
{
  /*
   * Uso:
   * ./estacion archivo.csv [0x76|0x77]
   *
   * La direccion opcional permite probar un sensor ausente
   * sin desconectar cables ni modificar el codigo.
   */
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Uso: %s archivo.csv [0x76|0x77]\n", argv[0]);
    return EXIT_FAILURE;
  }

  uint8_t direccion = DIRECCION_SENSOR;

  if (argc == 3) {
    if (strcmp(argv[2], "0x77") == 0) {
      direccion = 0x77;
    } else if (strcmp(argv[2], "0x76") != 0) {
      fprintf(stderr, "Direccion permitida: 0x76 o 0x77\n");
      return EXIT_FAILURE;
    }
  }

  if (configurar_senales() < 0) {
    perror("Configurar senales");
    return EXIT_FAILURE;
  }

  int resultado = EXIT_FAILURE;
  const char *estado_final = "ERROR DEL SISTEMA";

  oled *pantalla = NULL;
  bme280 *sensor = NULL;
  registro *archivo = NULL;

  pantalla = oled_abrir(BUS_I2C, DIRECCION_PANTALLA);

  if (pantalla == NULL) {
    perror("Inicializar OLED");
    goto terminar;
  }

  if (oled_mostrar(pantalla, "INICIANDO",
                   "ESTACION AMBIENTAL", "", "") < 0) {
    perror("Mostrar inicio");
    goto terminar;
  }

  archivo = registro_abrir(argv[1]);

  if (archivo == NULL) {
    estado_final = "ERROR DE ARCHIVO";
    perror("Crear CSV");
    goto terminar;
  }

  sensor = bme280_abrir(BUS_I2C, direccion);

  if (sensor == NULL) {
    estado_final = "ERROR SENSOR";
    perror("Inicializar BME280");
    goto terminar;
  }

  struct timespec inicio;

  if (clock_gettime(CLOCK_MONOTONIC, &inicio) < 0) {
    perror("Leer reloj monotono");
    goto terminar;
  }

  struct timespec proxima_lectura = inicio;
  double proximo_registro = PERIODO_REGISTRO_S;

  /*
   * Decision temporal:
   * plazos absolutos con CLOCK_MONOTONIC.
   * Primera lectura inmediata; primer registro hacia t = 10 s.
   */
  while (!detener) {
    int espera = esperar_hasta(&proxima_lectura);

    if (espera < 0) {
      perror("Esperar siguiente ciclo");
      goto terminar;
    }

    if (espera > 0) {
      break;
    }

    bme280_medicion medicion;

    if (bme280_leer(sensor, &medicion) < 0) {
      estado_final = "ERROR SENSOR";
      perror("Leer BME280");
      goto terminar;
    }

    /* Marca temporal al terminar la adquisicion. */
    struct timespec instante;
    struct timespec ahora;

    if (clock_gettime(CLOCK_REALTIME, &instante) < 0 ||
        clock_gettime(CLOCK_MONOTONIC, &ahora) < 0) {
      perror("Obtener fecha de medicion");
      goto terminar;
    }

    double transcurrido = segundos_entre(&ahora, &inicio);
    int guardado = 0;

    /*
     * Guarda una medicion actual al alcanzar cada plazo.
     * No inventa muestras para cubrir intervalos perdidos.
     */
    if (transcurrido >= proximo_registro) {
      if (registro_guardar(archivo, &instante,
                           transcurrido, &medicion) < 0) {
        estado_final = "ERROR DE ARCHIVO";
        perror("Guardar CSV");
        goto terminar;
      }

      guardado = 1;

      do {
        proximo_registro += PERIODO_REGISTRO_S;
      } while (proximo_registro <= transcurrido);
    }

    if (mostrar_medicion(pantalla, &medicion) < 0) {
      estado_final = "ERROR PANTALLA";
      perror("Actualizar OLED");
      goto terminar;
    }

    if (printf(
          "t=%.3f s | TEMP=%.2f C | HUM=%.2f %% | "
          "PRES=%.2f hPa | CSV=%s\n",
          transcurrido,
          medicion.temperatura_c,
          medicion.humedad_pct,
          medicion.presion_hpa,
          guardado ? "SI" : "NO") < 0 ||
        fflush(stdout) == EOF) {
      perror("Escribir registro de terminal");
      goto terminar;
    }

    proxima_lectura.tv_sec += PERIODO_LECTURA_S;

    /*
     * Detecta si el trabajo del ciclo consumio todo el periodo.
     * Esta version se detiene en lugar de acumular retrasos
     * o ejecutar muchas lecturas consecutivas para recuperarse.
     */
    if (clock_gettime(CLOCK_MONOTONIC, &ahora) < 0) {
      perror("Comprobar duracion del ciclo");
      goto terminar;
    }

    if (!detener && segundos_entre(&ahora, &proxima_lectura) >= 0.0) {
      estado_final = "ERROR DE TIEMPO";
      errno = ETIMEDOUT;
      perror("Ciclo excedio el periodo de lectura");
      goto terminar;
    }
  }

  resultado = EXIT_SUCCESS;
  estado_final = "ESTACION DETENIDA";

terminar:
  if (archivo != NULL && registro_cerrar(archivo) < 0) {
    perror("Cerrar CSV");
    estado_final = "ERROR DE ARCHIVO";
    resultado = EXIT_FAILURE;
  }

  if (sensor != NULL && bme280_cerrar(sensor) < 0) {
    perror("Cerrar BME280");
    estado_final = "ERROR SENSOR";
    resultado = EXIT_FAILURE;
  }

  if (pantalla != NULL) {
    /* Evita dejar mediciones congeladas como si fueran actuales. */
    if (oled_mostrar(pantalla, estado_final,
                     "SIN ACTUALIZACION",
                     "CONSULTAR REGISTRO", "") < 0) {
      perror("Mostrar estado final");
      resultado = EXIT_FAILURE;
    }

    if (oled_cerrar(pantalla) < 0) {
      perror("Cerrar OLED");
      resultado = EXIT_FAILURE;
    }
  }

  return resultado;
}
