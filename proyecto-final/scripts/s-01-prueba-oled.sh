#!/bin/bash
#@Autor       Nicolás Montesinos Iván Emmanuel
#@Fecha       02/11/2026
#@Descripcion Compila y prueba la OLED; guarda salida y errores.
#             Elaborado con asistencia de IA.
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
mkdir -p bin evidencias

registro="evidencias/prueba-oled-$(date +%Y%m%d-%H%M%S).log"

{
  date --iso-8601=seconds
  gcc -std=c11 -Wall -Wextra -Wpedantic -O2 \
    src/prueba-oled.c -o bin/prueba-oled
  ./bin/prueba-oled
} 2>&1 | tee "$registro"

