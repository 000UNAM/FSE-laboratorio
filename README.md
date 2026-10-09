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
- Procesos, hilos y sincronización con mutex y semáforos.
- Módulos del kernel, dispositivos de caracteres y llamadas al sistema.
- Comunicación entre procesos y medición del costo de acceso al kernel.
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
| [05](practicas/05/) | Procesos e hilos | fork, pthreads, mutex, semáforos, nice y taskset |
| [06](practicas/06/) | Drivers y espacio de kernel | Módulos, sysfs, miscdevice, IPC y strace |

---

## 📁 Estructura del repositorio

| Directorio | Archivos publicados |
| --- | --- |
| [`practicas/01/`](practicas/01/) | `reporte-sistema.c`, `sysinfo.sh` |
| [`practicas/02/`](practicas/02/) | `leds.c`, `boton-polling.c`, `semaforo.c` |
| [`practicas/03/`](practicas/03/) | `bmp280_id.c`, `bme280_temp.c`, `leer_pot.c`, `sensor_combinado.c` |
| [`practicas/04/`](practicas/04/) | `sensor-combinado.service` |
| [`practicas/05/`](practicas/05/) | `fork_hijos.c`, `zombie.c`, `contador_hilos.c`, `contador_mutex.c`, `costo.c`, `quemar.c`, `productor_consumidor.c`, `respuestas_p5.txt` |
| [`practicas/06/`](practicas/06/) | `Makefile`, `hola.c`, `parametros.c`, `p6buf.c`, `cliente.c`, `demo_ipc.c`, `tiempo.c`, `p6cnt.c`, `descargar.sh`, `evidencia.md` |

Cada directorio corresponde a una práctica y contiene principalmente los
**programas fuente desarrollados en C o Bash** y los archivos de configuración
necesarios.

Los programas y módulos se compilan localmente. La excepción publicada es
[`practicas/06/descargar.sh`](practicas/06/descargar.sh): un ejecutable ARM64
generado con `shc` para retirar los módulos de la práctica 6. Aunque su nombre
termina en `.sh`, es un binario ELF y se ejecuta directamente con `./descargar.sh`.

Los scripts auxiliares de trabajo y la carpeta local `ejecutables-arm64/`
se conservan fuera de los archivos publicados en GitHub.

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

### 🧵 Práctica 5 — Procesos e hilos

Estudio de la creación y observación de procesos e hilos, el acceso concurrente
a memoria compartida y la coordinación de tareas. Se realizó por SSH, sin
circuito externo.

#### Temas y programas

| Archivo | Experimento |
| --- | --- |
| `fork_hijos.c` | Crear tres hijos con `fork`, observar PID y PPID y esperar su terminación. |
| `zombie.c` | Observar un hijo terminado en estado `Z` y recoger su estado mediante `waitpid`. |
| `contador_hilos.c` | Incrementar un contador desde dos hilos sin proteger la sección crítica. |
| `contador_mutex.c` | Proteger los incrementos con un mutex. |
| `costo.c` | Comparar ciclos completos de creación, terminación y espera de procesos e hilos. |
| `quemar.c` | Observar el reparto de CPU con `nice` y afinidad mediante `taskset`. |
| `productor_consumidor.c` | Coordinar un buffer circular de ocho posiciones con semáforos y mutex. |

#### Resultados observados

- El hijo zombie permaneció visible hasta que su padre ejecutó `waitpid`.
- El contador sin mutex no obtuvo los **2 000 000** incrementos esperados en
  ninguna de diez ejecuciones; la versión con mutex los obtuvo en las diez.
  La versión sin protección contiene una carrera de datos y no garantiza un
  resultado definido en C.
- En tres ensayos de 2 000 ciclos, los promedios fueron **533.455 ms** para
  procesos y **112.254 ms** para hilos. Estos tiempos incluyen creación,
  terminación, espera y el bucle del experimento.
- Con dos cargas en el mismo núcleo y autogroup, la muestra final mostró un
  reparto aproximado de **99 % / 1 %** con `nice 0 / 19`, y **50 % / 50 %** con
  `nice 0 / 0`. Son resultados de esa ejecución, no porcentajes garantizados.
- El productor y el consumidor transfirieron **100 datos**, sin errores ni
  elementos pendientes; la ocupación máxima fue **8/8**.

[Códigos completos de la práctica 5](practicas/05/) ·
[Respuestas y referencias](practicas/05/respuestas_p5.txt)

---

### 🧩 Práctica 6 — Drivers y espacio de kernel

