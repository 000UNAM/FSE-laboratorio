#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

void mostrar_linea(FILE *f, const char *etiqueta) {
  char linea[256];

  rewind(f);

  while (fgets(linea, sizeof(linea), f)) {
    if (strncmp(linea, etiqueta, strlen(etiqueta)) == 0) {
      printf("%s", linea);
      return;
    }
  }

  printf("%s: no encontrado\n", etiqueta);
}

int main(void) {
  struct utsname info;

  if (uname(&info) == -1) {
    perror("No se pudo consultar el sistema");
    return 1;
  }

  printf("=== Reporte del sistema ===\n");
  printf("Arquitectura: %s\n", info.machine);
  printf("Kernel:       %s\n", info.release);

  FILE *cpu = fopen("/proc/cpuinfo", "r");

  if (cpu) {
    printf("\n-- CPU --\n");
    mostrar_linea(cpu, "Model");
    mostrar_linea(cpu, "Hardware");
    fclose(cpu);
  } else {
    perror("No se pudo abrir /proc/cpuinfo");
    return 1;
  }

  FILE *mem = fopen("/proc/meminfo", "r");

  if (mem) {
    printf("\n-- Memoria --\n");
    mostrar_linea(mem, "MemTotal");
    mostrar_linea(mem, "MemFree");
    fclose(mem);
  } else {
    perror("No se pudo abrir /proc/meminfo");
    return 1;
  }

  return 0;
}
