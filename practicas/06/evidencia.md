# Práctica 6: Driver — evidencia y respuestas

Grupo 6 · Fundamentos de Sistemas Embebidos · Facultad de Ingeniería, UNAM.

[Códigos completos de la práctica](https://github.com/000UNAM/FSE-laboratorio/tree/main/practicas/06)

Las salidas siguientes proceden de las ejecuciones reales en la Raspberry. Los cálculos derivados se distinguen de las mediciones. Se trabajó sin circuito externo.

## Parte A. Entorno y headers

### A.1 Salidas de los cinco comandos

```text
$ uname -r
6.18.50+rpt-rpi-v8
$ uname -m
aarch64
$ dpkg --print-architecture
arm64
$ grep PRETTY_NAME /etc/os-release
PRETTY_NAME="Debian GNU/Linux 13 (trixie)"
$ gcc --version | head -n 1
gcc (Debian 14.2.0-19) 14.2.0
```

El kernel y el sistema de paquetes son de 64 bits. Se utilizó la Raspberry Pi 4 Model B Rev 1.5, usuario ivan, hostname rpi-fse-inm, en `/unam/fse/practicas/06`.

### A.2–A.4 Headers y correspondencia

```text
$ ls /lib/modules/$(uname -r)/build/Makefile
/lib/modules/6.18.50+rpt-rpi-v8/build/Makefile
$ readlink -f /lib/modules/$(uname -r)/build
/usr/src/linux-headers-6.18.50+rpt-rpi-v8
```

La ruta termina con la misma versión que `uname -r`. También se consultó la versión de construcción de los headers y coincidió. No se necesitó el Plan B. Registro de comprobación:

```text
Registro de esta ejecucion: /unam/fse/practicas/06/preparacion-p06-20261008-175743-uqjau7.log

=== Correspondencia de headers ===
Kernel en ejecucion: 6.18.50+rpt-rpi-v8
Version de headers:  6.18.50+rpt-rpi-v8
Ruta de headers:     /usr/src/linux-headers-6.18.50+rpt-rpi-v8
OK: versiones coincidentes.

Se conserva evidencia.md existente.

=== Contenido actual de la practica ===
total 36
drwxrwxr-x 2 ivan ivan 4096 Oct  8 17:57 .
drwxr-xr-x 8 ivan ivan 4096 Oct  8 17:22 ..
-rw-rw-r-- 1 ivan ivan 4198 Oct  8 17:34 diagnostico-p06-20261008-173457.log
-rw-rw-r-- 1 ivan ivan  478 Oct  8 17:48 evidencia.md
-rw-rw-r-- 1 ivan ivan   83 Oct  8 17:48 preparacion-p06-20261008-174845.log
-rw------- 1 ivan ivan  379 Oct  8 17:57 preparacion-p06-20261008-175743-uqjau7.log
-rwxr-xr-x 1 ivan ivan 1913 Oct  8 17:33 s-01-verificar-entorno.sh
-rwxr-xr-x 1 ivan ivan 2245 Oct  8 17:57 s-02-preparar-practica.sh

Preparacion terminada. No se ha cargado ningun modulo.
```

## Parte B. Módulo mínimo

### Carga, descarga e intervalo

```text
[ 4012.175968] hola: modulo cargado
[ 4012.288511] hola: modulo descargado
```

Tiempo transcurrido: `4012.288511 − 4012.175968 = 0.112543 s` (112.543 ms). Incluye los comandos de observación entre carga y descarga; no es el costo aislado de insmod.

### ps, lsmod y Used by

`hola` no apareció como proceso independiente en `ps` y sí apareció como módulo en `lsmod`. Un módulo es código cargado en el kernel; no tiene por ello un PID propio. `ps` lista tareas, mientras `lsmod` lista módulos cargados.

`Used by` indica el número de referencias al módulo y, cuando corresponde, los módulos que dependen de él. El valor observado fue 0; no es uso de CPU. El aviso de módulo externo (out-of-tree) informa que el kernel quedó marcado como tainted por código externo, sin impedir la carga de esta prueba.

Registro completo:

```text
Registro: /unam/fse/practicas/06/ciclo-hola-20261008-181342-YSIrOA.log

=== Cargar ===
[   11.091287] Bluetooth: RFCOMM TTY layer initialized
[   11.091313] Bluetooth: RFCOMM socket layer initialized
[   11.091329] Bluetooth: RFCOMM ver 1.11
[   18.747008] bcmgenet fd580000.ethernet: configuring instance for external RGMII (RX delay)
[   18.749464] bcmgenet fd580000.ethernet eth0: Link is Down
[   18.760239] brcmfmac: brcmf_cfg80211_set_power_mgmt: power save enabled
[ 4012.174451] hola: loading out-of-tree module taints kernel.
[ 4012.175968] hola: modulo cargado

=== Modulos cargados: lsmod ===
Module                  Size  Used by
hola                   12288  0

=== sysfs ===
coresize
holders
initsize
initstate
notes
refcnt
sections
srcversion
taint
uevent

=== Procesos cuyo nombre es hola ===
    PID    PPID COMMAND         COMMAND
No hay procesos llamados hola.

=== /proc/modules ===
hola 12288 0 - Live 0x0000000000000000 (O)

=== Descargar ===
[   11.091313] Bluetooth: RFCOMM socket layer initialized
[   11.091329] Bluetooth: RFCOMM ver 1.11
[   18.747008] bcmgenet fd580000.ethernet: configuring instance for external RGMII (RX delay)
[   18.749464] bcmgenet fd580000.ethernet eth0: Link is Down
[   18.760239] brcmfmac: brcmf_cfg80211_set_power_mgmt: power save enabled
[ 4012.174451] hola: loading out-of-tree module taints kernel.
[ 4012.175968] hola: modulo cargado
[ 4012.288511] hola: modulo descargado

OK: hola ya no esta cargado.

=== Ultimos mensajes de hola y sus tiempos ===
[ 4012.175968] hola: modulo cargado
[ 4012.288511] hola: modulo descargado

Comparte el registro para calcular el intervalo y documentar resultados.
```

## Parte C. Parámetros y sysfs

### Permisos y líneas que los determinan

```text
-r--r--r-- 1 root root 4096 Oct 8 18:25 nombre
-rw-r--r-- 1 root root 4096 Oct 8 18:25 veces
```

```c
module_param(nombre, charp, 0444);
module_param(veces, int, 0644);
```

0444 permite lectura para todos; 0644 permite además escritura al propietario root. Los valores iniciales fueron nombre=mundo y veces=1. Con nombre=Pi veces=3 se imprimieron tres saludos.

### Cambio a 5 y ubicación de la variable

Se escribió `5` mediante `sudo tee /sys/module/parametros/parameters/veces`. Un proceso de usuario con privilegios solicitó la escritura; el manejador de parámetros del kernel actualizó la variable `veces`, que vive dentro del módulo en espacio de kernel. La descarga informó:

```text
[ 4682.776540] parametros: adios Pi, veces vale 5 al descargar
```

La escritura no vuelve a ejecutar la inicialización. La comprobación 1–5 de este código se hace al cargar, no automáticamente en cada escritura posterior por sysfs.

### Rechazo de veces=9

```text
insmod: ERROR: could not insert module ./parametros.ko: Invalid parameters
Codigo de salida de insmod: 1
[ 4682.856623] parametros: veces=9 fuera de rango (1 a 5)
```

“Invalid parameters” significa “parámetros no válidos”. El módulo no quedó cargado. Su función de inicialización devolvió `-EINVAL`; el código de salida del comando insmod fue 1, que no debe confundirse con el número de errno.

```text
Registro: /unam/fse/practicas/06/prueba-parametros-20261008-182506-fk7fQ3.log

=== Prueba 1: valores predeterminados ===
$ sync; sudo insmod ./parametros.ko
nombre: mundo
veces: 1
[   18.760239] brcmfmac: brcmf_cfg80211_set_power_mgmt: power save enabled
[ 4012.174451] hola: loading out-of-tree module taints kernel.
[ 4012.175968] hola: modulo cargado
[ 4012.288511] hola: modulo descargado
[ 4682.544870] parametros: hola mundo (1 de 1)

=== Prueba 2: nombre=Pi veces=3 ===
$ sync; sudo insmod ./parametros.ko nombre=Pi veces=3
[ 4682.544870] parametros: hola mundo (1 de 1)
[ 4682.626359] parametros: adios mundo, veces vale 1 al descargar
[ 4682.678890] parametros: hola Pi (1 de 3)
[ 4682.678908] parametros: hola Pi (2 de 3)
[ 4682.678911] parametros: hola Pi (3 de 3)

=== Permisos de los parametros ===
total 0
-r--r--r-- 1 root root 4096 Oct  8 18:25 nombre
-rw-r--r-- 1 root root 4096 Oct  8 18:25 veces

=== Modificar veces a 5 mediante sysfs ===
$ printf "5\n" | sudo tee /sys/module/parametros/parameters/veces
5
Valor leido despues de escribir: 5
[ 4682.626359] parametros: adios mundo, veces vale 1 al descargar
[ 4682.678890] parametros: hola Pi (1 de 3)
[ 4682.678908] parametros: hola Pi (2 de 3)
[ 4682.678911] parametros: hola Pi (3 de 3)
[ 4682.776540] parametros: adios Pi, veces vale 5 al descargar

=== Prueba 3: veces=9 (se espera rechazo) ===
$ sync; sudo insmod ./parametros.ko veces=9
insmod: ERROR: could not insert module ./parametros.ko: Invalid parameters
Codigo de salida de insmod: 1
Module                  Size  Used by
[ 4682.678890] parametros: hola Pi (1 de 3)
[ 4682.678908] parametros: hola Pi (2 de 3)
[ 4682.678911] parametros: hola Pi (3 de 3)
[ 4682.776540] parametros: adios Pi, veces vale 5 al descargar
[ 4682.856623] parametros: veces=9 fuera de rango (1 a 5)

OK: prueba terminada; parametros no esta cargado.
```

## Parte D. Dispositivo virtual p6buf

### Nodo, permisos y números

```text
crw-rw-rw- 1 root root 10, 266 Oct 8 18:37 /dev/p6buf
```

El primer carácter `c` indica un dispositivo de caracteres. 10 es el número major (clase misc) y 266 el minor asignado dinámicamente al dispositivo. Ocupan el lugar donde un archivo ordinario muestra su tamaño; no son bytes almacenados. El minor puede variar entre ejecuciones.

### Mensajes de open y write

```text
[ 5429.427457] p6buf: open (pid 4192, proceso s-08-probar-p6b)
[ 5429.427512] p6buf: write de 12 bytes (pid 4192)
[ 5429.429621] p6buf: open (pid 4216, proceso cat)
[ 5429.432568] p6buf: open (pid 4217, proceso cat)
```

El PID 4192 corresponde al shell del script que abrió y escribió mediante echo; el nombre mostrado por el kernel está truncado. Los PID 4216 y 4217 corresponden a cat, usado para lectura y comprobación. La función write de este módulo imprime PID y cantidad, pero no vuelve a imprimir el nombre del proceso: se relaciona con el mensaje de open del mismo PID.

### Escritura de 300 bytes

```text
tr: write error: No space left on device
write(1, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"..., 300) = -1 ENOSPC (No space left on device)
```

El driver devolvió `-ENOSPC` (errno 28, “no hay espacio disponible”). El comando strace/tr terminó con código 1. El buffer siguió conteniendo `hola kernel` con salto de línea, 12 bytes: se rechaza el tamaño antes de copiar. El error se refiere a la capacidad de 256 bytes del driver.

```text
Registro: /unam/fse/practicas/06/prueba-p6buf-20261008-183733-ErU86c.log
Traza: /unam/fse/practicas/06/prueba-p6buf-20261008-183733-ErU86c-300bytes.strace

=== Cargar p6buf ===
[ 4682.678908] parametros: hola Pi (2 de 3)
[ 4682.678911] parametros: hola Pi (3 de 3)
[ 4682.776540] parametros: adios Pi, veces vale 5 al descargar
[ 4682.856623] parametros: veces=9 fuera de rango (1 a 5)
[ 5429.374136] p6buf: /dev/p6buf creado, minor 266

=== Archivo de dispositivo y registro misc ===
crw-rw-rw- 1 root root 10, 266 Oct  8 18:37 /dev/p6buf
266 p6buf

=== Escribir y leer hola kernel ===
$ echo "hola kernel" > /dev/p6buf
$ cat /dev/p6buf
hola kernel
Bytes almacenados: 12
[ 4682.678911] parametros: hola Pi (3 de 3)
[ 4682.776540] parametros: adios Pi, veces vale 5 al descargar
[ 4682.856623] parametros: veces=9 fuera de rango (1 a 5)
[ 5429.374136] p6buf: /dev/p6buf creado, minor 266
[ 5429.427457] p6buf: open (pid 4192, proceso s-08-probar-p6b)
[ 5429.427512] p6buf: write de 12 bytes (pid 4192)
[ 5429.429621] p6buf: open (pid 4216, proceso cat)
[ 5429.432568] p6buf: open (pid 4217, proceso cat)

=== Intentar escribir 300 bytes ===
tr: write error: No space left on device
Codigo de salida de head: 0
Codigo de salida de strace/tr: 1

=== Traza de write y errno ===
write(1, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"..., 300) = -1 ENOSPC (No space left on device)
write(2, "tr: ", 4)                     = 4
write(2, "write error", 11)             = 11
write(2, ": No space left on device", 25) = 25
write(2, "\n", 1)                       = 1
+++ exited with 1 +++

=== Contenido despues del rechazo ===
hola kernel
OK: el contenido no cambio.
[ 4682.856623] parametros: veces=9 fuera de rango (1 a 5)
[ 5429.374136] p6buf: /dev/p6buf creado, minor 266
[ 5429.427457] p6buf: open (pid 4192, proceso s-08-probar-p6b)
[ 5429.427512] p6buf: write de 12 bytes (pid 4192)
[ 5429.429621] p6buf: open (pid 4216, proceso cat)
[ 5429.432568] p6buf: open (pid 4217, proceso cat)
[ 5429.478336] p6buf: open (pid 4225, proceso s-08-probar-p6b)
[ 5429.574390] p6buf: open (pid 4230, proceso cat)

OK: pruebas terminadas. p6buf queda cargado para las partes E y F.
Para descargarlo si terminas la sesion: sudo rmmod p6buf
```

## Parte E. Cliente y comparación con IPC

### Traza del descriptor de p6buf

```text
openat(AT_FDCWD</unam/fse/practicas/06>, "/dev/p6buf", O_RDWR) = 3</dev/p6buf<char 10:266>>
write(3</dev/p6buf<char 10:266>>, "hola desde espacio de usuario", 29) = 29
lseek(3</dev/p6buf<char 10:266>>, 0, SEEK_SET) = 0
read(3</dev/p6buf<char 10:266>>, "hola desde espacio de usuario", 256) = 29
close(3</dev/p6buf<char 10:266>>)       = 0
```

`openat` devuelve el descriptor 3; write devuelve 29 bytes; lseek devuelve posición 0; read devuelve 29 bytes; close devuelve 0 (éxito).

### Relación con dmesg

```text
[ 5788.260080] p6buf: open (pid 4289, proceso cliente)
[ 5788.260181] p6buf: write de 29 bytes (pid 4289)
```

La llamada openat produjo el mensaje de open y write produjo el mensaje de 29 bytes. Las funciones de lectura, búsqueda de posición y cierre no imprimen un mensaje propio en este driver; por eso no se espera una línea de dmesg por cada syscall.

### pipe y fork en demo_ipc

Las llamadas observadas fueron `pipe2` para la tubería y `clone` para la implementación de fork de la biblioteca. La tubería del intercambio fue 18767; padre 4352 e hijo 4353. Se transmitieron 29 bytes y el padre esperó con wait4.

Ambos programas usan llamadas al sistema para pedir servicios y transferir datos mediante descriptores. En demo_ipc el trabajo corresponde al código de tuberías y gestión de procesos de Linux; en cliente, VFS dirige las operaciones a las funciones registradas por nuestro módulo p6buf.

Traza real del IPC:

```text
4352  close(3</etc/ld.so.cache>)        = 0
4352  read(3</usr/lib/aarch64-linux-gnu/libc.so.6>, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0\267\0\1\0\0\0\200$\2\0\0\0\0\0"..., 832) = 832
4352  close(3</usr/lib/aarch64-linux-gnu/libc.so.6>) = 0
4352  pipe2([3<pipe:[18767]>, 4<pipe:[18767]>], 0) = 0
4352  clone(child_stack=NULL, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD, child_tidptr=0x7fa1c090f0) = 4353
4352  close(3<pipe:[18767]>)            = 0
4352  write(4<pipe:[18767]>, "hola desde espacio de usuario", 29) = 29
4353  close(4<pipe:[18767]> <unfinished ...>
4352  close(4<pipe:[18767]> <unfinished ...>
4353  <... close resumed>)              = 0
4352  <... close resumed>)              = 0
4353  read(3<pipe:[18767]>,  <unfinished ...>
4352  wait4(4353,  <unfinished ...>
4353  <... read resumed>"hola desde espacio de usuario", 256) = 29
4353  close(3<pipe:[18767]>)            = 0
4353  write(1<pipe:[19699]>, "el hijo recibio 29 bytes: hola d"..., 56) = 56
4353  +++ exited with 0 +++
4352  <... wait4 resumed>[{WIFEXITED(s) && WEXITSTATUS(s) == 0}], 0, NULL) = 4353
4352  --- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_EXITED, si_pid=4353, si_uid=1000, si_status=0, si_utime=0, si_stime=0} ---
4352  +++ exited with 0 +++
```

## Parte F. Costo de cruzar al kernel

### Tres corridas completas


```text
Registro: /unam/fse/practicas/06/tiempos-p06-20261008-185730-o31Cqj.log

=== Preparar contenido del driver ===
el driver devolvio 29 bytes: hola desde espacio de usuario

=== Fuente utilizada ===
/*
 * @Autor       Ivan E. Nicolas Montesinos
 * @Fecha       08/10/2026
 * @Descripcion Mide memoria, syscall, pread y SPI opcional.
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#define N 1000000
#define N_SPI 10000
static double ahora_ns(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) == -1) {
        perror("clock_gettime"); return -1;
    }
    return t.tv_sec * 1e9 + t.tv_nsec;
}
int main(void)
{
    static volatile char memoria = 'x';
    char c = 0;
    int fd = open("/dev/p6buf", O_RDONLY);
    double t0, t1, t2, t3;
    if (fd == -1) { perror("open /dev/p6buf"); return 1; }
    t0 = ahora_ns();
    for (int i = 0; i < N; i++) c = memoria;
    t1 = ahora_ns();
    for (int i = 0; i < N; i++) {
        if (syscall(SYS_getppid) == -1) {
            perror("getppid"); close(fd); return 1;
        }
    }
    t2 = ahora_ns();
    for (int i = 0; i < N; i++) {
        ssize_t n = pread(fd, &c, 1, 0);
        if (n != 1) {
            if (n == -1) perror("pread");
            else fprintf(stderr, "pread no devolvio un byte; revise el contenido\n");
            close(fd); return 1;
        }
    }
    t3 = ahora_ns();
    close(fd);
    if (t0 < 0 || t1 < 0 || t2 < 0 || t3 < 0) return 1;
    printf("lectura de memoria de usuario: %.1f ns por operacion\n", (t1-t0)/N);
    printf("syscall getppid:               %.1f ns por operacion\n", (t2-t1)/N);
    printf("pread sobre /dev/p6buf:        %.1f ns por operacion\n", (t3-t2)/N);
    (void)c;
    int fs = open("/dev/spidev0.0", O_RDWR);
    if (fs == -1) {
        perror("open /dev/spidev0.0");
        puts("SPI omitido: dispositivo ausente o sin acceso."); return 0;
    }
    unsigned char tx[3] = {1, 8 << 4, 0}, rx[3] = {0};
    struct spi_ioc_transfer tr = {
        .tx_buf = (uintptr_t)tx, .rx_buf = (uintptr_t)rx,
        .len = 3, .speed_hz = 1000000, .bits_per_word = 8
    };
    double inicio = ahora_ns();
    for (int i = 0; i < N_SPI; i++) {
        int n = ioctl(fs, SPI_IOC_MESSAGE(1), &tr);
        if (n != 3) {
            if (n == -1) perror("ioctl SPI_IOC_MESSAGE");
            else fprintf(stderr, "Transferencia SPI incompleta: %d bytes\n", n);
            puts("SPI sin medicion valida: no se completaron las 10000 transferencias.");
            close(fs); return 0;
        }
    }
    double fin = ahora_ns();
    close(fs);
    if (inicio < 0 || fin < 0) return 1;
    printf("transferencia SPI de 3 bytes:  %.1f ns por operacion\n", (fin-inicio)/N_SPI);
    puts("SPI mide transferencias del bus; no verifica la presencia del MCP3008.");
    return 0;
}

=== Compilar ===

=== Corrida 1 de 3 ===
lectura de memoria de usuario: 1.1 ns por operacion
syscall getppid:               444.1 ns por operacion
pread sobre /dev/p6buf:        606.7 ns por operacion
transferencia SPI de 3 bytes:  36034.2 ns por operacion
SPI mide transferencias del bus; no verifica la presencia del MCP3008.

=== Corrida 2 de 3 ===
lectura de memoria de usuario: 1.1 ns por operacion
syscall getppid:               451.9 ns por operacion
pread sobre /dev/p6buf:        605.0 ns por operacion
transferencia SPI de 3 bytes:  35979.3 ns por operacion
SPI mide transferencias del bus; no verifica la presencia del MCP3008.

=== Corrida 3 de 3 ===
lectura de memoria de usuario: 1.1 ns por operacion
syscall getppid:               449.2 ns por operacion
pread sobre /dev/p6buf:        614.9 ns por operacion
transferencia SPI de 3 bytes:  35906.0 ns por operacion
SPI mide transferencias del bus; no verifica la presencia del MCP3008.

OK: tres corridas registradas.
```

### Medianas

| Operación | Corrida 1 (ns/op) | Corrida 2 | Corrida 3 | Mediana |
|---|---:|---:|---:|---:|
| Memoria de usuario | 1.1 | 1.1 | 1.1 | 1.1 |
| syscall getppid | 444.1 | 451.9 | 449.2 | 449.2 |
| pread de 1 byte en p6buf | 606.7 | 605.0 | 614.9 | 606.7 |
| SPI, transferencia de 3 bytes | 36034.2 | 35979.3 | 35906.0 | 35979.3 |

Se ejecutaron 1000000 operaciones para memoria/getppid/pread y 10000 transferencias SPI por corrida, a 1 MHz. Son promedios por operación; se calculó la mediana de los tres promedios.

### ¿Por qué memoria es volatile?

`volatile` obliga al compilador a conservar los accesos observables a esa variable según las reglas del lenguaje y evita sustituir las lecturas repetidas por un único valor constante. No desactiva la caché, no mide por sí solo RAM externa y no sustituye mutex ni garantiza sincronización entre hilos.

### 100000 lecturas por segundo

`100000 × 606.7 ns = 60670000 ns = 60.67 ms`.

Fracción de un segundo: `60.67 / 1000 × 100 = 6.067 %`. Es una estimación basada en tiempo transcurrido; no una medición directa del porcentaje de CPU.

### Tasa SPI y comparación

`1000000000 / 35979.3 ≈ 27793.76` transferencias por segundo; `35979.3 / 606.7 ≈ 59.30` veces el tiempo de pread.

La primera cifra estima un techo basado únicamente en transferencias consecutivas de este ensayo; una aplicación real puede ser más lenta. Como no había MCP3008 conectado, se midió el bus y no una tasa de conversiones válidas de un sensor. No se atribuyen valores de voltaje a esta prueba.

## Parte G. Programa y drivers

### Adaptación sin circuito

sensor_combinado abrió `/dev/i2c-1`, seleccionó la dirección 0x76 y falló con EIO al acceder a la calibración. Terminó antes de abrir SPI o GPIO. Por tanto, se hicieron trazas separadas de bmp280_id, leer_pot y gpiodetect para completar la identificación. No se presenta esta tabla como si los tres dispositivos hubieran sido abiertos por una ejecución exitosa del programa combinado.

| Programa observado | Ruta abierta | ioctl principal observado | Major | Minor | Driver/interfaz |
|---|---|---|---:|---:|---|
| sensor_combinado y bmp280_id | /dev/i2c-1 | I2C_SLAVE, dirección 0x76 | 89 | 1 | i2c_dev; controlador i2c_bcm2835 |
| leer_pot | /dev/spidev0.0 | Configuración SPI y SPI_IOC_MESSAGE | 153 | 0 | spidev; controlador spi_bcm2835 |
| gpiodetect | /dev/gpiochip0 | GPIO_GET_CHIPINFO_IOCTL | 254 | 0 | pinctrl-bcm2835, etiqueta pinctrl-bcm2711 |
| gpiodetect | /dev/gpiochip1 | GPIO_GET_CHIPINFO_IOCTL | 254 | 1 | raspberrypi-exp-gpio |

`/dev/spidev0.1` (153:1) se identificó en el inventario, no se atribuye su apertura al programa medido. gpiochip4 era un enlace a gpiochip0; stat -L permitió examinar el dispositivo de destino. No se accionaron líneas GPIO.

### lsmod y sysfs

En lsmod se observaron i2c_dev, i2c_bcm2835, i2c_brcmstb, spidev, spi_bcm2835 y raspberrypi_gpiomem, además del módulo p6buf durante la sesión. El controlador GPIO principal pinctrl-bcm2835 está integrado y no aparece como módulo cargable: la vinculación en sysfs para fe200000.gpio y la información devuelta por gpiodetect comprueban su presencia. No se debe confundir raspberrypi_gpiomem con el driver de gpiochip0.

### /sys/bus/i2c/devices y BMP280

Se observaron adaptadores i2c-1, i2c-20 e i2c-21. No se encontró un cliente instanciado como 1-0076. En esta ejecución no se demostró un driver del kernel vinculado al BMP280. Los programas de usuario implementan la selección de registros y la interpretación de datos del sensor a través de i2c-dev; el controlador del kernel realiza las transferencias del bus. La ausencia de un cliente instanciado no prueba que el kernel carezca de soporte compilado o instalable para BMP280.

En p6buf nuestro código del módulo define el significado de read y write sobre el buffer; con i2c-dev el kernel proporciona operaciones genéricas del bus y el programa de usuario decide qué registros del BMP280 consultar y cómo interpretarlos.

### Registros de identificación y trazas


```text
Registro: /unam/fse/practicas/06/drivers-p06-20261008-190020-VfKPi8.log

=== Servicio anterior ===
inactive
disabled

=== Dispositivos y drivers asociados ===
crw-rw---- 1 root i2c 89, 1 Oct  3 17:04 /dev/i2c-1
major=89 minor=1
/sys/devices/platform/soc/fe804000.i2c/i2c-1/i2c-dev/i2c-1
crw-rw---- 1 root i2c 89, 20 Oct  3 17:04 /dev/i2c-20
major=89 minor=20
/sys/devices/platform/soc/fef04500.i2c/i2c-20/i2c-dev/i2c-20
crw-rw---- 1 root i2c 89, 21 Oct  3 17:04 /dev/i2c-21
major=89 minor=21
/sys/devices/platform/soc/fef09500.i2c/i2c-21/i2c-dev/i2c-21
crw-rw---- 1 root spi 153, 0 Oct  3 17:04 /dev/spidev0.0
major=153 minor=0
/sys/devices/platform/soc/fe204000.spi/spi_master/spi0/spi0.0/spidev/spidev0.0
/sys/bus/spi/drivers/spidev
/sys/module/spidev
crw-rw---- 1 root spi 153, 1 Oct  3 17:04 /dev/spidev0.1
major=153 minor=1
/sys/devices/platform/soc/fe204000.spi/spi_master/spi0/spi0.1/spidev/spidev0.1
/sys/bus/spi/drivers/spidev
/sys/module/spidev
crw-rw----+ 1 root gpio 254, 0 Oct  3 17:04 /dev/gpiochip0
major=254 minor=0
/sys/devices/platform/soc/fe200000.gpio/gpiochip0
crw-rw----+ 1 root gpio 254, 1 Oct  3 17:04 /dev/gpiochip1
major=254 minor=1
/sys/devices/platform/soc/soc:firmware/soc:firmware:gpio/gpiochip1
lrwxrwxrwx 1 root root 9 Oct  3 17:04 /dev/gpiochip4 -> gpiochip0
major=0 minor=0
/sys/dev/char/0:0
crw-rw-rw- 1 root root 10, 266 Oct  8 18:37 /dev/p6buf
major=10 minor=266
/sys/devices/virtual/misc/p6buf

=== Modulos relacionados ===
Module                  Size  Used by
p6buf                  12288  0
spidev                 20480  0
raspberrypi_gpiomem    12288  0
spi_bcm2835            20480  0
i2c_dev                16384  0
i2c_brcmstb            12288  0
i2c_bcm2835            16384  0

=== GPIO y version de libgpiod ===
gpiochip0 [pinctrl-bcm2711] (58 lines)
gpiochip1 [raspberrypi-exp-gpio] (8 lines)
2.2.1

=== Dispositivos I2C en sysfs ===
total 0
lrwxrwxrwx 1 root root 0 Oct  3 17:04 i2c-1 -> ../../../devices/platform/soc/fe804000.i2c/i2c-1
lrwxrwxrwx 1 root root 0 Oct  3 17:04 i2c-20 -> ../../../devices/platform/soc/fef04500.i2c/i2c-20
lrwxrwxrwx 1 root root 0 Oct  3 17:04 i2c-21 -> ../../../devices/platform/soc/fef09500.i2c/i2c-21

=== Drivers de plataforma ===
/sys/bus/platform/drivers/brcmstb-i2c
/sys/bus/platform/drivers/i2c-bcm2835
/sys/bus/platform/drivers/pinctrl-bcm2712
/sys/bus/platform/drivers/pinctrl-bcm2835
/sys/bus/platform/drivers/pinctrl-rp1
/sys/bus/platform/drivers/spi-bcm2835

=== Fuentes existentes (no se ejecutan) ===

--- /unam/fse/practicas/03/leer_pot.c ---
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

int main(void) {
  int fd = open("/dev/spidev0.0", O_RDWR);

  if (fd < 0) {
    perror("open");
    return 1;
  }

  unsigned char mode = SPI_MODE_0;
  unsigned char bits = 8;
  unsigned int speed = 1000000;

  if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
    perror("SPI_IOC_WR_MODE");
    return 1;
  }

  if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
    perror("SPI_IOC_WR_BITS_PER_WORD");
    return 1;
  }

  if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
    perror("SPI_IOC_WR_MAX_SPEED_HZ");
    return 1;
  }

  while (1) {
    unsigned char canal = 0;

    unsigned char tx[3] = {
      0x01,
      (8 + canal) << 4,
      0x00
    };

    unsigned char rx[3] = {0};

    struct spi_ioc_transfer tr = {
      .tx_buf = (unsigned long)tx,
      .rx_buf = (unsigned long)rx,
      .len = 3,
      .speed_hz = speed,
      .bits_per_word = bits
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
      perror("SPI_IOC_MESSAGE");
      return 1;
    }

    int valor = ((rx[1] & 0x03) << 8) | rx[2];

    float voltaje = (valor * 3.3f) / 1023.0f;

    printf("Valor: %4d   Voltaje: %.2f V\n",
           valor, voltaje);

    usleep(300000);
  }

  close(fd);
  return 0;
}

--- /unam/fse/practicas/03/sensor_combinado.c ---
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

--- /unam/fse/practicas/03/bmp280_id.c ---
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

Fuentes encontradas: 3
OK: consulta terminada.
```

```text
Registro: /unam/fse/practicas/06/trazas-drivers-p06-20261008-190738-XqsURB.log

=== Compilar copias de ejecutables en la practica 6 ===

=== Traza: sensor_combinado ===
calibracion BME280: Input/output error
Codigo de salida: 1
Traza guardada: /unam/fse/practicas/06/trazas-drivers-p06-20261008-190738-XqsURB-sensor_combinado.strace

Operaciones sobre dispositivos:
4598  openat(AT_FDCWD</unam/fse/practicas/06>, "/dev/i2c-1", O_RDWR) = 3</dev/i2c-1<char 89:1>>
4598  ioctl(3</dev/i2c-1<char 89:1>>, _IOC(_IOC_NONE, 0x7, 0x3, 0), 0x76) = 0
4598  write(3</dev/i2c-1<char 89:1>>, "\210", 1) = -1 EIO (Input/output error)
4598  close(3</dev/i2c-1<char 89:1>>)   = 0

=== Traza: bmp280_id ===
write: Input/output error
Codigo de salida: 1
Traza guardada: /unam/fse/practicas/06/trazas-drivers-p06-20261008-190738-XqsURB-bmp280_id.strace

Operaciones sobre dispositivos:
4609  openat(AT_FDCWD</unam/fse/practicas/06>, "/dev/i2c-1", O_RDWR) = 3</dev/i2c-1<char 89:1>>
4609  ioctl(3</dev/i2c-1<char 89:1>>, _IOC(_IOC_NONE, 0x7, 0x3, 0), 0x76) = 0
4609  write(3</dev/i2c-1<char 89:1>>, "\320", 1) = -1 EIO (Input/output error)
4609  close(3</dev/i2c-1<char 89:1>>)   = 0

=== Traza: leer_pot ===
Codigo de salida: 124
124: se alcanzo el limite de tiempo del experimento.
Traza guardada: /unam/fse/practicas/06/trazas-drivers-p06-20261008-190738-XqsURB-leer_pot.strace

Operaciones sobre dispositivos:
4620  openat(AT_FDCWD</unam/fse/practicas/06>, "/dev/spidev0.0", O_RDWR) = 3</dev/spidev0.0<char 153:0>>
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_WR_MODE, 0x7ff9e2048e) = 0
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_WR_BITS_PER_WORD, 0x7ff9e2048f) = 0
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_WR_MAX_SPEED_HZ, 0x7ff9e2049c) = 0
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_MESSAGE(32), 0x7ff9e204a0) = 3
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_MESSAGE(32), 0x7ff9e204a0) = 3
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_MESSAGE(32), 0x7ff9e204a0) = 3
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_MESSAGE(32), 0x7ff9e204a0) = 3
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_MESSAGE(32), 0x7ff9e204a0) = 3
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_MESSAGE(32), 0x7ff9e204a0) = 3
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_MESSAGE(32), 0x7ff9e204a0) = 3
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_MESSAGE(32), 0x7ff9e204a0) = 3
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_MESSAGE(32), 0x7ff9e204a0) = 3
4620  ioctl(3</dev/spidev0.0<char 153:0>>, SPI_IOC_MESSAGE(32), 0x7ff9e204a0) = 3

=== Traza: gpio-consulta ===
gpiochip0 [pinctrl-bcm2711] (58 lines)
gpiochip1 [raspberrypi-exp-gpio] (8 lines)
Codigo de salida: 0
Traza guardada: /unam/fse/practicas/06/trazas-drivers-p06-20261008-190738-XqsURB-gpio-consulta.strace

Operaciones sobre dispositivos:
4626  openat(AT_FDCWD</unam/fse/practicas/06>, "/dev/", O_RDONLY|O_NONBLOCK|O_CLOEXEC|O_DIRECTORY) = 3</dev>
4626  openat(AT_FDCWD</unam/fse/practicas/06>, "/dev/gpiochip0", O_RDWR|O_CLOEXEC) = 3</dev/gpiochip0<char 254:0>>
4626  ioctl(3</dev/gpiochip0<char 254:0>>, GPIO_GET_CHIPINFO_IOCTL, {name="gpiochip0", label="pinctrl-bcm2711", lines=58}) = 0
4626  close(3</dev/gpiochip0<char 254:0>>) = 0
4626  openat(AT_FDCWD</unam/fse/practicas/06>, "/dev/gpiochip1", O_RDWR|O_CLOEXEC) = 3</dev/gpiochip1<char 254:1>>
4626  ioctl(3</dev/gpiochip1<char 254:1>>, GPIO_GET_CHIPINFO_IOCTL, {name="gpiochip1", label="raspberrypi-exp-gpio", lines=8}) = 0
4626  close(3</dev/gpiochip1<char 254:1>>) = 0

=== Numeros y enlaces corregidos: seguir enlaces simbolicos ===
/dev/i2c-1: tipo=character special file major(hex)=59 minor(hex)=1
/dev/spidev0.0: tipo=character special file major(hex)=99 minor(hex)=0
/dev/gpiochip0: tipo=character special file major(hex)=fe minor(hex)=0
/dev/gpiochip4: tipo=character special file major(hex)=fe minor(hex)=0

=== Drivers vinculados al hardware de esta Pi ===
/sys/class/spidev/spidev0.0/device/driver -> /sys/bus/spi/drivers/spidev
/sys/module/spidev
/sys/bus/platform/devices/fe804000.i2c/driver -> /sys/bus/platform/drivers/i2c-bcm2835
/sys/module/i2c_bcm2835
/sys/bus/platform/devices/fe204000.spi/driver -> /sys/bus/platform/drivers/spi-bcm2835
/sys/module/spi_bcm2835
/sys/bus/platform/devices/fe200000.gpio/driver -> /sys/bus/platform/drivers/pinctrl-bcm2835

Consulta terminada; revise los codigos y las trazas antes de concluir.
```

## Reto final. p6cnt

Se eligió EINVAL (errno 22, “argumento no válido”) para órdenes distintas de reset.
La escritura llega a un dispositivo válido, pero su contenido no pertenece al conjunto de órdenes admitidas; por ello EINVAL describe el error de argumento y no un problema de espacio o permisos.

Las aperturas para escribir también incrementan el contador, aunque después write rechace la orden. Tras un reset, el siguiente cat muestra 1 porque su propia apertura se incluye. El total histórico fue 9 y el contador desde el último reset fue 3.

La referencia no_llseek se retiró por incompatibilidad con el kernel actual; la versión corregida compiló y se probó.

```text
Registro: /unam/fse/practicas/06/prueba-p6cnt-20261008-191955-e1OSXr.log
Traza: /unam/fse/practicas/06/prueba-p6cnt-20261008-191955-e1OSXr-rechazo.strace

=== Cargar ===
crw-rw-rw- 1 root root 10, 267 Oct  8 19:20 /dev/p6cnt

=== Tres lecturas consecutivas ===
aperturas: 1
aperturas: 2
aperturas: 3

=== Reset con salto de linea y lectura ===
aperturas: 1

=== Reset sin salto de linea y lectura ===
aperturas: 1

=== Rechazar otra orden ===
bash: line 1: printf: write error: Invalid argument
Codigo de salida del comando: 1
write(1, "hola\n", 5)                   = -1 EINVAL (Invalid argument)
write(2, "bash: line 1: printf: write erro"..., 52) = 52
+++ exited with 1 +++

=== Lectura posterior al rechazo ===
La apertura para escribir tambien incremento el contador.
aperturas: 3

=== Descargar y comprobar ===
/dev/p6cnt ya no existe; p6cnt no esta cargado.
[ 7981.487005] p6cnt: /dev/p6cnt creado, minor 267
[ 7981.506748] p6cnt: contador reiniciado
[ 7981.510843] p6cnt: contador reiniciado
[ 7981.559852] p6cnt: /dev/p6cnt eliminado; total de aperturas: 9; desde reset: 3

OK: contador, ambos resets, rechazo y descarga verificados.
```

## Parte H. Descarga final

Se utilizó `s-16-descargar-modulos.sh` como nombre adaptado de `descargar.sh`. Revisa el orden inverso y tolera módulos ya ausentes. Salida completa:

```text
Registro: /unam/fse/practicas/06/descarga-p06-20261008-192334-ovEurU.log

=== Descargar en orden inverso ===
No estaba cargado: p6cnt
Descargado: p6buf
No estaba cargado: parametros
No estaba cargado: hola

=== Comprobar estado final ===
Ningun modulo de la practica 6 sigue cargado.
No existen /dev/p6buf ni /dev/p6cnt.

=== Ultimos mensajes de los modulos ===
[ 5788.260080] p6buf: open (pid 4289, proceso cliente)
[ 5788.260181] p6buf: write de 29 bytes (pid 4289)
[ 6626.335404] p6buf: open (pid 4449, proceso cliente)
[ 6626.335449] p6buf: write de 29 bytes (pid 4449)
[ 6626.636914] p6buf: open (pid 4457, proceso tiempo)
[ 6628.051180] p6buf: open (pid 4458, proceso tiempo)
[ 6629.470902] p6buf: open (pid 4459, proceso tiempo)
[ 7981.487005] p6cnt: /dev/p6cnt creado, minor 267
[ 7981.506748] p6cnt: contador reiniciado
[ 7981.510843] p6cnt: contador reiniciado
[ 7981.559852] p6cnt: /dev/p6cnt eliminado; total de aperturas: 9; desde reset: 3
[ 8190.413758] p6buf: /dev/p6buf eliminado

OK: descarga y estado final verificados.
```
