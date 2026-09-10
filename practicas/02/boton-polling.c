#define _POSIX_C_SOURCE 200809L

#include <gpiod.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static volatile sig_atomic_t detener = 0;

static void al_interrumpir(int senal)
{
  (void)senal;
  detener = 1;
}

int main(void)
{
  const unsigned int boton = 5;
  const unsigned int led = 17;
  struct gpiod_chip *chip = NULL;
  struct gpiod_line_settings *ajustes = NULL;
  struct gpiod_line_config *config = NULL;
  struct gpiod_line_request *solicitud = NULL;
  int resultado = EXIT_FAILURE;

  struct sigaction accion = {0};
  accion.sa_handler = al_interrumpir;
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

  /* Entrada con la resistencia pull-down externa del circuito. */
  if (gpiod_line_settings_set_direction(
        ajustes, GPIOD_LINE_DIRECTION_INPUT) < 0 ||
      gpiod_line_settings_set_bias(
        ajustes, GPIOD_LINE_BIAS_DISABLED) < 0 ||
      gpiod_line_config_add_line_settings(
        config, &boton, 1, ajustes) < 0) {
    perror("Configurar boton");
    goto limpiar;
  }

  /* La configuracion anterior ya guardo los ajustes del boton. */
  gpiod_line_settings_reset(ajustes);

  if (gpiod_line_settings_set_direction(
        ajustes, GPIOD_LINE_DIRECTION_OUTPUT) < 0 ||
      gpiod_line_settings_set_output_value(
        ajustes, GPIOD_LINE_VALUE_INACTIVE) < 0 ||
      gpiod_line_config_add_line_settings(
        config, &led, 1, ajustes) < 0) {
    perror("Configurar LED");
    goto limpiar;
  }

  solicitud = gpiod_chip_request_lines(chip, NULL, config);
  if (!solicitud) {
    perror("Solicitar GPIO");
    goto limpiar;
  }

  puts("Polling: boton GPIO5 controla LED GPIO17.");
  puts("Presiona y suelta el boton. Ctrl+C para terminar.");
  fflush(stdout);

  /* Sin pausas: queremos medir el costo del polling activo. */
  while (!detener) {
    int estado = gpiod_line_request_get_value(solicitud, boton);

    if (estado < 0) {
      if (detener && errno == EINTR)
        break;
      perror("Leer boton");
      goto limpiar;
    }

    if (gpiod_line_request_set_value(
          solicitud, led, (enum gpiod_line_value)estado) < 0) {
      if (detener && errno == EINTR)
        break;
      perror("Actualizar LED");
      goto limpiar;
    }
  }

  resultado = EXIT_SUCCESS;

limpiar:
  if (solicitud) {
    if (gpiod_line_request_set_value(
          solicitud, led, GPIOD_LINE_VALUE_INACTIVE) < 0) {
      perror("Apagar LED");
      resultado = EXIT_FAILURE;
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
