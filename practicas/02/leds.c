#define _POSIX_C_SOURCE 200809L

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_LEDS 4

int main(void)
{
  const unsigned int pines[N_LEDS] = {17, 27, 22, 23};
  const enum gpiod_line_value apagados[N_LEDS] = {
    GPIOD_LINE_VALUE_INACTIVE, GPIOD_LINE_VALUE_INACTIVE,
    GPIOD_LINE_VALUE_INACTIVE, GPIOD_LINE_VALUE_INACTIVE
  };
  const struct timespec pausa = {
    .tv_sec = 0,
    .tv_nsec = 150000000
  };

  struct gpiod_chip *chip = NULL;
  struct gpiod_line_settings *ajustes = NULL;
  struct gpiod_line_config *config = NULL;
  struct gpiod_line_request *solicitud = NULL;
  int resultado = EXIT_FAILURE;

  /* Abrir el controlador GPIO. */
  chip = gpiod_chip_open("/dev/gpiochip0");
  if (!chip) {
    perror("Abrir gpiochip0");
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

  /* Configurar las cuatro salidas inicialmente apagadas. */
  if (gpiod_line_settings_set_direction(
        ajustes, GPIOD_LINE_DIRECTION_OUTPUT) < 0 ||
      gpiod_line_settings_set_output_value(
        ajustes, GPIOD_LINE_VALUE_INACTIVE) < 0) {
    perror("Configurar salidas");
    goto limpiar;
  }

  if (gpiod_line_config_add_line_settings(
        config, pines, N_LEDS, ajustes) < 0) {
    perror("Agregar pines");
    goto limpiar;
  }

  solicitud = gpiod_chip_request_lines(chip, NULL, config);
  if (!solicitud) {
    perror("Solicitar GPIO");
    goto limpiar;
  }

  puts("Secuencia: GPIO17 -> GPIO27 -> GPIO22 -> GPIO23");

  /* Encender y apagar cada LED antes de pasar al siguiente. */
  for (int ronda = 1; ronda <= 5; ronda++) {
    printf("Ronda %d/5\n", ronda);

    for (int i = 0; i < N_LEDS; i++) {
      if (gpiod_line_request_set_value(
            solicitud, pines[i], GPIOD_LINE_VALUE_ACTIVE) < 0) {
        perror("Encender LED");
        goto limpiar;
      }

      if (nanosleep(&pausa, NULL) < 0) {
        perror("Esperar");
        goto limpiar;
      }

      if (gpiod_line_request_set_value(
            solicitud, pines[i], GPIOD_LINE_VALUE_INACTIVE) < 0) {
        perror("Apagar LED");
        goto limpiar;
      }
    }
  }

  resultado = EXIT_SUCCESS;

limpiar:
  /* Intentar apagar todas las salidas antes de liberarlas. */
  if (solicitud) {
    if (gpiod_line_request_set_values(solicitud, apagados) < 0) {
      perror("Apagar salidas al terminar");
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

  if (resultado == EXIT_SUCCESS)
    puts("Finalizado: cinco rondas completadas.");

  return resultado;
}
