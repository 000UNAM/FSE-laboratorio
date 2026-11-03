#define _POSIX_C_SOURCE 200809L

#include "bme280.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

/*
 * Modulo BME280 del proyecto final.
 * Elaborado con asistencia de IA.
 * Referencias: hoja de datos Bosch BME280 e interfaz i2c-dev de Linux.
 */

enum {
  REG_CALIB_TP = 0x88,
  REG_CALIB_H1 = 0xA1,
  REG_ID = 0xD0,
  REG_RESET = 0xE0,
  REG_CALIB_H = 0xE1,
  REG_CTRL_HUM = 0xF2,
  REG_STATUS = 0xF3,
  REG_CTRL_MEAS = 0xF4,
  REG_CONFIG = 0xF5,
  REG_DATOS = 0xF7,

  ID_BME280 = 0x60,
  RESET_BME280 = 0xB6,
  HUM_X1 = 0x01,
  MEDICION_X1_FORZADA = 0x25,

  STATUS_COPIANDO = 0x01,
  STATUS_MIDIENDO = 0x08
};

/* Estado privado: conexion y coeficientes propios de cada sensor. */
struct bme280 {
  int fd;
  uint8_t direccion;

  uint16_t t1, p1;
  int16_t t2, t3;
  int16_t p2, p3, p4, p5, p6, p7, p8, p9;

  uint8_t h1, h3;
  int16_t h2, h4, h5;
  int8_t h6;
};

/* Suspende el proceso sin mantener la CPU ocupada. */
static int esperar_ms(unsigned int ms)
{
  struct timespec pausa = {
    .tv_sec = (time_t)(ms / 1000U),
    .tv_nsec = (long)(ms % 1000U) * 1000000L
  };

  while (nanosleep(&pausa, &pausa) < 0) {
    if (errno != EINTR) {
      return -1;
    }
  }

  return 0;
}

/*
 * Una transaccion combinada:
 * 1. Indicar el registro inicial.
 * 2. Leer los bytes mediante un inicio repetido de I2C.
 */
static int leer_regs(bme280 *sensor, uint8_t reg,
                     uint8_t *datos, uint16_t cantidad)
{
  struct i2c_msg mensajes[2] = {
    {
      .addr = sensor->direccion,
      .flags = 0,
      .len = 1,
      .buf = &reg
    },
    {
      .addr = sensor->direccion,
      .flags = I2C_M_RD,
      .len = cantidad,
      .buf = datos
    }
  };

  struct i2c_rdwr_ioctl_data transferencia = {
    .msgs = mensajes,
    .nmsgs = 2
  };

  int resultado = ioctl(sensor->fd, I2C_RDWR, &transferencia);

  if (resultado < 0) {
    return -1;
  }

  if (resultado != 2) {
    errno = EIO;
    return -1;
  }

  return 0;
}

/* Escribir la direccion de un registro seguida de su valor. */
static int escribir_reg(bme280 *sensor, uint8_t reg, uint8_t valor)
{
  uint8_t datos[2] = {reg, valor};

  struct i2c_msg mensaje = {
    .addr = sensor->direccion,
    .flags = 0,
    .len = sizeof datos,
    .buf = datos
  };

  struct i2c_rdwr_ioctl_data transferencia = {
    .msgs = &mensaje,
    .nmsgs = 1
  };

  int resultado = ioctl(sensor->fd, I2C_RDWR, &transferencia);

  if (resultado < 0) {
    return -1;
  }

  if (resultado != 1) {
    errno = EIO;
    return -1;
  }

  return 0;
}

/* Combina dos bytes: primero viene el menos significativo. */
static uint16_t convertir_u16(const uint8_t *bytes)
{
  return (uint16_t)((uint16_t)bytes[0] |
                    ((uint16_t)bytes[1] << 8));
}

/* Interpreta un entero de 16 bits con signo sin perder negativos. */
static int16_t convertir_s16(const uint8_t *bytes)
{
  int32_t valor = convertir_u16(bytes);

  if (valor >= 32768) {
    valor -= 65536;
  }

  return (int16_t)valor;
}

/* H4 y H5 tienen 12 bits con signo, repartidos entre registros. */
static int16_t convertir_s12(uint16_t valor)
{
  int32_t resultado = valor & 0x0FFF;

  if (resultado >= 2048) {
    resultado -= 4096;
  }

  return (int16_t)resultado;
}

/*
 * Espera a que se apaguen los bits indicados de STATUS.
 * Limita las consultas para evitar un ciclo infinito.
 */
static int esperar_estado(bme280 *sensor, uint8_t mascara)
{
  enum { MAX_CONSULTAS = 50, PAUSA_MS = 2 };

  for (int intento = 0; intento < MAX_CONSULTAS; intento++) {
    uint8_t estado;

    if (leer_regs(sensor, REG_STATUS, &estado, 1) < 0) {
      return -1;
    }

    if ((estado & mascara) == 0) {
      return 0;
    }

    if (esperar_ms(PAUSA_MS) < 0) {
      return -1;
    }
  }

  errno = ETIMEDOUT;
  return -1;
}

