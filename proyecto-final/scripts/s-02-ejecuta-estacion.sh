#!/bin/bash
#@Autor       Nicolás Montesinos Iván Emmanuel
#@Fecha       02/11/2026
#@Descripcion Ejecuta la estacion y conserva sus evidencias.
#             Elaborado con asistencia de IA.

set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."

for herramienta in timeout tee mktemp date; do
  if ! command -v "$herramienta" >/dev/null 2>&1; then
    printf 'Falta la herramienta: %s\n' "$herramienta" >&2
    exit 1
  fi
done

if [[ ! -x bin/estacion ]]; then
  printf 'No existe el ejecutable bin/estacion.\n' >&2
  exit 1
fi

mkdir -p evidencias

# Una carpeta nueva por ejecucion evita sobrescribir resultados.
marca=$(date +%Y%m%d-%H%M%S)
carpeta=$(mktemp -d "evidencias/estacion-${marca}-XXXXXX")

csv="$carpeta/mediciones.csv"
registro="$carpeta/ejecucion.log"
duracion=310

{
  printf 'Inicio: %s\n' "$(date --iso-8601=seconds)"
  printf 'Duracion solicitada: %s segundos\n' "$duracion"
  printf 'CSV: %s\n' "$csv"
  printf 'Lecturas: cada 2 s; registros CSV: cada 10 s.\n'
} | tee "$registro"

# Ctrl+C llega al programa. El script continua para anotar su salida.
# tee -i permite conservar los mensajes finales al pulsar Ctrl+C.
trap ':' INT

# Desactivamos temporalmente la salida automatica por error:
# necesitamos recoger el resultado de ambos lados de la tuberia.
set +e

timeout --foreground --preserve-status \
  --signal=TERM --kill-after=5s "${duracion}s" \
  ./bin/estacion "$csv" 2>&1 | tee -i -a "$registro"

estados=("${PIPESTATUS[@]}")

set -e
trap - INT

codigo_estacion=${estados[0]}
codigo_tee=${estados[1]}

{
  printf '\nFin: %s\n' "$(date --iso-8601=seconds)"
  printf 'Codigo de salida del supervisor/programa: %s\n' \
    "$codigo_estacion"
  printf 'Codigo de salida de tee: %s\n' "$codigo_tee"
  printf 'Carpeta de evidencias: %s\n' "$carpeta"
} | tee -a "$registro"

if (( codigo_estacion != 0 )); then
  exit "$codigo_estacion"
fi

if (( codigo_tee != 0 )); then
  exit "$codigo_tee"
fi
