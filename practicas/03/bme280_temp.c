#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_ADDR 0x76

static int leer_regs(int fd, uint8_t reg, uint8_t *datos, int cantidad) {
  if (write(fd, &reg, 1) != 1) {
    return -1;
  }

  if (read(fd, datos, cantidad) != cantidad) {
    return -1;
  }

  return 0;
}

static int escribir_reg(int fd, uint8_t reg, uint8_t valor) {
  uint8_t datos[2] = {reg, valor};

  if (write(fd, datos, 2) != 2) {
    return -1;
  }

  return 0;
}

int main(void) {
  int fd = open("/dev/i2c-1", O_RDWR);

  if (fd < 0) {
    perror("open");
    return 1;
  }

  if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
    perror("ioctl I2C_SLAVE");
    close(fd);
    return 1;
  }

  /*
   * Coeficientes de calibracion de temperatura:
   * dig_T1 -> 0x88 / 0x89
   * dig_T2 -> 0x8A / 0x8B
   * dig_T3 -> 0x8C / 0x8D
   */
  uint8_t calib[6];

  if (leer_regs(fd, 0x88, calib, 6) < 0) {
    perror("leer calibracion");
    close(fd);
    return 1;
  }

  uint16_t dig_T1 =
    ((uint16_t)calib[1] << 8) | calib[0];

  int16_t dig_T2 =
    (int16_t)(((uint16_t)calib[3] << 8) | calib[2]);

  int16_t dig_T3 =
    (int16_t)(((uint16_t)calib[5] << 8) | calib[4]);

  printf("Coeficientes de calibracion:\n");
  printf("dig_T1 = %u\n", dig_T1);
  printf("dig_T2 = %d\n", dig_T2);
  printf("dig_T3 = %d\n", dig_T3);

  /*
   * ctrl_meas = 0x27
   *
   * Temperatura: oversampling x1
   * Presion:     oversampling x1
   * Modo:        normal
   */
  if (escribir_reg(fd, 0xF4, 0x27) < 0) {
    perror("configurar sensor");
    close(fd);
    return 1;
  }

  printf("\nLeyendo temperatura cada 2 segundos...\n");
  printf("Ctrl+C para terminar.\n\n");

  while (1) {
    uint8_t datos[3];

    /*
     * Temperatura:
     * 0xFA -> MSB
     * 0xFB -> LSB
     * 0xFC -> XLSB
     */
    if (leer_regs(fd, 0xFA, datos, 3) < 0) {
      perror("leer temperatura");
      break;
    }

    int32_t adc_T =
      ((int32_t)datos[0] << 12) |
      ((int32_t)datos[1] << 4) |
      ((int32_t)datos[2] >> 4);

    /*
     * Formula de compensacion del datasheet BME280.
     */
    double var1 =
      ((adc_T / 16384.0) -
       (dig_T1 / 1024.0)) * dig_T2;

    double var2 =
      ((adc_T / 131072.0) -
       (dig_T1 / 8192.0));

    var2 =
      var2 * var2 * dig_T3;

    double t_fine = var1 + var2;
    double temperatura = t_fine / 5120.0;

    printf("ADC_T: %ld   Temperatura: %.2f °C\n",
           (long)adc_T, temperatura);

    sleep(2);
  }

  close(fd);
  return 0;
}