Construcción de módulos y dispositivos virtuales para estudiar cómo una
aplicación de usuario solicita operaciones al kernel. Se trabajó en una
Raspberry Pi 4 con **Debian 13 (trixie), ARM64**, kernel
**`6.18.50+rpt-rpi-v8`** y GCC **14.2.0**. Estas versiones corresponden al
entorno del experimento.

#### Partes y archivos

| Parte | Trabajo realizado | Archivos o interfaces |
| --- | --- | --- |
| A | Comprobar el entorno y la correspondencia entre el kernel y sus headers. | `Makefile`, `/lib/modules/$(uname -r)/build` |
| B | Cargar y descargar un módulo mínimo; observar `lsmod`, sysfs y el diario del kernel. | `hola.c` |
| C | Pasar parámetros al cargar el módulo y consultar o modificar los permitidos en sysfs. | `parametros.c`, `/sys/module/parametros/parameters/` |
| D | Implementar un dispositivo de caracteres con un buffer de 256 bytes protegido por mutex. | `p6buf.c`, `/dev/p6buf` |
| E | Usar el dispositivo desde C y comparar la interacción con una tubería entre padre e hijo. | `cliente.c`, `demo_ipc.c`, `strace` |
| F | Medir lectura de memoria, `getppid`, `pread` y transferencias SPI. | `tiempo.c` |
| G | Relacionar programas de I2C, SPI y GPIO con los nodos y drivers que utilizan. | Programas de la práctica 3, `/dev`, `/sys`, trazas de `strace` |
| Reto | Contar aperturas, aceptar la orden `reset` y rechazar órdenes inválidas. | `p6cnt.c`, `/dev/p6cnt` |
| H | Retirar los módulos y comprobar que desaparecen los dispositivos virtuales. | `descargar.sh` |

#### Resultados observados

- Los headers coincidieron con la versión del kernel en ejecución.
- `hola` apareció en `lsmod`; cargar un módulo no creó un proceso con PID propio.
- Se verificaron los permisos y los cambios de parámetros mediante sysfs.
- El cliente escribió y recuperó un mensaje de **29 bytes** en `p6buf`.
  Una escritura de **300 bytes** fue rechazada con `ENOSPC`
  («no queda espacio en el dispositivo»).
- `p6cnt` contó aperturas, aceptó `reset` y rechazó una orden inválida con
  `EINVAL` («argumento no válido»). La siguiente lectura después del reinicio
  mostró una apertura, porque abrir el dispositivo también incrementa el contador.
- Al terminar, se comprobó la ausencia de `hola`, `parametros`, `p6buf` y
  `p6cnt`, así como de `/dev/p6buf` y `/dev/p6cnt`.

#### Mediciones de tiempo

Mediana de los promedios por operación obtenidos en tres corridas:

| Operación | Tiempo |
| --- | ---: |
| Lectura de memoria de usuario | 1.1 ns/op |
| Llamada al sistema `getppid` | 449.2 ns/op |
| `pread` de un byte en `/dev/p6buf` | 606.7 ns/op |
| Transferencia SPI de tres bytes a 1 MHz | 35 979.3 ns/op |

Cada corrida utilizó 1 000 000 de operaciones para memoria, `getppid` y
`pread`, y 10 000 transferencias SPI. Son mediciones de este equipo y de este
programa, con el trabajo del bucle incluido.

**La práctica 6 se realizó sin circuito externo.** Las consultas I2C al sensor
ausente produjeron `EIO` («error de entrada/salida»). Las transferencias SPI
completadas permiten estudiar el acceso al bus, pero no acreditan la presencia
del MCP3008 ni una lectura válida de voltaje o temperatura.

[Códigos completos de la práctica 6](practicas/06/) ·
[Evidencias y respuestas de las partes A–G, reto y descarga final](practicas/06/evidencia.md)

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

Los binarios de estos programas se generan localmente.

### Compilar los programas de la práctica 5

En la terminal SSH de la Raspberry Pi, desde la carpeta de la práctica:

```bash
cd /unam/fse/practicas/05

gcc -Wall -Wextra -std=c11 -O0 fork_hijos.c -o fork_hijos
gcc -Wall -Wextra -std=c11 -O0 zombie.c -o zombie
gcc -Wall -Wextra -std=c11 -O0 -pthread contador_hilos.c -o contador_hilos
gcc -Wall -Wextra -std=c11 -O0 -pthread contador_mutex.c -o contador_mutex
gcc -Wall -Wextra -std=c11 -O0 -pthread costo.c -o costo
gcc -Wall -Wextra -std=c11 -O2 quemar.c -o quemar
gcc -Wall -Wextra -std=c11 -O2 -pthread productor_consumidor.c -o productor_consumidor
```

