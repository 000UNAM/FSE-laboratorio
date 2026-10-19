# Práctica 7 — C embebido

## Verificación inicial del circuito

- Cuatro LEDs: funcionamiento comprobado.
- Dos botones: funcionamiento comprobado.
- Potenciómetro mediante MCP3008: valores observados de 0 a 1023.
- Voltaje estimado: de 0.00 a 3.30 V.
- Sensor BME280: identificador leído 0x60.
- Calibración: T1=27893, T2=26742, T3=50.
- Temperaturas observadas: de 21.08 a 21.39 °C.

Estas pruebas iniciales se realizaron antes de las actividades en C.

## Parte A — Preparación del entorno

```text
gcc (Debian 14.2.0-19) 14.2.0
GNU Make 4.4.1
aarch64
64
ivan adm dialout cdrom sudo audio video plugdev games users netdev lpadmin gpio i2c spi render input
crw-rw---- 1 root gpio 237, 0 Oct 15 20:44 /dev/gpiomem
crw-rw---- 1 root i2c   89, 1 Oct 15 20:44 /dev/i2c-1
crw-rw---- 1 root spi  153, 0 Oct 15 20:44 /dev/spidev0.0
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- 76 --                         
```
## Parte B — Tipos y representación en memoria

### Predicciones
Para el entorno aarch64 de 64 bits:
- sizeof(long): 8 bytes.
- 200 + 100 calculado como int: 300.
- Al convertir 300 a uint8_t: 44, porque 300 módulo 256 = 44.
- ~reg == 0xF0, con reg = 0x0F: falso.
- (uint8_t)~reg == 0xF0: verdadero.
- Estructura normal: 12 bytes; valor en desplazamiento 4 y crc en 8.
- Estructura compacta: 6 bytes; valor en desplazamiento 1 y crc en 5.
- Bytes de 0x01020304 en orden de memoria: 04 03 02 01.


### Compilación y resultado

```text
tipos.c: In function ‘main’:
tipos.c:51:39: warning: comparison of promoted bitwise complement of an unsigned value with constant [-Wsign-compare]
   51 |     printf("~reg == 0xF0: %d\n", ~reg == 0xF0);
      |                                       ^~
=== Tamaños en bytes ===
char:      1
short:     2
int:       4
long:      8
long long: 8
void *:    8
uint8_t:   1
uint16_t:  2
uint32_t:  4
uint64_t:  8

INT_MAX:    2147483647
UINT32_MAX: 4294967295

=== Promociones y conversiones ===
Suma como int: 300
Suma convertida a uint8_t: 44
~reg: 0xFFFFFFF0
(uint8_t)~reg: 0xF0
~reg == 0xF0: 0
(uint8_t)~reg == 0xF0: 1

=== Estructuras ===
Normal: tamaño=12, valor=4, crc=8
Compacta: tamaño=6, valor=1, crc=5

=== Orden de bytes en memoria ===
Palabra: 0x01020304
Bytes: 04 03 02 01
```

### Análisis de la parte B

Los resultados coincidieron con las predicciones.
La suma de dos uint8_t se realiza después de promoverlos a int:
200 + 100 produce 300. Al convertir a uint8_t se obtiene 44,
porque la conversión conserva el resultado módulo 256.

El operador ~ también promueve el operando a int. En este
entorno, ~0x0F produce -16, mostrado como 0xFFFFFFF0 al convertirlo
a unsigned int. Al convertirlo a uint8_t se obtiene 0xF0.
Por ello las comparaciones dieron 0 y 1, respectivamente.
El compilador advirtió sobre la primera comparación.

La estructura normal ocupó 12 bytes por la alineación y el
relleno: 3 bytes antes de valor y 3 al final. La estructura
packed ocupó 6 bytes. El relleno no tiene por qué contener ceros.

Los bytes 04 03 02 01 muestran el orden little-endian:
el byte menos significativo se almacena en la dirección menor.

## Parte C — Bits y máscaras

### Predicciones antes de ejecutar

- SET 17: 0x00020000.
- SET 3: 0x00020008.
- TOGGLE 17: 0x00000008.
- CLEAR 3: 0x00000000.
- TEST 17: 0.
- GPIO17: GPFSEL1, desplazamiento 21, máscara 0x00E00000.
- GPIO22: GPFSEL2, desplazamiento 6, máscara 0x000001C0.
- Desde 0xFFFFFFFF, configurar GPIO17 como salida dará 0xFF3FFFFF.
- Desde 0xFFFFFFFF, configurar GPIO22 como salida dará 0xFFFFFE7F.
- Al borrar el campo y escribir 001, el campo resultante será 1.
- Si solo usamos OR, el campo permanecerá en 111, equivalente a 7.

### Compilación y ejecución

```text
Compilando bits.c...
gcc -std=gnu11 -Wall -Wextra -Wconversion -O0 -g -o bits bits.c
Ejecutando ./bits...
=== Operaciones sobre bits ===
Inicial = 0x00000000  00000000_00000000_00000000_00000000
SET 17 = 0x00020000  00000000_00000010_00000000_00000000
SET 3 = 0x00020008  00000000_00000010_00000000_00001000
TOGGLE 17 = 0x00000008  00000000_00000000_00000000_00001000
CLEAR 3 = 0x00000000  00000000_00000000_00000000_00000000
TEST 17: 0

=== Campos de configuracion GPIO ===
GPIO17: GPFSEL1, desplazamiento=21, mascara=0x00E00000
GPIO27: GPFSEL2, desplazamiento=21, mascara=0x00E00000
GPIO22: GPFSEL2, desplazamiento= 6, mascara=0x000001C0
GPIO23: GPFSEL2, desplazamiento= 9, mascara=0x00000E00
GPIO 5: GPFSEL0, desplazamiento=15, mascara=0x00038000
GPIO 6: GPFSEL0, desplazamiento=18, mascara=0x001C0000

GPIO17: GPFSEL1, desplazamiento=21
Mascara = 0x00E00000  00000000_11100000_00000000_00000000
Inicial = 0xFFFFFFFF  11111111_11111111_11111111_11111111
Borrar campo y poner 001 = 0xFF3FFFFF  11111111_00111111_11111111_11111111
Campo resultante: 1
Solo OR, sin borrar = 0xFFFFFFFF  11111111_11111111_11111111_11111111
Campo con solo OR: 7

GPIO22: GPFSEL2, desplazamiento=6
Mascara = 0x000001C0  00000000_00000000_00000001_11000000
Inicial = 0xFFFFFFFF  11111111_11111111_11111111_11111111
Borrar campo y poner 001 = 0xFFFFFE7F  11111111_11111111_11111110_01111111
Campo resultante: 1
Solo OR, sin borrar = 0xFFFFFFFF  11111111_11111111_11111111_11111111
Campo con solo OR: 7

```