/* Lee los coeficientes de fabrica de este sensor. */
static int cargar_calibracion(bme280 *sensor)
{
  uint8_t tp[24];
  uint8_t h[7];

  if (leer_regs(sensor, REG_CALIB_TP, tp, sizeof tp) < 0 ||
      leer_regs(sensor, REG_CALIB_H1, &sensor->h1, 1) < 0 ||
      leer_regs(sensor, REG_CALIB_H, h, sizeof h) < 0) {
    return -1;
  }

  sensor->t1 = convertir_u16(&tp[0]);
  sensor->t2 = convertir_s16(&tp[2]);
  sensor->t3 = convertir_s16(&tp[4]);

  sensor->p1 = convertir_u16(&tp[6]);
  sensor->p2 = convertir_s16(&tp[8]);
  sensor->p3 = convertir_s16(&tp[10]);
  sensor->p4 = convertir_s16(&tp[12]);
  sensor->p5 = convertir_s16(&tp[14]);
  sensor->p6 = convertir_s16(&tp[16]);
  sensor->p7 = convertir_s16(&tp[18]);
  sensor->p8 = convertir_s16(&tp[20]);
  sensor->p9 = convertir_s16(&tp[22]);

  sensor->h2 = convertir_s16(&h[0]);
  sensor->h3 = h[2];

  sensor->h4 = convertir_s12(
    ((uint16_t)h[3] << 4) | (h[4] & 0x0F)
  );

  sensor->h5 = convertir_s12(
    ((uint16_t)h[5] << 4) | (h[4] >> 4)
  );

  sensor->h6 = (int8_t)(
    h[6] < 128 ? (int)h[6] : (int)h[6] - 256
  );

  /* P1 interviene en el divisor de la compensacion de presion. */
  if (sensor->p1 == 0 || sensor->t1 == 0) {
    errno = EIO;
    return -1;
  }

  return 0;
}

bme280 *bme280_abrir(const char *bus, uint8_t direccion)
{
  if (bus == NULL ||
      (direccion != 0x76 && direccion != 0x77)) {
    errno = EINVAL;
    return NULL;
  }

  bme280 *sensor = calloc(1, sizeof *sensor);

  if (sensor == NULL) {
    return NULL;
  }

  sensor->direccion = direccion;
  sensor->fd = open(bus, O_RDWR | O_CLOEXEC);

  if (sensor->fd < 0) {
    int error_original = errno;
    free(sensor);
    errno = error_original;
    return NULL;
  }

  /*
   * Selecciona la direccion sin forzar el acceso si un
   * controlador del kernel ya tiene ocupado el dispositivo.
   */
  if (ioctl(sensor->fd, I2C_SLAVE, direccion) < 0) {
    goto fallo;
  }

  uint8_t identificador;

  if (leer_regs(sensor, REG_ID, &identificador, 1) < 0) {
    goto fallo;
  }

  if (identificador != ID_BME280) {
    errno = ENODEV;
    goto fallo;
  }

  /*
   * Reinicio para partir de una configuracion conocida.
   * Esperamos antes de consultar la copia de calibracion.
   */
  if (escribir_reg(sensor, REG_RESET, RESET_BME280) < 0 ||
      esperar_ms(5) < 0 ||
      esperar_estado(sensor, STATUS_COPIANDO) < 0 ||
      cargar_calibracion(sensor) < 0) {
    goto fallo;
  }

  /*
   * Sin filtro y humedad x1.
   * CTRL_HUM se aplica al escribir despues CTRL_MEAS,
   * lo cual haremos al solicitar la medicion.
   */
  if (escribir_reg(sensor, REG_CONFIG, 0x00) < 0 ||
      escribir_reg(sensor, REG_CTRL_HUM, HUM_X1) < 0) {
    goto fallo;
  }

  return sensor;

fallo:
  {
    /* Conserva el error que origino el fallo. */
    int error_original = errno;

    if (close(sensor->fd) < 0) {
      perror("cerrar I2C tras fallo de inicializacion");
    }

    free(sensor);
    errno = error_original;
    return NULL;
  }
}

int bme280_cerrar(bme280 *sensor)
{
  if (sensor == NULL) {
    errno = EINVAL;
    return -1;
  }

  int resultado = close(sensor->fd);
  int error_cierre = errno;

  free(sensor);

  if (resultado < 0) {
    errno = error_cierre;
  }

  return resultado;
}

