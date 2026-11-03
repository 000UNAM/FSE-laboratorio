
#!/bin/bash
#@Autor       Nicolás Montesinos Iván Emmanuel
#@Fecha       02/11/2026
#@Descripcion Compila la estacion ambiental de forma segura.
#             Elaborado con asistencia de IA.

set -euo pipefail

# Ir a la raiz del proyecto.
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."

# Verificar disponibilidad del compilador.
if ! command -v gcc >/dev/null 2>&1; then
  echo "ERROR: GCC no esta disponible." >&2
  exit 1
fi

# Archivos fuente del programa principal.
fuentes=(
  src/estacion.c
  src/bme280.c
  src/oled.c
  src/registro.c
)

# Comprobar que existen los archivos fuente.
for fuente in "${fuentes[@]}"; do
  if [[ ! -f "$fuente" ]]; then
    echo "ERROR: No existe $fuente" >&2
    exit 1
  fi
done

mkdir -p bin evidencias

# Crear un espacio temporal para compilar.
temporal=$(mktemp -d bin/.estacion-temporal-XXXXXXXX)

# Limpiar unicamente los archivos temporales propios.
trap 'rm -f -- "$temporal/estacion"; rmdir -- "$temporal"' EXIT

# Cada compilacion conserva su propio registro.
registro=$(mktemp evidencias/compilacion-estacion-XXXXXXXX.log)

printf 'Compilando estacion ambiental...\n' | tee "$registro"

# Compilar sin reemplazar todavia el ejecutable original.
if gcc -std=c11 -Wall -Wextra -Wpedantic -O2 \
    "${fuentes[@]}" -o "$temporal/estacion" \
    2>&1 | tee -a "$registro"; then

  # Reemplazar solo despues de compilar correctamente.
  mv -f -- "$temporal/estacion" bin/estacion

  {
    echo "Compilacion finalizada correctamente."
    echo "Ejecutable: bin/estacion"
    echo "Registro: $registro"
  } | tee -a "$registro"

else
  echo "ERROR: Fallo la compilacion o su registro." \
    | tee -a "$registro" >&2
  exit 1
fi