### Análisis de bits y máscaras

Los resultados coincidieron con las predicciones.
SET usa OR para activar bits; CLEAR usa AND con la máscara
invertida para borrarlos; TOGGLE usa XOR para invertirlos.
TEST comprueba si el bit seleccionado está activo.

Para configurar un campo de tres bits primero se borran sus
bits anteriores. Usar únicamente OR dejó el campo en 7,
mientras que borrar y escribir produjo el valor 1.
Las operaciones se realizaron sobre variables, no sobre hardware.

## Parte C — Desbordamiento

### Predicciones antes de ejecutar

- Sin argumentos, int comienza en INT_MAX - 2.
- En el tercer incremento se intenta superar INT_MAX:
  esto es comportamiento indefinido para un entero con signo.
- UBSan debe detectar ese desbordamiento.
- No hay un resultado posterior garantizado por C para int.
- uint32_t tiene aritmética módulo 2^32.
- Con entrada -6, la conversión a uint32_t produce 4294967290.
- En esa segunda prueba, uint32_t pasa de 4294967295 a 0
  en el sexto incremento; int pasa de -1 a 0 sin desbordarse.

### Compilación y resultados

```text
=== Compilación normal ===
gcc -std=gnu11 -Wall -Wextra -Wconversion -O0 -g -o desborde desborde.c
=== Ejecución normal desde INT_MAX - 2 ===
Inicial: int=2147483645, uint32_t=2147483645
Paso  1: int=  2147483646, uint32_t=2147483646
Paso  2: int=  2147483647, uint32_t=2147483647
Paso  3: int= -2147483648, uint32_t=2147483648
Paso  4: int= -2147483647, uint32_t=2147483649
Paso  5: int= -2147483646, uint32_t=2147483650
Paso  6: int= -2147483645, uint32_t=2147483651
Paso  7: int= -2147483644, uint32_t=2147483652
Paso  8: int= -2147483643, uint32_t=2147483653
Paso  9: int= -2147483642, uint32_t=2147483654
Paso 10: int= -2147483641, uint32_t=2147483655
=== Compilación con UBSan ===
=== Detección de desbordamiento con signo ===
Inicial: int=2147483645, uint32_t=2147483645
Paso  1: int=  2147483646, uint32_t=2147483646
Paso  2: int=  2147483647, uint32_t=2147483647
desborde.c:40:16: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
Código de salida de UBSan: 1
=== Prueba desde -6 con UBSan ===
Inicial: int=-6, uint32_t=4294967290
Paso  1: int=          -5, uint32_t=4294967291
Paso  2: int=          -4, uint32_t=4294967292
Paso  3: int=          -3, uint32_t=4294967293
Paso  4: int=          -2, uint32_t=4294967294
Paso  5: int=          -1, uint32_t=4294967295
Paso  6: int=           0, uint32_t=0
Paso  7: int=           1, uint32_t=1
Paso  8: int=           2, uint32_t=2
Paso  9: int=           3, uint32_t=3
Paso 10: int=           4, uint32_t=4

```

### Análisis del desbordamiento

En la ejecución normal se observó que int pasó de 2147483647
a -2147483648. Ese resultado no está garantizado por C:
el desbordamiento de enteros con signo es comportamiento indefinido.

UBSan detectó la suma 2147483647 + 1 y terminó con código 1.
Con entrada -6, uint32_t comenzó en 4294967290 y pasó a cero
en el sexto incremento, sin diagnóstico de UBSan.
La aritmética sin signo está definida módulo 2^32.

## Parte D — Lectura de registros GPIO

Se utiliza mmap sobre /dev/gpiomem y una estructura cuyos
desplazamientos se verifican mediante _Static_assert.
Este programa únicamente lee GPFSEL y GPLEV.

GPFSEL indica la función seleccionada para cada pin.
GPLEV indica el nivel lógico leído.
gpioinfo proporciona la información administrada por el kernel.

### Resultados

```text
=== Compilación ===
gcc -std=gnu11 -Wall -Wextra -Wconversion -O0 -g -o regs_leer regs_leer.c
=== Lectura directa de registros ===
GPIO | Registro | Campo | Funcion | Nivel
   2 | GPFSEL0  |     4 | ALT0    | 1
   3 | GPFSEL0  |     4 | ALT0    | 1
   8 | GPFSEL0  |     1 | Salida  | 1
   9 | GPFSEL0  |     4 | ALT0    | 0
  10 | GPFSEL1  |     4 | ALT0    | 0
  11 | GPFSEL1  |     4 | ALT0    | 0
   5 | GPFSEL0  |     0 | Entrada | 0
   6 | GPFSEL0  |     0 | Entrada | 0
  17 | GPFSEL1  |     0 | Entrada | 0
  27 | GPFSEL2  |     0 | Entrada | 0
  22 | GPFSEL2  |     0 | Entrada | 0
  23 | GPFSEL2  |     0 | Entrada | 0
=== Información del kernel ===
gpiochip0 2	"GPIO2"         	input consumer="kernel"
gpiochip0 3	"GPIO3"         	input consumer="kernel"
gpiochip0 5	"GPIO5"         	input
gpiochip0 6	"GPIO6"         	input
gpiochip0 8	"GPIO8"         	output active-low consumer="spi0 CS0"
gpiochip0 9	"GPIO9"         	input consumer="kernel"
gpiochip0 10	"GPIO10"        	input consumer="kernel"
gpiochip0 11	"GPIO11"        	input consumer="kernel"
gpiochip0 17	"GPIO17"        	input
gpiochip0 22	"GPIO22"        	input
gpiochip0 23	"GPIO23"        	input
gpiochip0 27	"GPIO27"        	input

```

