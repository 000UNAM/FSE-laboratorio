
# 🌡️ Estación ambiental con Raspberry Pi 4

![C](https://img.shields.io/badge/C-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![Raspberry Pi](https://img.shields.io/badge/Raspberry%20Pi-%23A22846.svg?style=for-the-badge&logo=raspberrypi&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-%23FCC624.svg?style=for-the-badge&logo=linux&logoColor=black)
![Bash](https://img.shields.io/badge/Bash-%234EAA25.svg?style=for-the-badge&logo=gnubash&logoColor=white)
![Git](https://img.shields.io/badge/Git-%23F05032.svg?style=for-the-badge&logo=git&logoColor=white)
![GitHub](https://img.shields.io/badge/GitHub-%23181717.svg?style=for-the-badge&logo=github&logoColor=white)

**Universidad Nacional Autónoma de México**  
Facultad de Ingeniería — División de Ingeniería Eléctrica  
**Asignatura:** Fundamentos de Sistemas Embebidos (1858)  
**Grupo:** 6  
**Semestre:** 2027-1

---


## 📑 Índice

- [🚀 Descripción del proyecto](#-descripción-del-proyecto)
- [🔌 Hardware y conexiones](#-hardware-y-conexiones)
- [🖥️ Entorno de desarrollo](#-entorno-de-desarrollo)
- [📁 Estructura del proyecto](#-estructura-del-proyecto)
- [🛠️ Compilación](#-compilación)
- [▶️ Ejecución](#️-ejecución)
- [⏱️ Funcionamiento y temporización](#️-funcionamiento-y-temporización)
- [🧪 Pruebas realizadas](#-pruebas-realizadas)
- [🛡️ Manejo de errores](#️-manejo-de-errores)
- [🧠 Decisiones técnicas](#-decisiones-técnicas)
- [🤖 Uso de inteligencia artificial](#-uso-de-inteligencia-artificial)
- [📚 Referencias](#-referencias)

---



## 🚀 Descripción del proyecto

Sistema embebido de monitoreo ambiental desarrollado en C y Bash sobre una Raspberry Pi 4.

Utiliza un sensor BME280 para adquirir temperatura, humedad relativa y presión atmosférica, una pantalla OLED SSD1306 para mostrar los resultados y archivos CSV para conservar las mediciones.

El sistema funciona de manera periódica sin requerir intervención manual durante la adquisición.

## 🔌 Hardware y conexiones

| Componente | Descripción |
|---|---|
| Raspberry Pi 4 | Unidad principal de procesamiento |
| BME280 | Sensor de temperatura, humedad y presión |
| SSD1306 | Pantalla OLED de 128 × 64 píxeles |
| Bus I2C | Comunicación compartida con ambos periféricos |

Configuración comprobada:

- Bus: `/dev/i2c-1`.
- Sensor BME280: dirección `0x76`, identificador `0x60`.
- Pantalla SSD1306: dirección `0x3C`.
- Líneas I2C de la Raspberry Pi 4: SDA en GPIO2 (pin físico 3) y SCL en GPIO3 (pin físico 5).

Ambos dispositivos comparten las líneas SDA y SCL y requieren alimentación y tierra adecuadas. El cableado físico debe contrastarse con las fotografías del montaje.

## 🖥️ Entorno de desarrollo

| Elemento | Versión o configuración |
|---|---|
| Hardware | Raspberry Pi 4 |
| Sistema operativo | Debian GNU/Linux 13 (Trixie) |
| Arquitectura | aarch64 |
| Compilador | GCC 14.2 |
| Lenguaje principal | C11 |
| Automatización | Bash |
| Interfaz de hardware | I2C |

## 📁 Estructura del proyecto

```text
proyecto-final/
├── src/
│   ├── bme280.c
│   ├── bme280.h
│   ├── oled.c
│   ├── oled.h
│   ├── registro.c
│   ├── registro.h
│   ├── estacion.c
│   ├── prueba-bme280.c
│   ├── prueba-oled.c
│   └── prueba-estacion.c
├── scripts/
│   ├── s-01-prueba-oled.sh
│   ├── s-02-ejecuta-estacion.sh
│   └── s-03-compila-estacion.sh
├── bin/
├── evidencias/
└── README.md
```

Los archivos del directorio `bin/` se generan mediante compilación. Las evidencias contienen registros de ejecución y resultados experimentales.

## 🛠️ Compilación

Se requiere GCC y un entorno Linux con las cabeceras necesarias para los dispositivos I2C.

Desde la raíz del proyecto:

```bash
chmod u+x scripts/s-03-compila-estacion.sh
./scripts/s-03-compila-estacion.sh
```

El script comprueba los archivos fuente, compila los módulos y genera:

`bin/estacion`

Se utilizan las opciones `-std=c11 -Wall -Wextra -Wpedantic -O2`.

La compilación se realiza primero en un directorio temporal para evitar reemplazar el ejecutable funcional si GCC falla.

## ▶️ Ejecución

Para iniciar una ejecución supervisada:

```bash
chmod u+x scripts/s-02-ejecuta-estacion.sh
./scripts/s-02-ejecuta-estacion.sh
```

El script crea automáticamente una carpeta de evidencias, ejecuta el programa durante 310 segundos y conserva los archivos:

- `mediciones.csv`: mediciones ambientales.
- `ejecucion.log`: salida del programa y resultados del supervisor.

El programa también puede iniciarse directamente:

```bash
./bin/estacion mediciones.csv
```

En este caso, la ejecución continúa hasta recibir una señal de terminación o producirse un error. El archivo CSV de destino debe ser nuevo.

## ⏱️ Funcionamiento y temporización

La estación utiliza `CLOCK_MONOTONIC` y plazos absolutos para organizar las lecturas.

| Operación | Periodo programado |
|---|---|
| Lectura del BME280 | 2 segundos |
| Actualización de OLED | 2 segundos |
| Almacenamiento CSV | 10 segundos |

El BME280 funciona en modo forzado. El archivo CSV contiene fecha UTC, tiempo transcurrido, temperatura, humedad relativa y presión atmosférica.

La apertura del CSV utiliza creación exclusiva para evitar sobrescribir archivos existentes.

## 🧪 Pruebas realizadas

### Ejecución de 310 segundos

Se realizó una ejecución supervisada con los siguientes resultados:

| Parámetro | Resultado |
|---|---|
| Lecturas registradas | 155 |
| Intervalos de lectura | 154 |
| Promedio de intervalos | 2.000000 s |
| Mínimo registrado | 2.000 s |
| Máximo registrado | 2.000 s |
| Registros CSV | 30 |
| Errores detectados en la estructura y periodicidad del CSV | 0 |
| Código de salida del programa/supervisor | 0 |
| Código de salida de `tee` | 0 |

Los resultados temporales corresponden a las marcas registradas con resolución de milisegundos. No constituyen una medición independiente de precisión temporal absoluta.

### Prueba F1: dirección incorrecta del sensor

Se utilizó la dirección I2C `0x77` cuando el sensor real estaba configurado en `0x76`.

**Resultado obtenido:**

- Error de comunicación: `Remote I/O error`.
- Código de salida: `1`.
- Archivo CSV creado únicamente con encabezado.
- Ninguna medición almacenada.

### Prueba F2: ruta de almacenamiento inválida

Se solicitó crear el CSV dentro de un directorio inexistente.

**Resultado obtenido:**

- Error: `No such file or directory`.
- Código de salida: `1`.
- No se creó el CSV.
- La OLED mostró `ERROR DE ARCHIVO`.

Ambas pruebas utilizaron fallos controlados sin modificar el circuito físico.

## 🛡️ Manejo de errores

El programa contempla errores de inicialización y comunicación con los periféricos, escritura de archivos, actualización de la pantalla y temporización.

También maneja las señales `SIGINT` y `SIGTERM` para terminar la ejecución y liberar los recursos abiertos.

Cuando ocurre un error, el programa intenta informar el estado final mediante la pantalla OLED y termina con un código de salida distinto de cero.

## 🧠 Decisiones técnicas

Se adoptaron las siguientes decisiones:

- Utilizar I2C para comunicar el sensor y la pantalla por un mismo bus.
- Emplear `CLOCK_MONOTONIC` con plazos absolutos para reducir la acumulación de retrasos entre ciclos.
- Guardar registros cada 10 segundos y actualizar la pantalla cada 2 segundos.
- Utilizar `fflush()` después de cada escritura CSV para vaciar el búfer de la biblioteca C.
- Crear archivos CSV nuevos para proteger los resultados anteriores.
- Separar la adquisición, visualización, almacenamiento y coordinación en módulos C independientes.

## 🤖 Uso de inteligencia artificial

Se utilizó ChatGPT como herramienta de asistencia técnica y pedagógica durante el desarrollo del proyecto.

Su participación incluyó apoyo en la elaboración y revisión del código C y Bash, manejo de errores, temporización, automatización de compilación y ejecución, análisis de resultados y redacción de documentación.

Las conexiones físicas, ejecuciones en Raspberry Pi, observaciones de la pantalla y archivos de evidencia proceden de las pruebas realizadas en el hardware real.

El uso de IA incluyó asistencia en la generación de código y no se limitó a correcciones de redacción.

## 📚 Referencias

- [Raspberry Pi — Documentación oficial](https://www.raspberrypi.com/documentation/)
- [Bosch Sensortec — Sensor BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/)
- [Linux — Interfaz I2C para espacio de usuario](https://docs.kernel.org/i2c/dev-interface.html)
- [GNU GCC — Documentación](https://gcc.gnu.org/onlinedocs/)