/* Reconstruye un dato de 20 bits a partir de tres registros. */
static int32_t convertir_adc20(const uint8_t *datos)
{
  return ((int32_t)datos[0] << 12) |
         ((int32_t)datos[1] << 4) |
         ((int32_t)datos[2] >> 4);
}

/*
 * Compensacion basada en el apendice 8.1 de Bosch.
 * Solo entrega resultados cuando el conjunto es valido.
 */
static int compensar(const bme280 *s, const uint8_t *datos,
                     bme280_medicion *salida)
{
  enum {
    ADC20_NO_DISPONIBLE = 0x80000,
    ADC16_NO_DISPONIBLE = 0x8000
  };

  int32_t adc_p = convertir_adc20(&datos[0]);
  int32_t adc_t = convertir_adc20(&datos[3]);
  int32_t adc_h = ((int32_t)datos[6] << 8) | datos[7];

  if (adc_t == ADC20_NO_DISPONIBLE ||
      adc_p == ADC20_NO_DISPONIBLE ||
      adc_h == ADC16_NO_DISPONIBLE) {
    errno = EIO;
    return -1;
  }

  /* Temperatura y valor auxiliar para las otras compensaciones. */
  double a = (adc_t / 16384.0 - s->t1 / 1024.0) * s->t2;
  double b = adc_t / 131072.0 - s->t1 / 8192.0;
  b = b * b * s->t3;

  double temperatura = (a + b) / 5120.0;
  int32_t t_fine = (int32_t)(a + b);

  /* Presion: el resultado de esta formula esta en pascales. */
  a = t_fine / 2.0 - 64000.0;
  b = a * a * s->p6 / 32768.0;
  b += a * s->p5 * 2.0;
  b = b / 4.0 + s->p4 * 65536.0;

  a = (s->p3 * a * a / 524288.0 + s->p2 * a) / 524288.0;
  a = (1.0 + a / 32768.0) * s->p1;

  if (a == 0.0) {
    errno = ERANGE;
    return -1;
  }

  double presion = 1048576.0 - adc_p;
  presion = (presion - b / 4096.0) * 6250.0 / a;

  a = s->p9 * presion * presion / 2147483648.0;
  b = presion * s->p8 / 32768.0;
  presion += (a + b + s->p7) / 16.0;

  /* Humedad relativa, expresada como porcentaje. */
  double ajuste = (double)t_fine - 76800.0;

  double humedad =
    (adc_h - (s->h4 * 64.0 + s->h5 * ajuste / 16384.0)) *
    (s->h2 / 65536.0 *
     (1.0 + s->h6 * ajuste / 67108864.0 *
      (1.0 + s->h3 * ajuste / 67108864.0)));

  humedad *= 1.0 - s->h1 * humedad / 524288.0;

  if (!isfinite(temperatura) ||
      !isfinite(presion) ||
      !isfinite(humedad)) {
    errno = ERANGE;
    return -1;
  }

  /* Limites de humedad utilizados por la formula de Bosch. */
  if (humedad < 0.0) {
    humedad = 0.0;
  } else if (humedad > 100.0) {
    humedad = 100.0;
  }

  /* Rechaza temperatura o presion fuera del rango del sensor. */
  if (temperatura < -40.0 || temperatura > 85.0 ||
      presion < 30000.0 || presion > 110000.0) {
    errno = ERANGE;
    return -1;
  }

  bme280_medicion medicion = {
    .temperatura_c = temperatura,
    .humedad_pct = humedad,
    .presion_hpa = presion / 100.0
  };

  *salida = medicion;
  return 0;
}

int bme280_leer(bme280 *sensor, bme280_medicion *salida)
{
  enum { ESPERA_CONVERSION_MS = 20, NUM_REGISTROS = 8 };

  if (sensor == NULL || salida == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* Comprueba que no quede una conversion anterior en curso. */
  if (esperar_estado(sensor, STATUS_MIDIENDO | STATUS_COPIANDO) < 0) {
    return -1;
  }

  /*
   * Decision de energia para documentar en la bitacora:
   * modo forzado, sobremuestreo x1 y reposo entre solicitudes.
   * El programa principal determinara el intervalo de muestreo.
   */
  if (escribir_reg(sensor, REG_CTRL_MEAS,
                   MEDICION_X1_FORZADA) < 0) {
    return -1;
  }

  /*
   * Margen para completar la conversion con x1 en las tres
   * variables. Despues comprobamos tambien el estado.
   */
  if (esperar_ms(ESPERA_CONVERSION_MS) < 0 ||
      esperar_estado(sensor, STATUS_MIDIENDO | STATUS_COPIANDO) < 0) {
    return -1;
  }

  /* Lee presion, temperatura y humedad en una sola transferencia. */
  uint8_t datos[NUM_REGISTROS];

  if (leer_regs(sensor, REG_DATOS, datos, sizeof datos) < 0) {
    return -1;
  }

  return compensar(sensor, datos, salida);
}