### Interpretación de la lectura inicial

GPIO2, GPIO3, GPIO9, GPIO10 y GPIO11 mostraron función ALT0.
GPIO8 mostró función de salida y gpioinfo lo identificó
como "spi0 CS0", activo en bajo. Se conserva esa configuración.

Los GPIO5, GPIO6, GPIO17, GPIO27, GPIO22 y GPIO23 estaban
configurados como entradas, con nivel cero.

gpioinfo muestra información administrada por el kernel;
sus etiquetas input/output no sustituyen la lectura del
campo de función alternativa en GPFSEL.

### Prueba de LEDs por registros

Se modifican únicamente los campos de función de GPIO17,
GPIO27, GPIO22 y GPIO23. Cada campo ocupa tres bits.

GPSET activa los pines cuyos bits se escriben en 1.
GPCLR desactiva los pines cuyos bits se escriben en 1.
Se usan asignaciones directas, sin leer estos registros
para hacer operaciones de lectura-modificación-escritura.

Predicciones:
- Cuatro LEDs encendidos: máscara 0x08C20000.
- Solo GPIO17 y GPIO22 encendidos: máscara 0x00420000.
- Todos apagados: máscara 0x00000000.
- Los pines de los LEDs terminarán como entradas.
- Las funciones de los pines I2C y SPI se conservarán.

### Resultados

```text
gcc -std=gnu11 -Wall -Wextra -Wconversion -O0 -g -o leds_regs leds_regs.c
make: 'regs_leer' is up to date.
=== Estado anterior ===
GPIO | Registro | Campo | Funcion | Nivel
   2 | GPFSEL0  |     4 | ALT0    | 1
   3 | GPFSEL0  |     4 | ALT0    | 1
   8 | GPFSEL0  |     1 | Salida  | 1
   9 | GPFSEL0  |     4 | ALT0    | 0
  10 | GPFSEL1  |     4 | ALT0    | 0
  11 | GPFSEL1  |     4 | ALT0    | 0
   5 | GPFSEL0  |     0 | Entrada | 0
   6 | GPFSEL0  |     0 | Entrada | 0
  17 | GPFSEL1  |     0 | Entrada | 0
  27 | GPFSEL2  |     0 | Entrada | 0
  22 | GPFSEL2  |     0 | Entrada | 0
  23 | GPFSEL2  |     0 | Entrada | 0
=== Observa los LEDs ===
=== Los cuatro LEDs: tres parpadeos ===
GPLEV0 (solo LEDs): 0x08C20000
GPLEV0 (solo LEDs): 0x00000000
GPLEV0 (solo LEDs): 0x08C20000
GPLEV0 (solo LEDs): 0x00000000
GPLEV0 (solo LEDs): 0x08C20000
GPLEV0 (solo LEDs): 0x00000000
=== Secuencia individual: tres vueltas ===
GPIO17
GPLEV0 (solo LEDs): 0x00020000
GPIO27
GPLEV0 (solo LEDs): 0x08000000
GPIO22
GPLEV0 (solo LEDs): 0x00400000
GPIO23
GPLEV0 (solo LEDs): 0x00800000
GPIO17
GPLEV0 (solo LEDs): 0x00020000
GPIO27
GPLEV0 (solo LEDs): 0x08000000
GPIO22
GPLEV0 (solo LEDs): 0x00400000
GPIO23
GPLEV0 (solo LEDs): 0x00800000
GPIO17
GPLEV0 (solo LEDs): 0x00020000
GPIO27
GPLEV0 (solo LEDs): 0x08000000
GPIO22
GPLEV0 (solo LEDs): 0x00400000
GPIO23
GPLEV0 (solo LEDs): 0x00800000
=== Solo GPIO17 y GPIO22: tres parpadeos ===
GPLEV0 (solo LEDs): 0x00420000
GPLEV0 (solo LEDs): 0x00420000
GPLEV0 (solo LEDs): 0x00420000
Fin: LEDs apagados y sus pines como entradas.
=== Estado posterior ===
GPIO | Registro | Campo | Funcion | Nivel
   2 | GPFSEL0  |     4 | ALT0    | 1
   3 | GPFSEL0  |     4 | ALT0    | 1
   8 | GPFSEL0  |     1 | Salida  | 1
   9 | GPFSEL0  |     4 | ALT0    | 0
  10 | GPFSEL1  |     4 | ALT0    | 0
  11 | GPFSEL1  |     4 | ALT0    | 0
   5 | GPFSEL0  |     0 | Entrada | 0
   6 | GPFSEL0  |     0 | Entrada | 0
  17 | GPFSEL1  |     0 | Entrada | 0
  27 | GPFSEL2  |     0 | Entrada | 0
  22 | GPFSEL2  |     0 | Entrada | 0
  23 | GPFSEL2  |     0 | Entrada | 0

```

### Resultado de la prueba de LEDs

Se observaron las tres secuencias: los cuatro LEDs juntos,
encendido individual y únicamente GPIO17 y GPIO22.
Las lecturas coincidieron con las máscaras previstas.
Al terminar, los pines de LEDs quedaron como entradas,
con nivel cero. Las funciones de I2C y SPI se conservaron.

Para encender los cuatro LEDs juntos se realizó una escritura
en GPSET0; para apagarlos, una escritura en GPCLR0.

