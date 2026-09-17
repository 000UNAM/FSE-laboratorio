#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>

#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>
#include <gpiod.h>

#define I2C_ADDR 0x76
#define LED_GPIO 17

/* =========================================================
 * I2C - BME280
 * ========================================================= */

static int leer_regs(int fd, uint8_t reg,
                     uint8_t *datos, int cantidad) {
  if (write(fd, &reg, 1) != 1) {
    return -1;
  }

  if (read(fd, datos, cantidad) != cantidad) {
    return -1;
  }

  return 0;
}

static int escribir_reg(int fd,
                        uint8_t reg,
                        uint8_t valor) {
  uint8_t datos[2] = {reg, valor};

  if (write(fd, datos, 2) != 2) {
    return -1;
  }

  return 0;
}

static double leer_temperatura(
  int fd,
  uint16_t dig_T1,
  int16_t dig_T2,
  int16_t dig_T3
) {
  uint8_t datos[3];

  if (leer_regs(fd, 0xFA, datos, 3) < 0) {
    return -999.0;
  }

  int32_t adc_T =
    ((int32_t)datos[0] << 12) |
    ((int32_t)datos[1] << 4) |
    ((int32_t)datos[2] >> 4);

  double var1 =
    ((adc_T / 16384.0) -
     (dig_T1 / 1024.0)) * dig_T2;

  double var2 =
    ((adc_T / 131072.0) -
     (dig_T1 / 8192.0));

  var2 = var2 * var2 * dig_T3;

  double t_fine = var1 + var2;

  return t_fine / 5120.0;
}

/* =========================================================
 * SPI - MCP3008
 * ========================================================= */

static int leer_potenciometro(int fd,
                              unsigned int speed) {
  uint8_t canal = 0;

  uint8_t tx[3] = {
    0x01,
    (8 + canal) << 4,
    0x00
  };

  uint8_t rx[3] = {0};

  struct spi_ioc_transfer tr = {
    .tx_buf = (uintptr_t)tx,
    .rx_buf = (uintptr_t)rx,
    .len = 3,
    .speed_hz = speed,
    .bits_per_word = 8
  };

  if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
    return -1;
  }

  return ((rx[1] & 0x03) << 8) | rx[2];
}

/* =========================================================
 * MAIN
 * ========================================================= */