`-pthread` habilita las opciones de compilación y enlace necesarias para los
programas con hilos. Se conservan las opciones de optimización utilizadas en
los experimentos.

### Compilar los módulos y programas de la práctica 6

Estos comandos se ejecutan **dentro de la sesión SSH de la Raspberry Pi**.
Primero se debe comprobar que los headers corresponden al kernel en ejecución:

```bash
cd /unam/fse/practicas/06

uname -r
readlink -f "/lib/modules/$(uname -r)/build"
make -s -C "/lib/modules/$(uname -r)/build" kernelrelease
```

La versión mostrada por `kernelrelease` debe coincidir con `uname -r`. Si faltan
los headers o las versiones difieren, se debe resolver esa diferencia antes de
compilar. Con la comprobación correcta:

```bash
make
gcc -Wall -Wextra -O2 cliente.c -o cliente
gcc -Wall -Wextra -O2 demo_ipc.c -o demo_ipc
gcc -Wall -Wextra -O2 tiempo.c -o tiempo
```

El `Makefile` utiliza kbuild para generar `hola.ko`, `parametros.ko`, `p6buf.ko`
y `p6cnt.ko` para ese kernel. Los tres comandos `gcc` compilan los programas de
usuario. Compilar los módulos no los carga.

### Probar el dispositivo virtual y finalizar

Con `p6buf` todavía descargado, en la Raspberry Pi:

```bash
cd /unam/fse/practicas/06
sudo insmod ./p6buf.ko
./cliente "hola desde espacio de usuario"
./demo_ipc
./tiempo
```

`cliente` deja contenido en el buffer antes de la medición con `tiempo`.
La parte SPI requiere que exista `/dev/spidev0.0` y que el usuario tenga acceso;
si no puede abrirlo, el programa indica que la omite.

Para retirar los módulos de la práctica:

```bash
cd /unam/fse/practicas/06
./descargar.sh
```

`descargar.sh` descarga los módulos presentes, comprueba su ausencia y genera
un log. Se puede repetir: los módulos que ya están descargados se omiten.
Su ruta de trabajo está fijada a `/unam/fse/practicas/06`.

El archivo publicado es un **ELF ARM64 generado con `shc`** y requiere un
entorno Linux compatible y Bash. Se ejecuta con `./descargar.sh`, no con
`bash descargar.sh`. Se puede copiar a Linux Mint como respaldo, pero no se
ejecuta de forma nativa en una computadora x86_64. `shc` dificulta la lectura
directa del script; no garantiza que su contenido sea irrecuperable.

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

El repositorio conserva el historial de las prácticas **01 a 06** en la rama
`main`. La publicación se realiza desde el repositorio de la Raspberry Pi,
ubicado en `/unam/fse`.

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

Si el remoto contiene cambios nuevos, primero se consulta la diferencia:

```bash
git fetch origin
git status --short --branch
git log --oneline --left-right HEAD...origin/main
```

Con los cambios locales preparados, si la rama solo está atrasada se puede
integrar mediante `git merge --ff-only origin/main`. Si existen commits en
ambos lados, se revisan y se integran mediante merge, como se hizo en la
práctica 5, antes de volver a publicar. Se seleccionan los archivos por su ruta
para evitar incluir binarios, registros o scripts auxiliares por accidente.

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
- Procesos, PID, PPID y estados de ejecución.
- Hilos POSIX, condiciones de carrera y exclusión mutua.
- Semáforos, productor-consumidor y buffers circulares.
- Prioridad con `nice` y afinidad de CPU.
- Módulos del kernel y parámetros mediante sysfs.
- Dispositivos de caracteres y operaciones de archivo.
- Llamadas al sistema, tuberías y trazas con `strace`.
- Medición del costo de acceso al kernel y a buses de periféricos.
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
- [Linux: construcción de módulos externos con kbuild](https://docs.kernel.org/kbuild/modules.html)
- [Linux manual pages: fork(2)](https://man7.org/linux/man-pages/man2/fork.2.html)
- [Linux manual pages: pthreads(7)](https://man7.org/linux/man-pages/man7/pthreads.7.html)
- [shc: documentación del proyecto](https://github.com/neurobin/shc)
- [systemd Documentation](https://www.freedesktop.org/software/systemd/man/systemd.html)
- [systemd-analyze Documentation](https://www.freedesktop.org/software/systemd/man/systemd-analyze.html)
- [NetworkManager-wait-online](https://networkmanager.dev/docs/api/latest/NetworkManager-wait-online.service.html)
- [Git Documentation](https://git-scm.com/doc)
- [Microchip MCP3008](https://www.microchip.com/en-us/product/mcp3008)
- [Bosch BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/)