## Parte E — volatile y optimización

### Predicciones

Con volatile, se espera que el programa consulte GPLEV
repetidamente y detecte el botón tanto con -O0 como con -O2.

Sin volatile y con -O0, se espera que detecte el botón,
aunque ese funcionamiento no justifica omitir volatile.

Sin volatile y con -O2, el compilador puede sacar la lectura
fuera del ciclo. Si el botón estaba suelto al comenzar,
el programa podría quedarse esperando incluso al presionarlo.

El límite será de 10 segundos:
- Código 0: el programa terminó normalmente.
- Código 124: timeout terminó la espera.

### Compilación y pruebas

```text
=== b_con_O0 ===
Esperando el boton GPIO5...
Boton detectado.
Código de salida: 0
=== b_sin_O0 ===
Esperando el boton GPIO5...
Boton detectado.
Código de salida: 0
=== b_con_O2 ===
Esperando el boton GPIO5...
Boton detectado.
Código de salida: 0
=== b_sin_O2 ===
Esperando el boton GPIO5...
Código de salida: 124
=== Generando ensamblador con -O2 ===
=== con.s: acceso a GPLEV (offset 52) ===
=== sin.s: acceso a GPLEV (offset 52) ===

```

### Inspección del ensamblador

```text
=== con.s ===
main:
	stp	x29, x30, [sp, -32]!
	mov	w1, 4098
	adrp	x0, .LC0
	mov	x29, sp
	add	x0, x0, :lo12:.LC0
	movk	w1, 0x10, lsl 16
	bl	open
	tbnz	w0, #31, .L16
	mov	w4, w0
	mov	x5, 0
	mov	w3, 1
	mov	w2, 3
	mov	x1, 4096
	stp	x19, x20, [sp, 16]
	mov	w20, w0
	mov	x0, 0
	bl	mmap
	mov	x19, x0
	cmn	x0, #1
	beq	.L17
	mov	w0, w20
	bl	close
	cbz	x19, .L14
	ldr	w1, [x19]
	adrp	x0, .LC3
	add	x0, x0, :lo12:.LC3
	and	w1, w1, -229377
	str	w1, [x19]
	bl	puts
	adrp	x0, :got:stdout;ldr	x0, [x0, :got_lo12:stdout]
	ldr	x0, [x0]
	bl	fflush
	ldr	w0, [x19, 52]
	tbz	x0, 5, .L5
	adrp	x0, .LC4
	add	x0, x0, :lo12:.LC4
	bl	puts
	mov	x0, x19
	mov	x1, 4096
	bl	munmap
	tbnz	w0, #31, .L18
	ldp	x19, x20, [sp, 16]
	mov	w0, 0
	ldp	x29, x30, [sp], 32
	ret
	ldp	x19, x20, [sp, 16]
	mov	w0, 1
	b	.L1
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	bl	perror
	mov	w0, w20
	bl	close
	mov	w0, 1
	ldp	x19, x20, [sp, 16]
	b	.L1
	adrp	x0, .LC1
	add	x0, x0, :lo12:.LC1
	bl	perror
	mov	w0, 1
	b	.L1
	adrp	x0, .LC5
	add	x0, x0, :lo12:.LC5
	bl	perror
	b	.L6
=== sin.s ===
main:
	stp	x29, x30, [sp, -32]!
	mov	w1, 4098
	adrp	x0, .LC0
	mov	x29, sp
	add	x0, x0, :lo12:.LC0
	movk	w1, 0x10, lsl 16
	bl	open
	tbnz	w0, #31, .L15
	mov	w4, w0
	mov	x5, 0
	mov	w3, 1
	mov	w2, 3
	mov	x1, 4096
	stp	x19, x20, [sp, 16]
	mov	w20, w0
	mov	x0, 0
	bl	mmap
	mov	x19, x0
	cmn	x0, #1
	beq	.L16
	mov	w0, w20
	bl	close
	cbz	x19, .L13
	ldr	w1, [x19]
	adrp	x0, .LC3
	add	x0, x0, :lo12:.LC3
	and	w1, w1, -229377
	str	w1, [x19]
	bl	puts
	adrp	x0, :got:stdout;ldr	x0, [x0, :got_lo12:stdout]
	ldr	x0, [x0]
	bl	fflush
	ldr	w0, [x19, 52]
	tbz	x0, 5, .L6
	adrp	x0, .LC4
	add	x0, x0, :lo12:.LC4
	bl	puts
	mov	x0, x19
	mov	x1, 4096
	bl	munmap
	tbnz	w0, #31, .L17
	ldp	x19, x20, [sp, 16]
	mov	w0, 0
	ldp	x29, x30, [sp], 32
	ret
	b	.L6
	ldp	x19, x20, [sp, 16]
	mov	w0, 1
	b	.L1
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	bl	perror
	mov	w0, w20
	bl	close
	mov	w0, 1
	ldp	x19, x20, [sp, 16]
	b	.L1
	adrp	x0, .LC1
	add	x0, x0, :lo12:.LC1
	bl	perror
	mov	w0, 1
	b	.L1
	adrp	x0, .LC5
	add	x0, x0, :lo12:.LC5
	bl	perror
	b	.L7

```

### Inspección del ensamblador

