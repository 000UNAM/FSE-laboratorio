# 🔧 Fundamentos de Sistemas Embebidos

![C](https://img.shields.io/badge/C-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Raspberry Pi](https://img.shields.io/badge/Raspberry%20Pi-%23A22846.svg?style=for-the-badge&logo=raspberrypi&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-%23FCC624.svg?style=for-the-badge&logo=linux&logoColor=black)
![Git](https://img.shields.io/badge/Git-%23F05032.svg?style=for-the-badge&logo=git&logoColor=white)
![GitHub](https://img.shields.io/badge/GitHub-%23181717.svg?style=for-the-badge&logo=github&logoColor=white)

# 🚀 Objetivo

Aplicar conceptos fundamentales de sistemas embebidos utilizando una **Raspberry Pi 4** como plataforma de desarrollo.

A lo largo de las prácticas se trabaja con:

- Administración básica de Raspberry Pi OS.
- Programación en C.
- GPIO digital.
- Entradas y salidas.
- Polling y eventos.
- Protocolos de comunicación I2C y SPI.
- Sensores externos.
- Conversión analógico-digital.
- Control de versiones con Git.
- Desarrollo y prueba de sistemas que interactúan con hardware real.

---

# 📁 Estructura del repositorio

```text
FSE-laboratorio/
└── practicas/
    ├── 01/
    │   ├── reporte-sistema.c
    │   └── sysinfo.sh
    │
    ├── 02/
    │   ├── leds.c
    │   ├── boton-polling.c
    │   └── semaforo.c
    │
    └── 03/
        ├── bmp280_id.c
        ├── bme280_temp.c
        ├── leer_pot.c
        └── sensor_combinado.c
````

Cada directorio corresponde a una práctica y contiene principalmente los **programas fuente desarrollados en C o Bash**.

Los archivos ejecutables generados durante la compilación no forman parte del repositorio.

---

# 🧪 Prácticas

## 📌 Práctica 1 — Introducción al entorno de la Raspberry Pi

Primera aproximación al entorno de desarrollo utilizado durante el curso.

### Temas trabajados

* Acceso remoto mediante SSH.
* Navegación y administración básica en Linux.
* Identificación del sistema.
* Uso de herramientas de línea de comandos.
* Programación básica en C.
* Obtención de información del sistema.
* Organización del directorio de prácticas.
* Primer uso de Git.

### Programas principales

```text
reporte-sistema.c
sysinfo.sh
```

---

## 💡 Práctica 2 — GPIO: entradas y salidas digitales

Uso de las líneas **GPIO de la Raspberry Pi** desde terminal y desde programas
en C mediante **libgpiod**.

### Temas trabajados

* Numeración GPIO y pines físicos.
* Control de LEDs.
* Lectura de botones.
* Uso de resistencias pull-down.
* Programación con libgpiod.
* Polling.
* Eventos.
* Uso de CPU.
* Antirrebote.
* Máquina de estados.
* Semáforo peatonal.

### Programas principales

```text
leds.c
boton-polling.c
semaforo.c
```

### 🎥 Evidencias en video

**Secuencia de LEDs**

[https://youtube.com/shorts/u49dvJY_IYM](https://youtube.com/shorts/u49dvJY_IYM)

**Semáforo peatonal**

[https://youtube.com/shorts/6uxbRbtLUmI](https://youtube.com/shorts/6uxbRbtLUmI)

---

## 🌡️ Práctica 3 — I2C y SPI: conversión analógica y sensores

Integración de distintos protocolos de comunicación utilizados comúnmente
en sistemas embebidos.

La Raspberry Pi no dispone de un convertidor analógico-digital nativo, por
lo que se utilizó un **MCP3008** para adquirir una señal analógica proveniente
de un potenciómetro.

También se utilizó un sensor **BME280** conectado mediante I2C.

### Hardware utilizado

* Raspberry Pi 4.
* BME280.
* MCP3008.
* Potenciómetro de 10 kΩ.
* LED.
* Resistencia.
* Protoboard.
* Cables jumper.

---

### 🔌 I2C

El sensor BME280 se conecta utilizando el bus I2C:

```text
BME280        Raspberry Pi

VIN    ───►   3.3 V
GND    ───►   GND
SCL    ───►   GPIO3
SDA    ───►   GPIO2
```

El dispositivo fue detectado en:

```text
0x76
```

El registro de identificación permitió determinar que el módulo contenía
realmente un **BME280**, cuyo Chip ID es:

```text
0x60
```

### Programas

```text
bmp280_id.c
bme280_temp.c
```

`bmp280_id.c` identifica el dispositivo conectado al bus I2C.

`bme280_temp.c` obtiene los coeficientes de calibración almacenados en el
sensor y realiza la compensación necesaria para obtener la temperatura
ambiental.

---

### ⚡ SPI

Para convertir la posición del potenciómetro en un valor digital se utilizó
el ADC **MCP3008**, conectado mediante SPI.

```text
MCP3008              Raspberry Pi

CLK     ─────────►   GPIO11 / SCLK
DOUT    ─────────►   GPIO9  / MISO
DIN     ◄─────────   GPIO10 / MOSI
CS      ◄─────────   GPIO8  / CE0
```

El MCP3008 es un convertidor de **10 bits**, por lo que proporciona valores
entre:

```text
0 ───────────────────────── 1023
0 V                         3.3 V
```

### Programa

```text
leer_pot.c
```

La lectura obtenida se convierte a voltaje mediante:

```text
V = (ADC × 3.3) / 1023
```

---

## 🔀 Sistema combinado

El programa:

```text
sensor_combinado.c
```

integra **I2C + SPI + GPIO**.

```text
                    ┌───────────────┐
 BME280 ─── I2C ───►│               │
                    │ Raspberry Pi  │──── GPIO ───► LED
 MCP3008 ── SPI ───►│               │
    ▲               └───────────────┘
    │
Potenciómetro
```

El BME280 obtiene la temperatura ambiental.

El MCP3008 convierte la posición del potenciómetro en un valor de 10 bits.

Posteriormente el valor del ADC se transforma en un umbral entre:

```text
0 °C ─────────────────────── 50 °C
```

mediante:

```text
umbral = (ADC × 50) / 1023
```

El comportamiento final es:

```text
temperatura > umbral
        │
        └────► LED ENCENDIDO

temperatura <= umbral
        │
        └────► LED APAGADO
```

### 🎥 Video de funcionamiento

**Práctica 3 — I2C y SPI con BME280 y MCP3008 en Raspberry Pi 4**

[https://youtube.com/shorts/DRcxMAXERnQ](https://youtube.com/shorts/DRcxMAXERnQ)

---

# 🛠️ Compilación

Los programas están desarrollados para **Linux / Raspberry Pi OS**.

## Requisitos

```bash
sudo apt install build-essential
sudo apt install libgpiod-dev
sudo apt install i2c-tools
```

---

## Compilar programas básicos

Por ejemplo:

```bash
gcc practicas/03/bmp280_id.c -o bmp280_id
```

```bash
gcc practicas/03/bme280_temp.c -o bme280_temp
```

```bash
gcc practicas/03/leer_pot.c -o leer_pot
```

---

## Compilar con libgpiod

El programa combinado utiliza libgpiod:

```bash
gcc practicas/03/sensor_combinado.c \
    -o sensor_combinado \
    $(pkg-config --cflags --libs libgpiod)
```

---

# ▶️ Ejecución

Ejemplo:

```bash
./sensor_combinado
```

Salida aproximada:

```text
=====================================
   SENSOR COMBINADO - PRACTICA 3
=====================================

BME280   : I2C 0x76
MCP3008  : SPI canal 0
LED      : GPIO17
Umbral   : 0 - 50 grados C

Temp: 21.53 C | Pot: 483 | Umbral: 23.61 C | LED: APAGADO
Temp: 21.53 C | Pot: 430 | Umbral: 21.02 C | LED: ENCENDIDO
Temp: 21.54 C | Pot: 501 | Umbral: 24.49 C | LED: APAGADO
```

---

# 🔄 Flujo de trabajo con Git

El repositorio conserva el historial de cada práctica.

```text
practica-01
     ↓
Reorganiza repositorio con raiz en /unam/fse
     ↓
practica-02
     ↓
practica-03
```

Para consultar el historial:

```bash
git log --oneline
```

Para verificar los archivos modificados:

```bash
git status
```

Para sincronizar los cambios:

```bash
git add <archivo>
git commit -m "descripcion"
git push
```

---

# 🧠 Conceptos estudiados

Durante las prácticas se han aplicado conceptos como:

* Sistemas embebidos.
* Linux embebido.
* GPIO.
* Entradas y salidas digitales.
* ADC.
* Polling.
* Eventos.
* Interrupciones a nivel de kernel.
* I2C.
* SPI.
* Sensores digitales.
* Conversión analógico-digital.
* Programación en C.
* Manejo de dispositivos mediante `/dev`.
* Interfaces del kernel de Linux.
* Control de versiones.

---

# 🚧 Desarrollo

El repositorio continuará creciendo conforme se desarrollen nuevas prácticas
de la asignatura.

Cada nueva práctica deberá mantener la estructura:

```text
practicas/
└── NN/
```

donde `NN` corresponde al número de práctica.

Se busca mantener:

* Código fuente legible.
* Separación entre código y binarios compilados.
* Commits identificables por práctica.
* Evidencias reproducibles.
* Documentación de conexiones y hardware.
* Uso consistente de Git.

---

# 📖 Referencias

* [Raspberry Pi Documentation](https://www.raspberrypi.com/documentation/)
* [libgpiod Documentation](https://libgpiod.readthedocs.io/)
* [Linux Kernel Documentation](https://docs.kernel.org/)
* [Git Documentation](https://git-scm.com/doc)
* [Microchip MCP3008](https://www.microchip.com/en-us/product/mcp3008)
* [Bosch BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/)


````

