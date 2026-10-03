# 🔧 Fundamentos de Sistemas Embebidos

![C](https://img.shields.io/badge/C-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Raspberry Pi](https://img.shields.io/badge/Raspberry%20Pi-%23A22846.svg?style=for-the-badge&logo=raspberrypi&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-%23FCC624.svg?style=for-the-badge&logo=linux&logoColor=black)
![systemd](https://img.shields.io/badge/systemd-%23000000.svg?style=for-the-badge&logo=systemd&logoColor=white)
![Git](https://img.shields.io/badge/Git-%23F05032.svg?style=for-the-badge&logo=git&logoColor=white)
![GitHub](https://img.shields.io/badge/GitHub-%23181717.svg?style=for-the-badge&logo=github&logoColor=white)

Repositorio de prácticas de la asignatura **Fundamentos de Sistemas Embebidos**,
realizadas sobre una **Raspberry Pi 4** con Linux.

---

## 🚀 Objetivo

Aplicar conceptos fundamentales de sistemas embebidos utilizando una
**Raspberry Pi 4** como plataforma de desarrollo.

A lo largo de las prácticas se trabaja con:

- Administración básica de Raspberry Pi OS.
- Programación en C y Bash.
- GPIO digital.
- Entradas y salidas.
- Polling y eventos.
- Protocolos de comunicación I2C y SPI.
- Sensores externos.
- Conversión analógico-digital.
- Proceso de arranque de Linux.
- Servicios y dependencias con systemd.
- Análisis y optimización del tiempo de arranque.
- Control de versiones con Git.
- Desarrollo y prueba de sistemas que interactúan con hardware real.

---

## 📚 Resumen de prácticas

| Práctica | Tema principal | Tecnologías |
| --- | --- | --- |
| 01 | Introducción al entorno de la Raspberry Pi | Linux, Bash, C, SSH y Git |
| 02 | Entradas y salidas digitales | GPIO, libgpiod, polling y eventos |
| 03 | Sensores y conversión analógica | I2C, SPI, BME280, MCP3008 y GPIO |
| 04 | Proceso de arranque | EEPROM, journalctl, systemd y systemd-analyze |

---

## 📁 Estructura del repositorio

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
    ├── 03/
    │   ├── bmp280_id.c
    │   ├── bme280_temp.c
    │   ├── leer_pot.c
    │   └── sensor_combinado.c
    │
    └── 04/
        └── sensor-combinado.service
```

Cada directorio corresponde a una práctica y contiene principalmente los
**programas fuente desarrollados en C o Bash** y los archivos de configuración
necesarios.

Los archivos ejecutables generados durante la compilación no forman parte del
repositorio.

---

## 🧪 Prácticas

### 📌 Práctica 1 — Introducción al entorno de la Raspberry Pi

Primera aproximación al entorno de desarrollo utilizado durante el curso.

#### Temas trabajados

- Acceso remoto mediante SSH.
- Navegación y administración básica en Linux.
- Identificación del sistema.
- Uso de herramientas de línea de comandos.
- Programación básica en C.
- Obtención de información del sistema.
- Organización del directorio de prácticas.
- Primer uso de Git.

#### Programas principales

```text
reporte-sistema.c
sysinfo.sh
```

---

### 💡 Práctica 2 — GPIO: entradas y salidas digitales

Uso de las líneas **GPIO de la Raspberry Pi** desde la terminal y desde
programas en C mediante **libgpiod**.

#### Temas trabajados

- Numeración GPIO y pines físicos.
- Control de LED.
- Lectura de botones.
- Uso de resistencias pull-down.
- Programación con libgpiod.
- Polling.
- Eventos.
- Uso de CPU.
- Antirrebote.
- Máquina de estados.
- Semáforo peatonal.

#### Programas principales

```text
leds.c
boton-polling.c
semaforo.c
```

#### 🎥 Evidencias en video

**Secuencia de LED**

[https://youtube.com/shorts/u49dvJY_IYM](https://youtube.com/shorts/u49dvJY_IYM)

**Semáforo peatonal**

[https://youtube.com/shorts/6uxbRbtLUmI](https://youtube.com/shorts/6uxbRbtLUmI)

---

### 🌡️ Práctica 3 — I2C y SPI: conversión analógica y sensores

Integración de distintos protocolos de comunicación utilizados comúnmente en
sistemas embebidos.

La Raspberry Pi no dispone de un convertidor analógico-digital nativo, por lo
que se utilizó un **MCP3008** para adquirir una señal analógica proveniente de
un potenciómetro.

También se utilizó un sensor **BME280** conectado mediante I2C.

#### Hardware utilizado

- Raspberry Pi 4.
- BME280.
- MCP3008.
- Potenciómetro de 10 kΩ.
- LED.
- Resistencia.
- Protoboard.
- Cables jumper.

#### 🔌 I2C

El sensor BME280 se conecta utilizando el bus I2C:

```text
BME280        Raspberry Pi

VIN    ───►   3.3 V
GND    ───►   GND
SCL    ───►   GPIO3 / SCL
SDA    ───►   GPIO2 / SDA
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

#### Programas

```text
bmp280_id.c
bme280_temp.c
```

`bmp280_id.c` identifica el dispositivo conectado al bus I2C.

`bme280_temp.c` obtiene los coeficientes de calibración almacenados en el sensor
y realiza la compensación necesaria para obtener la temperatura ambiental.

#### ⚡ SPI

Para convertir la posición del potenciómetro en un valor digital se utilizó el
ADC **MCP3008**, conectado mediante SPI.

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

#### Programa

```text
leer_pot.c
```

La lectura obtenida se convierte a voltaje mediante:

```text
V = (ADC × 3.3) / 1023
```

#### 🔀 Sistema combinado

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

Posteriormente, el valor del ADC se transforma en un umbral entre:

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

#### 🎥 Video de funcionamiento

**Práctica 3 — I2C y SPI con BME280 y MCP3008 en Raspberry Pi 4**

[https://youtube.com/shorts/DRcxMAXERnQ](https://youtube.com/shorts/DRcxMAXERnQ)

---

### ⚙️ Práctica 4 — Proceso de arranque y servicios systemd

Análisis del proceso de arranque de la Raspberry Pi, desde el firmware y el
bootloader de EEPROM hasta el inicio de servicios en el espacio de usuario.

#### Temas trabajados

- Identificación del núcleo y del arranque actual.
- Inspección de los archivos ubicados en `/boot/firmware`.
- Verificación del bootloader de EEPROM.
- Confirmación de las interfaces I2C y SPI.
- Revisión del diario del sistema con `journalctl`.
- Búsqueda de unidades fallidas.
- Medición del arranque con `systemd-analyze`.
- Creación de un servicio para `sensor_combinado`.
- Diagnóstico de dependencias incorrectas.
- Comprobación del inicio automático después de un reinicio.
- Optimización reversible del tiempo de arranque.
- Registro del archivo de unidad en Git.

#### Archivo principal

```text
practicas/04/sensor-combinado.service
```

La unidad ejecuta el programa compilado:

```text
/unam/fse/practicas/03/sensor_combinado
```

y comprueba previamente la existencia de los dispositivos:

```text
/dev/i2c-1
/dev/spidev0.0
```

La configuración relevante de la unidad es:

```ini
[Unit]
Description=Lectura combinada de sensores y control de LED
After=local-fs.target

[Service]
Type=simple
User=ivan
Group=ivan
SupplementaryGroups=i2c spi gpio
WorkingDirectory=/unam/fse/practicas/03
ExecStartPre=/usr/bin/test -c /dev/i2c-1
ExecStartPre=/usr/bin/test -c /dev/spidev0.0
ExecStart=/usr/bin/stdbuf -oL -eL /unam/fse/practicas/03/sensor_combinado
Restart=on-failure
RestartSec=5
KillSignal=SIGINT
TimeoutStopSec=5
StandardOutput=journal
StandardError=journal
SyslogIdentifier=sensor-combinado

[Install]
WantedBy=multi-user.target
```

> [!IMPORTANT]
> La unidad contiene rutas y un usuario específicos del equipo de laboratorio.
> Si el repositorio se clona en otra ubicación, se deben ajustar `User`,
> `Group`, `WorkingDirectory` y `ExecStart`.

#### Diagnóstico realizado

La primera versión intentó depender directamente de unidades con nombres como:

```text
dev-i2c\x2d1.device
dev-spidev0.0.device
```

Aunque `/dev/i2c-1` y `/dev/spidev0.0` existían, esas unidades de systemd
permanecían inactivas y provocaban un retraso cercano a **90 segundos**.

La solución fue iniciar después de `local-fs.target` y validar los nodos de
carácter mediante `ExecStartPre`. Después de la corrección, el servicio tardó
aproximadamente **116 ms** en iniciar.

#### Demostración del inicio automático

Después de habilitar el servicio se reinició la Raspberry Pi. Sin ejecutar
`systemctl start` manualmente, se comprobó:

- Estado `enabled`.
- Estado `active (running)`.
- Un PID nuevo después del reinicio.
- Una marca `ExecMainStartTimestamp` correspondiente al arranque actual.
- Nuevas mediciones en el diario.

Estas evidencias demostraron que el programa inició automáticamente.

#### Optimización del arranque

El análisis identificó a `NetworkManager-wait-online.service` como una espera
innecesaria para esta práctica. Se deshabilitó únicamente esa unidad; el servicio
principal `NetworkManager.service` permaneció habilitado y activo.

| Medición | Antes | Después | Mejora |
| --- | ---: | ---: | ---: |
| Arranque total | 24.964 s | 19.716 s | 5.248 s (21.0 %) |
| Espacio de usuario | 22.494 s | 17.134 s | 5.360 s (23.8 %) |
| `network-online.target` | 21.997 s | 16.065 s | 5.932 s (27.0 %) |
| `graphical.target` | 16.388 s | 16.411 s | Sin cambio significativo |

Después del reinicio se verificó que:

- La conexión Wi-Fi seguía activa.
- Existía una ruta predeterminada.
- El servicio de sensores continuaba funcionando.
- No existían unidades fallidas.

> [!WARNING]
> Deshabilitar `NetworkManager-wait-online.service` no es una optimización
> universal. Debe conservarse cuando algún servicio de arranque necesita que la
> red esté completamente configurada.

Al finalizar la práctica, `sensor-combinado.service` fue detenido y
deshabilitado para poder desmontar el circuito de forma segura.

---

## 🛠️ Compilación

Los programas están desarrollados para **Linux / Raspberry Pi OS**.

### Requisitos

```bash
sudo apt update
sudo apt install -y build-essential libgpiod-dev i2c-tools pkg-config
```

### Compilar los programas de la práctica 3

Desde la raíz del repositorio:

```bash
gcc practicas/03/bmp280_id.c \
  -o practicas/03/bmp280_id
```

```bash
gcc practicas/03/bme280_temp.c \
  -o practicas/03/bme280_temp
```

```bash
gcc practicas/03/leer_pot.c \
  -o practicas/03/leer_pot
```

### Compilar con libgpiod

El programa combinado utiliza libgpiod:

```bash
gcc practicas/03/sensor_combinado.c \
  -o practicas/03/sensor_combinado \
  $(pkg-config --cflags --libs libgpiod)
```

Los binarios se generan localmente, pero no se agregan al repositorio.

---

## ▶️ Ejecución manual

Ejemplo:

```bash
./practicas/03/sensor_combinado
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

## ⚙️ Instalación del servicio de la práctica 4

Antes de instalar la unidad, se debe compilar `sensor_combinado` y conectar el
hardware requerido.

Desde la raíz del repositorio:

```bash
sudo install -m 0644 \
  practicas/04/sensor-combinado.service \
  /etc/systemd/system/sensor-combinado.service

sudo systemctl daemon-reload
sudo systemctl enable --now sensor-combinado.service
```

### Verificar el servicio

```bash
systemctl is-enabled sensor-combinado.service
systemctl is-active sensor-combinado.service
systemctl status sensor-combinado.service --no-pager
```

### Consultar las mediciones

```bash
journalctl -u sensor-combinado.service -b --no-pager
```

Para seguir las mediciones en tiempo real:

```bash
journalctl -u sensor-combinado.service -f
```

### Detener y deshabilitar el servicio

```bash
sudo systemctl stop sensor-combinado.service
sudo systemctl disable sensor-combinado.service
```

---

## ⏱️ Comandos de diagnóstico del arranque

### Tiempo general

```bash
systemd-analyze time
```

### Unidades más lentas

```bash
systemd-analyze blame
```

### Cadena crítica general

```bash
systemd-analyze critical-chain
```

### Cadena del servicio de sensores

```bash
systemd-analyze critical-chain sensor-combinado.service
```

### Diario del arranque actual

```bash
journalctl -b --no-pager
```

### Unidades fallidas

```bash
systemctl --failed --no-pager
```

---

## 🔄 Flujo de trabajo con Git

El repositorio conserva el historial de cada práctica.

```text
practica-01
     ↓
Reorganiza repositorio con raiz en /unam/fse
     ↓
practica-02
     ↓
practica-03
     ↓
Update README.md
     ↓
practica-04
```

El commit final de la práctica 4 fue:

```text
1aa7cd0 practica-04
```

Para consultar el historial:

```bash
git log --oneline --decorate --graph
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

Si el remoto contiene cambios nuevos, primero se deben integrar:

```bash
git fetch origin
git rebase origin/main
git push origin main
```

No se recomienda utilizar `git push --force` para resolver una divergencia en
la rama principal.

---

## 🧠 Conceptos estudiados

Durante las prácticas se han aplicado conceptos como:

- Sistemas embebidos.
- Linux embebido.
- GPIO.
- Entradas y salidas digitales.
- ADC.
- Polling.
- Eventos.
- Interrupciones a nivel de kernel.
- I2C.
- SPI.
- Sensores digitales.
- Conversión analógico-digital.
- Programación en C.
- Manejo de dispositivos mediante `/dev`.
- Interfaces del kernel de Linux.
- Firmware y bootloader de EEPROM.
- Proceso de arranque de Linux.
- Diario del sistema.
- Unidades, servicios y objetivos de systemd.
- Dependencias de arranque.
- Medición con `systemd-analyze`.
- Optimización reversible.
- Control de versiones con Git.

---

## 🚧 Desarrollo

El repositorio continuará creciendo conforme se desarrollen nuevas prácticas de
la asignatura.

Cada nueva práctica deberá mantener la estructura:

```text
practicas/
└── NN/
```

donde `NN` corresponde al número de práctica.

Se busca mantener:

- Código fuente legible.
- Separación entre código y binarios compilados.
- Commits identificables por práctica.
- Evidencias reproducibles.
- Documentación de conexiones y hardware.
- Servicios con rutas y dependencias explícitas.
- Cambios de configuración reversibles.
- Uso consistente de Git.

---

## 📖 Referencias

- [Raspberry Pi Documentation](https://www.raspberrypi.com/documentation/)
- [Raspberry Pi bootloader EEPROM](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html)
- [libgpiod Documentation](https://libgpiod.readthedocs.io/)
- [Linux Kernel Documentation](https://docs.kernel.org/)
- [systemd Documentation](https://www.freedesktop.org/software/systemd/man/systemd.html)
- [systemd-analyze Documentation](https://www.freedesktop.org/software/systemd/man/systemd-analyze.html)
- [NetworkManager-wait-online](https://networkmanager.dev/docs/api/latest/NetworkManager-wait-online.service.html)
- [Git Documentation](https://git-scm.com/doc)
- [Microchip MCP3008](https://www.microchip.com/en-us/product/mcp3008)
- [Bosch BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/)