```text
=== con.s ===
main:
	stp	x29, x30, [sp, -32]!
	mov	w1, 4098
	adrp	x0, .LC0
	mov	x29, sp
	add	x0, x0, :lo12:.LC0
	movk	w1, 0x10, lsl 16
	bl	open
	tbnz	w0, #31, .L16
	mov	w4, w0
	mov	x5, 0
	mov	w3, 1
	mov	w2, 3
	mov	x1, 4096
	stp	x19, x20, [sp, 16]
	mov	w20, w0
	mov	x0, 0
	bl	mmap
	mov	x19, x0
	cmn	x0, #1
	beq	.L17
	mov	w0, w20
	bl	close
	cbz	x19, .L14
	ldr	w1, [x19]
	adrp	x0, .LC3
	add	x0, x0, :lo12:.LC3
	and	w1, w1, -229377
	str	w1, [x19]
	bl	puts
	adrp	x0, :got:stdout;ldr	x0, [x0, :got_lo12:stdout]
	ldr	x0, [x0]
	bl	fflush
.L5:
	ldr	w0, [x19, 52]
	tbz	x0, 5, .L5
	adrp	x0, .LC4
	add	x0, x0, :lo12:.LC4
	bl	puts
	mov	x0, x19
	mov	x1, 4096
	bl	munmap
	tbnz	w0, #31, .L18
.L6:
	ldp	x19, x20, [sp, 16]
	mov	w0, 0
.L1:
	ldp	x29, x30, [sp], 32
	ret
.L14:
	ldp	x19, x20, [sp, 16]
	mov	w0, 1
	b	.L1
.L17:
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	bl	perror
	mov	w0, w20
	bl	close
	mov	w0, 1
	ldp	x19, x20, [sp, 16]
	b	.L1
.L16:
	adrp	x0, .LC1
	add	x0, x0, :lo12:.LC1
	bl	perror
	mov	w0, 1
	b	.L1
.L18:
	adrp	x0, .LC5
	add	x0, x0, :lo12:.LC5
	bl	perror
	b	.L6
=== sin.s ===
main:
	stp	x29, x30, [sp, -32]!
	mov	w1, 4098
	adrp	x0, .LC0
	mov	x29, sp
	add	x0, x0, :lo12:.LC0
	movk	w1, 0x10, lsl 16
	bl	open
	tbnz	w0, #31, .L15
	mov	w4, w0
	mov	x5, 0
	mov	w3, 1
	mov	w2, 3
	mov	x1, 4096
	stp	x19, x20, [sp, 16]
	mov	w20, w0
	mov	x0, 0
	bl	mmap
	mov	x19, x0
	cmn	x0, #1
	beq	.L16
	mov	w0, w20
	bl	close
	cbz	x19, .L13
	ldr	w1, [x19]
	adrp	x0, .LC3
	add	x0, x0, :lo12:.LC3
	and	w1, w1, -229377
	str	w1, [x19]
	bl	puts
	adrp	x0, :got:stdout;ldr	x0, [x0, :got_lo12:stdout]
	ldr	x0, [x0]
	bl	fflush
	ldr	w0, [x19, 52]
	tbz	x0, 5, .L6
	adrp	x0, .LC4
	add	x0, x0, :lo12:.LC4
	bl	puts
	mov	x0, x19
	mov	x1, 4096
	bl	munmap
	tbnz	w0, #31, .L17
.L7:
	ldp	x19, x20, [sp, 16]
	mov	w0, 0
.L1:
	ldp	x29, x30, [sp], 32
	ret
.L6:
	b	.L6
.L13:
	ldp	x19, x20, [sp, 16]
	mov	w0, 1
	b	.L1
.L16:
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	bl	perror
	mov	w0, w20
	bl	close
	mov	w0, 1
	ldp	x19, x20, [sp, 16]
	b	.L1
.L15:
	adrp	x0, .LC1
	add	x0, x0, :lo12:.LC1
	bl	perror
	mov	w0, 1
	b	.L1
.L17:
	adrp	x0, .LC5
	add	x0, x0, :lo12:.LC5
	bl	perror
	b	.L7

```
### Conclusión de la prueba de volatile

Las versiones con volatile en -O0 y -O2 detectaron el botón
y terminaron con código 0. La versión sin volatile en -O0
también lo detectó. La versión sin volatile en -O2 agotó
los 10 segundos y timeout devolvió el código 124.

En con.s, la etiqueta .L5 está antes de la instrucción
ldr w0, [x19, 52]. El salto tbz regresa a esa etiqueta:
se vuelve a leer GPLEV0 en cada iteración.

En sin.s, la lectura ocurre antes del ciclo. Si el bit 5
está en cero, se salta a .L6, donde b .L6 forma un ciclo
infinito sin nuevas lecturas del registro.

volatile conserva los accesos necesarios a los registros
de hardware, cuyo contenido puede cambiar externamente.
No proporciona atomicidad ni sincronización entre hilos.

Los campos de registros como GPLEV y GPFSEL requieren
accesos volatile en esta implementación. Las variables
locales ordinarias, como índices y máscaras calculadas,
no lo necesitan por el solo hecho de controlar hardware.

## Parte F — Sensores desde C

Se lee el canal CH0 del MCP3008 mediante SPI, usando
una transferencia de tres bytes: 01 80 00.

El resultado se reconstruye con:
((rx[1] & 3) << 8) | rx[2].
La máscara conserva los dos bits superiores de la medición
de 10 bits. El voltaje se estima suponiendo VREF = 3.30 V.

El sensor instalado es un BME280, identificado anteriormente
con ID 0x60. Se utiliza su medición de temperatura.
La calibración se lee del dispositivo, sin fijarla en el código.

La temperatura cruda se reconstruye como:
(bytes[0] << 12) | (bytes[1] << 4) | (bytes[2] >> 4).

La compensación utiliza aritmética entera con intermedios
de 64 bits y devuelve centésimas de grado.

### Predicciones
- El potenciómetro debe recorrer aproximadamente 0 a 1023.
- Los extremos corresponden aproximadamente a 0 y 3.30 V.
- Cerca de la mitad se espera un valor próximo a 512.
- Al acercar una fuente de calor suave, se espera que
  la temperatura aumente gradualmente.

### Resultados

