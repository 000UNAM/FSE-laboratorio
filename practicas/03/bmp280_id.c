#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_ADDR 0x76
#define REG_ID   0xD0

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

  unsigned char reg = REG_ID;

  if (write(fd, &reg, 1) != 1) {
    perror("write");
    close(fd);
    return 1;
  }

  unsigned char chip_id;

  if (read(fd, &chip_id, 1) != 1) {
    perror("read");
    close(fd);
    return 1;
  }

  printf("Chip ID leido: 0x%02X\n", chip_id);

  if (chip_id == 0x58) {
    printf("Sensor identificado: BMP280\n");
  }
  else if (chip_id == 0x60) {
     printf("Sensor identificado: BME280\n");
  }
  else {
    printf("Sensor desconocido\n");
}

  close(fd);
  return 0;
}
