#define _POSIX_C_SOURCE 200809L

#include <gpiod.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

enum fase { VERDE, AMARILLO, ROJO, CRUCE };

static volatile sig_atomic_t detener = 0;

static void interrumpir(int senal)
{
  (void)senal;
  detener = 1;
}

static int64_t tiempo_ms(void)
{
  struct timespec t;

  if (clock_gettime(CLOCK_MONOTONIC, &t) < 0)
    return -1;

  return (int64_t)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

int main(void)
{
  const unsigned int leds[] = {17, 27, 22, 23};
  const unsigned int boton = 5;
  const struct timespec pausa = {0, 10000000}; /* 10 ms */

  struct gpiod_chip *chip = NULL;
  struct gpiod_line_settings *ajustes = NULL;
  struct gpiod_line_config *config = NULL;
  struct gpiod_line_request *solicitud = NULL;

  int resultado = EXIT_FAILURE;
  enum fase fase = VERDE;
  int pendiente = 0;
  int candidato = 0;
  int estable = 0;
  int anteriores[] = {-1, -1, -1, -1};
  int64_t inicio, cambio;

  struct sigaction accion = {0};
  accion.sa_handler = interrumpir;
  sigemptyset(&accion.sa_mask);

  if (sigaction(SIGINT, &accion, NULL) < 0 ||
      sigaction(SIGTERM, &accion, NULL) < 0) {
    perror("Configurar senales");
    return EXIT_FAILURE;
  }

  chip = gpiod_chip_open("/dev/gpiochip0");
  if (!chip) {
    perror("Abrir controlador");
    goto limpiar;
  }

  ajustes = gpiod_line_settings_new();
  if (!ajustes) {
    perror("Crear ajustes");
    goto limpiar;
  }

  config = gpiod_line_config_new();
  if (!config) {
    perror("Crear configuracion");
    goto limpiar;
  }

  if (gpiod_line_settings_set_direction(
        ajustes, GPIOD_LINE_DIRECTION_OUTPUT) < 0 ||
      gpiod_line_settings_set_output_value(
        ajustes, GPIOD_LINE_VALUE_INACTIVE) < 0 ||
      gpiod_line_config_add_line_settings(
        config, leds, 4, ajustes) < 0) {
    perror("Configurar LEDs");
    goto limpiar;
  }

  gpiod_line_settings_reset(ajustes);

  if (gpiod_line_settings_set_direction(
        ajustes, GPIOD_LINE_DIRECTION_INPUT) < 0 ||
      gpiod_line_settings_set_bias(
        ajustes, GPIOD_LINE_BIAS_DISABLED) < 0 ||
      gpiod_line_config_add_line_settings(
        config, &boton, 1, ajustes) < 0) {
    perror("Configurar boton");
    goto limpiar;
  }

  solicitud = gpiod_chip_request_lines(chip, NULL, config);
  if (!solicitud) {
    perror("Solicitar GPIO");
    goto limpiar;
  }

  inicio = tiempo_ms();
  if (inicio < 0) {
    perror("Leer reloj");
    goto limpiar;
  }
  cambio = inicio;

  puts("Semaforo iniciado. Ctrl+C para terminar.");
  puts("VERDE: 5 s");
  fflush(stdout);

  /*
   * Eleccion: polling con pausa de 10 ms.
   * Permite atender el boton, los tiempos y el parpadeo.
   * La pausa evita el bucle ocupado de la parte D.
   * El reloj monotono mide tiempos sin depender de la hora.
   */
  while (!detener) {
    int64_t ahora = tiempo_ms();
    if (ahora < 0) {
      perror("Leer reloj");
      goto limpiar;
    }

    int lectura = gpiod_line_request_get_value(solicitud, boton);
    if (lectura < 0) {
      if (detener && errno == EINTR)
        break;
      perror("Leer boton");
      goto limpiar;
    }

    /* Antirrebote: aceptar un estado tras 50 ms estable. */
    if (lectura != candidato) {
      candidato = lectura;
      cambio = ahora;
    }

    if (candidato != estable && ahora - cambio >= 50) {
      estable = candidato;

      /*
       * Una peticion pendiente agrupa pulsaciones adicionales.
       * Durante CRUCE no se prolonga ni se solicita otro cruce.
       */
      if (estable && fase != CRUCE && !pendiente) {
        pendiente = 1;
        puts("Peticion registrada.");
        fflush(stdout);
      }
    }

    enum fase siguiente = fase;
    int64_t transcurrido = ahora - inicio;

    if (fase == VERDE && transcurrido >= 5000)
      siguiente = AMARILLO;
    else if (fase == AMARILLO && transcurrido >= 2000)
      siguiente = pendiente ? CRUCE : ROJO;
    else if (fase == ROJO && transcurrido >= 5000)
      siguiente = pendiente ? CRUCE : VERDE;
    else if (fase == CRUCE && transcurrido >= 8000)
      siguiente = VERDE;

    if (siguiente != fase) {
      fase = siguiente;
      inicio = ahora;

      switch (fase) {
        case VERDE:
          puts("VERDE: 5 s");
          break;
        case AMARILLO:
          puts("AMARILLO: 2 s");
          break;
        case ROJO:
          puts("ROJO: 5 s");
          break;
        case CRUCE:
          pendiente = 0;
          puts("CRUCE: rojo 8 s e indicador parpadeando.");
          break;
      }
      fflush(stdout);
    }

    int valores[] = {
      fase == ROJO || fase == CRUCE,
      fase == AMARILLO,
      fase == VERDE,
      fase == CRUCE && ((ahora - inicio) / 500) % 2 == 0
    };

    /*
     * Apagar primero las salidas que cambian a 0.
     * Luego encender las que cambian a 1.
     * Evita encender dos luces vehiculares al cambiar de fase.
     */
    for (int nivel = 0; nivel <= 1; nivel++) {
      for (int i = 0; i < 4; i++) {
        if (valores[i] == nivel && valores[i] != anteriores[i]) {
          if (gpiod_line_request_set_value(
                solicitud, leds[i],
                nivel ? GPIOD_LINE_VALUE_ACTIVE
                      : GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("Actualizar LED");
            goto limpiar;
          }
          anteriores[i] = valores[i];
        }
      }
    }

    if (nanosleep(&pausa, NULL) < 0) {
      if (errno == EINTR)
        continue;
      perror("Pausa");
      goto limpiar;
    }
  }

  resultado = EXIT_SUCCESS;

limpiar:
  if (solicitud) {
    for (int i = 0; i < 4; i++) {
      if (gpiod_line_request_set_value(
            solicitud, leds[i], GPIOD_LINE_VALUE_INACTIVE) < 0) {
        perror("Apagar LED");
        resultado = EXIT_FAILURE;
      }
    }
    gpiod_line_request_release(solicitud);
  }

  if (config)
    gpiod_line_config_free(config);
  if (ajustes)
    gpiod_line_settings_free(ajustes);
  if (chip)
    gpiod_chip_close(chip);

  return resultado;
}