```text
gcc -std=gnu11 -Wall -Wextra -Wconversion -O0 -g -o sensores sensores.c
=== Potenciómetro: primer extremo ===
Direccion: 0x76, ID: 0x60
Calibracion: T1=27893, T2=26742, T3=50
Bytes T1: F5 6C

Muestra 1
SPI RX: 00 00 00 | Pot=0 | V=0.00
Temp bytes: 7B AD 00 | Cruda=506576 | T=19.22 C

Muestra 2
SPI RX: 00 00 00 | Pot=0 | V=0.00
Temp bytes: 7B DB 00 | Cruda=507312 | T=19.46 C

Muestra 3
SPI RX: 00 00 00 | Pot=0 | V=0.00
Temp bytes: 7B DF 00 | Cruda=507376 | T=19.48 C

Muestra 4
SPI RX: 00 00 00 | Pot=0 | V=0.00
Temp bytes: 7B E1 00 | Cruda=507408 | T=19.49 C

Muestra 5
SPI RX: 00 00 00 | Pot=0 | V=0.00
Temp bytes: 7B E4 00 | Cruda=507456 | T=19.50 C
=== Potenciómetro: extremo opuesto ===
Direccion: 0x76, ID: 0x60
Calibracion: T1=27893, T2=26742, T3=50
Bytes T1: F5 6C

Muestra 1
SPI RX: 00 03 FF | Pot=1023 | V=3.30
Temp bytes: 7B B6 00 | Cruda=506720 | T=19.27 C

Muestra 2
SPI RX: 00 03 FF | Pot=1023 | V=3.30
Temp bytes: 7B E3 00 | Cruda=507440 | T=19.50 C

Muestra 3
SPI RX: 00 03 FF | Pot=1023 | V=3.30
Temp bytes: 7B E9 00 | Cruda=507536 | T=19.53 C

Muestra 4
SPI RX: 00 03 FF | Pot=1023 | V=3.30
Temp bytes: 7B EB 00 | Cruda=507568 | T=19.54 C

Muestra 5
SPI RX: 00 03 FF | Pot=1023 | V=3.30
Temp bytes: 7B EB 00 | Cruda=507568 | T=19.54 C
=== Potenciómetro: punto medio ===
Direccion: 0x76, ID: 0x60
Calibracion: T1=27893, T2=26742, T3=50
Bytes T1: F5 6C

Muestra 1
SPI RX: 00 02 18 | Pot=536 | V=1.73
Temp bytes: 7B AE 00 | Cruda=506592 | T=19.23 C

Muestra 2
SPI RX: 00 02 18 | Pot=536 | V=1.73
Temp bytes: 7B DB 00 | Cruda=507312 | T=19.46 C

Muestra 3
SPI RX: 00 02 18 | Pot=536 | V=1.73
Temp bytes: 7B E3 00 | Cruda=507440 | T=19.50 C

Muestra 4
SPI RX: 00 02 18 | Pot=536 | V=1.73
Temp bytes: 7B E5 00 | Cruda=507472 | T=19.51 C

Muestra 5
SPI RX: 00 02 18 | Pot=536 | V=1.73
Temp bytes: 7B E8 00 | Cruda=507520 | T=19.52 C
=== Temperatura con aporte de calor suave ===
Direccion: 0x76, ID: 0x60
Calibracion: T1=27893, T2=26742, T3=50
Bytes T1: F5 6C

Muestra 1
SPI RX: 00 02 17 | Pot=535 | V=1.73
Temp bytes: 7C BF 00 | Cruda=510960 | T=20.62 C

Muestra 2
SPI RX: 00 02 17 | Pot=535 | V=1.73
Temp bytes: 7D 05 00 | Cruda=512080 | T=20.98 C

Muestra 3
SPI RX: 00 02 18 | Pot=536 | V=1.73
Temp bytes: 7C F9 00 | Cruda=511888 | T=20.91 C

Muestra 4
SPI RX: 00 02 17 | Pot=535 | V=1.73
Temp bytes: 7D 26 00 | Cruda=512608 | T=21.14 C

Muestra 5
SPI RX: 00 02 17 | Pot=535 | V=1.73
Temp bytes: 7D 12 00 | Cruda=512288 | T=21.04 C

```
### Análisis de las lecturas SPI e I2C

El potenciómetro produjo valores de 0 y 1023 en sus extremos,
y 536 en una posición intermedia. El voltaje se calculó mediante
V = lectura * 3.30 / 1023; no se midió directamente con multímetro.

Para la respuesta SPI 00 02 18, la reconstrucción fue:
((0x02 & 0x03) << 8) | 0x18 = 536.
La máscara 0x03 conserva únicamente los dos bits superiores
del resultado de 10 bits.

El sensor respondió en la dirección I2C 0x76 con identificador
0x60. Los coeficientes fueron T1=27893, T2=26742 y T3=50.

Los bytes F5 6C de T1 están en orden little-endian:
0x6CF5 = 27893. Interpretarlos al revés daría 0xF56C = 62828,
alterando el cálculo de compensación.

Los bytes de temperatura 7B AD 00 se reconstruyeron como:
(0x7B << 12) | (0xAD << 4) | (0x00 >> 4) = 506576.
La compensación produjo 19.22 °C.

Antes del aporte de calor se obtuvieron lecturas cercanas a
19.5 °C. Con calor suave se registraron entre 20.62 y 21.14 °C.
Esto demuestra una respuesta al cambio térmico, aunque no
constituye una calibración frente a un termómetro de referencia.

## Integración: panel de LEDs, botones y sensores

Predicciones antes de ejecutar:
- El potenciómetro controla una barra de 0 a 4 LEDs.
- Intervalos: 0–204, 205–409, 410–614, 615–819 y 820–1023.
- GPIO5 congela/reactiva la barra una vez por pulsación.
- GPIO6 imprime el potenciómetro actual y la temperatura,
  incluso cuando la barra está congelada.
- El antirrebote requiere aproximadamente 30 ms de estabilidad.
- Ctrl+C apaga los LEDs y devuelve sus pines a entrada.

