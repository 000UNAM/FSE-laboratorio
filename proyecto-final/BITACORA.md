# Bitácora técnica — Estación ambiental

**UNAM — Facultad de Ingeniería**  
**Asignatura:** Fundamentos de Sistemas Embebidos  
**Proyecto:** Estación ambiental con Raspberry Pi 4  
**Fecha de documentación:** 03/11/2026

---

## Clase 15. Almacenamiento de archivos

**Problema:** necesitamos conservar mediciones ambientales sin depender de que la pantalla OLED permanezca encendida.

**Decisión:** utilizar archivos CSV con cinco campos: fecha UTC, tiempo transcurrido, temperatura, humedad relativa y presión atmosférica.

**Cifras y justificación:**

- Periodo de registro: 10 segundos.
- Proyección para 1 hora: 360 registros.
- Proyección para 24 horas: 8,640 registros.
- Prueba efectuada: 30 registros durante una ejecución supervisada de 310 segundos.

**Implementación:** el módulo `registro.c` utiliza `fopen()` con modo `wx` para evitar sobrescribir archivos existentes. Después de escribir cada registro, utiliza `fflush()` para vaciar el búfer de la biblioteca C.

**Resultado:** el CSV de la prueba de 310 segundos se validó con 30 registros, 29 intervalos y ningún error detectado en su estructura o periodicidad dentro de la tolerancia evaluada.

## Clase 16. Administración de energía

**Problema:** un sistema de adquisición continua debe evitar trabajo innecesario del procesador.

**Decisión:** programar lecturas cada 2 segundos mediante esperas con `clock_nanosleep()` y plazos absolutos basados en `CLOCK_MONOTONIC`.

**Cifras y justificación:**

- Periodo de adquisición: 2 segundos.
- Frecuencia nominal: 0.5 lecturas por segundo.
- Periodo de almacenamiento: 10 segundos.
- Relación nominal: cinco periodos de adquisición por cada registro CSV.

La espera programada evita utilizar un ciclo de consulta activa permanente mientras llega el siguiente instante de lectura.

**Resultado:** se registraron 155 lecturas durante la ejecución supervisada y un intervalo promedio de 2.000 segundos según el log.

**Limitación:** no se ha medido el consumo eléctrico en watts ni la energía total consumida. Por tanto, no se atribuye un ahorro energético cuantificado al sistema.

## Clase 17. Linux embebido

**Problema:** necesitamos acceder a sensores físicos, programar tareas periódicas y conservar información desde una computadora de propósito embebido.

**Decisión:** utilizar Linux con módulos independientes en C para la adquisición, visualización, almacenamiento y coordinación.

**Configuración:**

- Plataforma: Raspberry Pi 4.
- Arquitectura: aarch64.
- Comunicación: `/dev/i2c-1`.
- BME280: dirección `0x76`.
- SSD1306: dirección `0x3C`.
- Resolución de la pantalla: 128 × 64 píxeles.

**Implementación:** `estacion.c` coordina las operaciones, gestiona `SIGINT` y `SIGTERM` y libera los recursos abiertos al finalizar.

**Resultado:** la ejecución de 310 segundos terminó con código 0. Adicionalmente, las pruebas de dirección incorrecta y ruta inválida produjeron código 1, permitiendo distinguir los fallos de una terminación satisfactoria.

## Clase 18. Elección del sistema operativo

**Problema:** seleccionar un sistema operativo que proporcione acceso a I2C, compilación nativa, herramientas de automatización y administración de procesos.

**Decisión:** conservar Debian GNU/Linux 13 (Trixie), instalado en la Raspberry Pi 4.

**Cifras y configuración:**

- Sistema operativo: Debian 13.
- Arquitectura: ARM de 64 bits.
- Compilador: GCC 14.2.
- Estándar del programa: C11.
- Opciones de compilación: `-Wall -Wextra -Wpedantic -O2`.

**Justificación:** Linux permite utilizar la interfaz I2C mediante dispositivos de carácter, desarrollar programas en C y automatizar las tareas mediante Bash, sin introducir otro sistema operativo.

**Resultado:** el programa completo compiló sin diagnósticos visibles y se verificó que el ejecutable generado por el script `s-03-compila-estacion.sh` era idéntico byte por byte al ejecutable de verificación anterior.

---

## Validación experimental — 2 de noviembre de 2026

Se realizó una ejecución supervisada de 310 segundos. Se comprobaron 155 lecturas del sensor y 30 registros CSV, sin errores detectados de periodicidad dentro de la resolución registrada.

También se realizaron dos pruebas controladas:

**F1 — Dirección I2C incorrecta:** se utilizó `0x77` en lugar de `0x76`. El programa mostró `Remote I/O error`, terminó con código 1 y no almacenó mediciones.

**F2 — Ruta CSV inválida:** se solicitó escribir dentro de un directorio inexistente. El programa mostró `No such file or directory`, terminó con código 1 y la OLED indicó `ERROR DE ARCHIVO`.

Las pruebas se ejecutaron sin modificar el cableado físico.

## Compilación reproducible — 3 de noviembre de 2026

Se verificó el script `s-03-compila-estacion.sh`, que compila `estacion.c`, `bme280.c`, `oled.c` y `registro.c`.

La ejecución terminó satisfactoriamente, sin diagnósticos visibles de GCC. La comparación mediante `cmp` confirmó que el ejecutable recién generado era idéntico al binario de verificación.

## Declaración de asistencia de IA

ChatGPT proporcionó asistencia en el diseño y revisión de código C y Bash, el análisis de las pruebas y la preparación de esta documentación. Las ejecuciones, mediciones y observaciones del hardware se documentan a partir de los resultados obtenidos en la Raspberry Pi real.

Las proyecciones numéricas se distinguen de los resultados experimentales.