int main(void) {
  /* ---------- BME280 ---------- */

  int fd_i2c = open("/dev/i2c-1", O_RDWR);

  if (fd_i2c < 0) {
    perror("open I2C");
    return 1;
  }

  if (ioctl(fd_i2c, I2C_SLAVE, I2C_ADDR) < 0) {
    perror("I2C_SLAVE");
    close(fd_i2c);
    return 1;
  }

  uint8_t calib[6];

  if (leer_regs(fd_i2c, 0x88, calib, 6) < 0) {
    perror("calibracion BME280");
    close(fd_i2c);
    return 1;
  }

  uint16_t dig_T1 =
    ((uint16_t)calib[1] << 8) | calib[0];

  int16_t dig_T2 =
    (int16_t)(((uint16_t)calib[3] << 8) |
              calib[2]);

  int16_t dig_T3 =
    (int16_t)(((uint16_t)calib[5] << 8) |
              calib[4]);

  if (escribir_reg(fd_i2c, 0xF4, 0x27) < 0) {
    perror("configurar BME280");
    close(fd_i2c);
    return 1;
  }

  usleep(100000);

  /* ---------- MCP3008 ---------- */

  int fd_spi = open("/dev/spidev0.0", O_RDWR);

  if (fd_spi < 0) {
    perror("open SPI");
    close(fd_i2c);
    return 1;
  }

  uint8_t mode = SPI_MODE_0;
  uint8_t bits = 8;
  unsigned int speed = 1000000;

  if (ioctl(fd_spi,
            SPI_IOC_WR_MODE,
            &mode) < 0) {
    perror("SPI mode");
    return 1;
  }

  if (ioctl(fd_spi,
            SPI_IOC_WR_BITS_PER_WORD,
            &bits) < 0) {
    perror("SPI bits");
    return 1;
  }

  if (ioctl(fd_spi,
            SPI_IOC_WR_MAX_SPEED_HZ,
            &speed) < 0) {
    perror("SPI speed");
    return 1;
  }

  /* ---------- GPIO17 / LED ---------- */

  struct gpiod_chip *chip =
    gpiod_chip_open("/dev/gpiochip0");

  if (!chip) {
    perror("gpiod_chip_open");
    return 1;
  }

  struct gpiod_line_settings *settings =
    gpiod_line_settings_new();

  struct gpiod_line_config *line_config =
    gpiod_line_config_new();

  struct gpiod_request_config *request_config =
    gpiod_request_config_new();

  if (!settings ||
      !line_config ||
      !request_config) {
    fprintf(stderr,
            "Error creando configuracion GPIO\n");
    return 1;
  }

  gpiod_line_settings_set_direction(
    settings,
    GPIOD_LINE_DIRECTION_OUTPUT
  );

  gpiod_line_settings_set_output_value(
    settings,
    GPIOD_LINE_VALUE_INACTIVE
  );

  unsigned int offset = LED_GPIO;

  if (gpiod_line_config_add_line_settings(
        line_config,
        &offset,
        1,
        settings) < 0) {
    fprintf(stderr,
            "Error agregando GPIO17\n");
    return 1;
  }

  gpiod_request_config_set_consumer(
    request_config,
    "sensor_combinado"
  );

  struct gpiod_line_request *request =
    gpiod_chip_request_lines(
      chip,
      request_config,
      line_config
    );

  if (!request) {
    perror("gpiod_chip_request_lines");
    return 1;
  }

  printf("=====================================\n");
  printf("   SENSOR COMBINADO - PRACTICA 3\n");
  printf("=====================================\n");
  printf("BME280   : I2C 0x76\n");
  printf("MCP3008  : SPI canal 0\n");
  printf("LED      : GPIO17\n");
  printf("Umbral   : 0 - 50 grados C\n");
  printf("Ctrl+C para terminar\n\n");

  double temperatura = 0.0;

  /*
   * Cada iteracion tarda 300 ms.
   * El pot se lee continuamente.
   * La temperatura se actualiza aproximadamente
   * cada 2.1 segundos.
   */
  int contador = 7;

  while (1) {
    int valor_pot =
      leer_potenciometro(fd_spi, speed);

    if (valor_pot < 0) {
      perror("leer MCP3008");
      break;
    }

    double umbral =
      (valor_pot * 50.0) / 1023.0;

    if (contador >= 7) {
      temperatura =
        leer_temperatura(
          fd_i2c,
          dig_T1,
          dig_T2,
          dig_T3
        );

      if (temperatura < -100.0) {
        fprintf(stderr,
                "Error leyendo temperatura\n");
        break;
      }

      contador = 0;
    }

    enum gpiod_line_value led;

    if (temperatura > umbral) {
      led = GPIOD_LINE_VALUE_ACTIVE;
    } else {
      led = GPIOD_LINE_VALUE_INACTIVE;
    }

    if (gpiod_line_request_set_value(
          request,
          LED_GPIO,
          led) < 0) {
      perror("GPIO LED");
      break;
    }

    printf(
      "Temp: %5.2f C | "
      "Pot: %4d | "
      "Umbral: %5.2f C | "
      "LED: %s\n",
      temperatura,
      valor_pot,
      umbral,
      led == GPIOD_LINE_VALUE_ACTIVE
        ? "ENCENDIDO"
        : "APAGADO"
    );

    fflush(stdout);

    contador++;
    usleep(300000);
  }

  gpiod_line_request_set_value(
    request,
    LED_GPIO,
    GPIOD_LINE_VALUE_INACTIVE
  );

  gpiod_line_request_release(request);
  gpiod_request_config_free(request_config);
  gpiod_line_config_free(line_config);
  gpiod_line_settings_free(settings);
  gpiod_chip_close(chip);

  close(fd_spi);
  close(fd_i2c);

  return 0;
}