### Registro de ejecución
```text
Panel activo.
GPIO5: congelar/reactivar. GPIO6: imprimir sensores.
Gira el potenciometro. Ctrl+C para terminar.
Pot=0 | LEDs=0
Pot=535 | LEDs=2
Pot=0 | LEDs=0
Pot=535 | LEDs=2
Pot=0 | LEDs=0
Pot=536 | LEDs=2
Pot=6 | LEDs=0
Pot=536 | LEDs=2
Pot=0 | LEDs=0
Pot=535 | LEDs=2
Pot=0 | LEDs=0
Pot=535 | LEDs=2
Pot=0 | LEDs=0
Pot=535 | LEDs=2
Pot=0 | LEDs=0
Pot=550 | LEDs=2
Pot=0 | LEDs=0
Pot=553 | LEDs=2
Pot=0 | LEDs=0
Pot=554 | LEDs=2
Pot=0 | LEDs=0
Pot=560 | LEDs=2
Pot=0 | LEDs=0
Pot=563 | LEDs=2
Pot=0 | LEDs=0
Pot=569 | LEDs=2
Pot=0 | LEDs=0
Pot=574 | LEDs=2
Pot=0 | LEDs=0
Pot=578 | LEDs=2
Pot=0 | LEDs=0
Pot=584 | LEDs=2
Pot=0 | LEDs=0
Pot=588 | LEDs=2
Pot=0 | LEDs=0
Pot=594 | LEDs=2
Pot=0 | LEDs=0
Pot=598 | LEDs=2
Pot=0 | LEDs=0
Pot=605 | LEDs=2
Pot=0 | LEDs=0
Pot=610 | LEDs=2
Pot=0 | LEDs=0
Pot=614 | LEDs=2
Pot=0 | LEDs=0
Pot=618 | LEDs=3
Pot=0 | LEDs=0
Pot=622 | LEDs=3
Pot=0 | LEDs=0
Pot=629 | LEDs=3
Pot=0 | LEDs=0
Pot=629 | LEDs=3
Pot=0 | LEDs=0
Pot=635 | LEDs=3
Pot=0 | LEDs=0
Pot=640 | LEDs=3
Pot=0 | LEDs=0
Pot=641 | LEDs=3
Pot=0 | LEDs=0
Pot=646 | LEDs=3
Pot=0 | LEDs=0
Pot=648 | LEDs=3
Pot=0 | LEDs=0
Pot=648 | LEDs=3
Pot=0 | LEDs=0
Pot=654 | LEDs=3
Pot=0 | LEDs=0
Pot=656 | LEDs=3
Pot=0 | LEDs=0
Pot=658 | LEDs=3
Pot=0 | LEDs=0
Pot=662 | LEDs=3
Pot=0 | LEDs=0
Pot=666 | LEDs=3
Pot=0 | LEDs=0
Pot=667 | LEDs=3
Pot=0 | LEDs=0
Pot=669 | LEDs=3
Pot=0 | LEDs=0
Pot=672 | LEDs=3
Pot=0 | LEDs=0
Pot=673 | LEDs=3
Pot=0 | LEDs=0
Pot=674 | LEDs=3
Pot=0 | LEDs=0
Pot=675 | LEDs=3
Pot=0 | LEDs=0
Pot=677 | LEDs=3
Pot=0 | LEDs=0
Pot=679 | LEDs=3
Pot=0 | LEDs=0
Pot=682 | LEDs=3
Pot=0 | LEDs=0
Pot=683 | LEDs=3
Pot=0 | LEDs=0
Pot=685 | LEDs=3
Pot=0 | LEDs=0
Pot=687 | LEDs=3
Pot=0 | LEDs=0
Pot=690 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=700 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=688 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=690 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=695 | LEDs=3
Pot=0 | LEDs=0
Pot=692 | LEDs=3
Pot=0 | LEDs=0
Pot=693 | LEDs=3
Pot=0 | LEDs=0
Pot=695 | LEDs=3
Pot=0 | LEDs=0
Pot=694 | LEDs=3
Pot=0 | LEDs=0
Pot=695 | LEDs=3
Pot=0 | LEDs=0
Pot=701 | LEDs=3
Pot=0 | LEDs=0
Pot=705 | LEDs=3
Pot=0 | LEDs=0
Pot=711 | LEDs=3
Pot=0 | LEDs=0
Pot=716 | LEDs=3
Pot=0 | LEDs=0
Pot=726 | LEDs=3
Pot=0 | LEDs=0
Pot=729 | LEDs=3
Pot=0 | LEDs=0
Pot=737 | LEDs=3
Pot=0 | LEDs=0
Pot=743 | LEDs=3
Pot=0 | LEDs=0
Pot=753 | LEDs=3
Pot=0 | LEDs=0
Pot=760 | LEDs=3
Pot=0 | LEDs=0
Pot=767 | LEDs=3
Pot=0 | LEDs=0
Pot=775 | LEDs=3
Pot=0 | LEDs=0
Pot=783 | LEDs=3
Pot=0 | LEDs=0
Pot=792 | LEDs=3
Pot=0 | LEDs=0
Pot=797 | LEDs=3
Pot=0 | LEDs=0
Pot=805 | LEDs=3
Pot=0 | LEDs=0
Pot=814 | LEDs=3
Pot=0 | LEDs=0
Pot=823 | LEDs=4
Pot=0 | LEDs=0
Pot=832 | LEDs=4
Pot=0 | LEDs=0
Pot=838 | LEDs=4
Pot=0 | LEDs=0
Pot=846 | LEDs=4
Pot=0 | LEDs=0
Pot=853 | LEDs=4
Pot=0 | LEDs=0
Pot=866 | LEDs=4
Pot=0 | LEDs=0
Pot=873 | LEDs=4
Pot=0 | LEDs=0
Pot=881 | LEDs=4
Pot=0 | LEDs=0
Pot=891 | LEDs=4
Pot=0 | LEDs=0
Pot=900 | LEDs=4
Pot=0 | LEDs=0
Pot=911 | LEDs=4
Pot=0 | LEDs=0
Pot=918 | LEDs=4
Pot=0 | LEDs=0
Pot=928 | LEDs=4
Pot=0 | LEDs=0
Pot=937 | LEDs=4
Pot=0 | LEDs=0
Pot=944 | LEDs=4
Pot=0 | LEDs=0
Pot=954 | LEDs=4
Pot=0 | LEDs=0
Pot=959 | LEDs=4
Pot=0 | LEDs=0
Pot=965 | LEDs=4
Pot=0 | LEDs=0
Pot=970 | LEDs=4
Pot=0 | LEDs=0
Pot=976 | LEDs=4
Pot=0 | LEDs=0
Pot=984 | LEDs=4
Pot=0 | LEDs=0
Pot=989 | LEDs=4
Pot=0 | LEDs=0
Pot=998 | LEDs=4
Pot=0 | LEDs=0
Pot=1002 | LEDs=4
Pot=0 | LEDs=0
Pot=1011 | LEDs=4
Pot=0 | LEDs=0
Pot=1017 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Barra CONGELADA.
Barra ACTIVA.
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=31 | LEDs=0
Pot=488 | LEDs=2
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=0 | LEDs=0
Pot=1023 | LEDs=4
Pot=817 | LEDs=3
Panel terminado. Apagando LEDs y liberando recursos.
Estado de salida: 0
```
GPIO | Registro | Campo | Funcion | Nivel
   2 | GPFSEL0  |     4 | ALT0    | 1
   3 | GPFSEL0  |     4 | ALT0    | 1
   8 | GPFSEL0  |     1 | Salida  | 1
   9 | GPFSEL0  |     4 | ALT0    | 0
  10 | GPFSEL1  |     4 | ALT0    | 0
  11 | GPFSEL1  |     4 | ALT0    | 0
   5 | GPFSEL0  |     0 | Entrada | 0
   6 | GPFSEL0  |     0 | Entrada | 0
  17 | GPFSEL1  |     0 | Entrada | 0
  27 | GPFSEL2  |     0 | Entrada | 0
  22 | GPFSEL2  |     0 | Entrada | 0
  23 | GPFSEL2  |     0 | Entrada | 0

## Integración: panel de LEDs, botones y sensores

Predicciones antes de ejecutar:
- El potenciómetro controla una barra de 0 a 4 LEDs.
- Intervalos: 0–204, 205–409, 410–614, 615–819 y 820–1023.
- GPIO5 congela/reactiva la barra una vez por pulsación.
- GPIO6 imprime el potenciómetro actual y la temperatura,
  incluso cuando la barra está congelada.
- El antirrebote requiere aproximadamente 30 ms de estabilidad.
- Ctrl+C apaga los LEDs y devuelve sus pines a entrada.

### Registro de ejecución
```text
Panel activo.
GPIO5: congelar/reactivar. GPIO6: imprimir sensores.
Gira el potenciometro. Ctrl+C para terminar.
Pot=0 | LEDs=0
Pot=205 | LEDs=1
Pot=412 | LEDs=2
Pot=634 | LEDs=3
Pot=820 | LEDs=4
Barra CONGELADA.
Lectura: Pot=1023 | T=19.86 C | Barra=congelada
Lectura: Pot=1023 | T=19.85 C | Barra=congelada
Lectura: Pot=1023 | T=19.85 C | Barra=congelada
Lectura: Pot=1023 | T=19.85 C | Barra=congelada
Barra ACTIVA.
Lectura: Pot=1023 | T=19.85 C | Barra=activa
Lectura: Pot=1023 | T=19.86 C | Barra=activa
Lectura: Pot=1023 | T=19.85 C | Barra=activa
Lectura: Pot=1023 | T=19.85 C | Barra=activa
Lectura: Pot=1023 | T=19.85 C | Barra=activa
Pot=819 | LEDs=3
Pot=613 | LEDs=2
Pot=409 | LEDs=1
Pot=411 | LEDs=2
Pot=388 | LEDs=1
Pot=203 | LEDs=0
Pot=209 | LEDs=1
Pot=202 | LEDs=0
Pot=205 | LEDs=1
Panel terminado. Apagando LEDs y liberando recursos.
Estado de salida: 0
```
GPIO | Registro | Campo | Funcion | Nivel
   2 | GPFSEL0  |     4 | ALT0    | 1
   3 | GPFSEL0  |     4 | ALT0    | 1
   8 | GPFSEL0  |     1 | Salida  | 1
   9 | GPFSEL0  |     4 | ALT0    | 0
  10 | GPFSEL1  |     4 | ALT0    | 0
  11 | GPFSEL1  |     4 | ALT0    | 0
   5 | GPFSEL0  |     0 | Entrada | 0
   6 | GPFSEL0  |     0 | Entrada | 0
  17 | GPFSEL1  |     0 | Entrada | 0
  27 | GPFSEL2  |     0 | Entrada | 0
  22 | GPFSEL2  |     0 | Entrada | 0
  23 | GPFSEL2  |     0 | Entrada | 0
### Resultados del panel integrado

Se observaron los cinco niveles de la barra:
0 LEDs con lectura 0, 1 con 205, 2 con 412,
3 con 634 y 4 con 820.

GPIO5 produjo los mensajes de barra congelada y activa.
GPIO6 permitió consultar los sensores en ambos modos:
se registró Pot=1023 y temperaturas de 19.85 a 19.86 °C.

Al disminuir el potenciómetro, la barra redujo su nivel.
Cerca de los umbrales se observaron cambios entre niveles
adyacentes; el programa no implementa histéresis.

Tras Ctrl+C, los GPIO17, 27, 22 y 23 quedaron configurados
como entradas con nivel 0. Los GPIO2, 3, 9, 10 y 11
conservaron ALT0, y GPIO8 permaneció como salida en nivel 1.

En esta ejecución no se reprodujeron los saltos grandes
hacia cero observados anteriormente; su causa no quedó determinada.
