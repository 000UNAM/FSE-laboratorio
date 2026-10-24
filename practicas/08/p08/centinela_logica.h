#ifndef CENTINELA_LOGICA_H
#define CENTINELA_LOGICA_H

/* Logica del Centinela. Aqui se escribe el trabajo de la Parte B.
 * Las funciones no usan hardware: se prueban con `make prueba`.
 * Cada funcion trae su descripcion; las reglas numericas estan en centinela_cfg.h.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "centinela_cfg.h"

/* Promedio entero (division entera) de n lecturas de temperatura en centesimas
 * de grado. Debe funcionar con valores negativos y no desbordar la suma. */
static int32_t temp_base(const int32_t *lecturas, size_t n)
{
    if (lecturas == NULL || n == 0)
        return 0;

    int64_t suma = 0;

    for (size_t i = 0; i < n; i++)
        suma += lecturas[i];

    return (int32_t)(suma / (int64_t)n);
}

/* Umbral de alarma en centesimas: base + UMBRAL_MIN_C100 + pot * UMBRAL_RANGO_C100 / 1023.
 * pot va de 0 a 1023. Solo aritmetica entera. */
static int32_t umbral_desde_pot(int32_t base_c100, unsigned pot)
{
    unsigned incremento =
        (pot * (unsigned)UMBRAL_RANGO_C100) / 1023u;

    return base_c100
           + UMBRAL_MIN_C100
           + (int32_t)incremento;
}

/* LEDs encendidos segun el margen = umbral - temperatura (en centesimas):
 *   margen <= 0   -> 4
 *   margen <= 100 -> 3
 *   margen <= 200 -> 2
 *   margen <= 400 -> 1
 *   mayor         -> 0 */
static unsigned nivel_barra(int32_t margen_c100)
{
    if (margen_c100 <= 0)
        return 4;

    if (margen_c100 <= 100)
        return 3;

    if (margen_c100 <= 200)
        return 2;

    if (margen_c100 <= 400)
        return 1;

    return 0;
}

/* Mascara de bits de los primeros `nivel` LEDs en el orden LED_1, LED_2, LED_3, LED_4.
 * Si nivel es mayor que 4, enciende los cuatro. */
static uint32_t mascara_leds(unsigned nivel)
{
    uint32_t mascara = 0;

    if (nivel >= 1)
        mascara |= BIT(LED_1);

    if (nivel >= 2)
        mascara |= BIT(LED_2);

    if (nivel >= 3)
        mascara |= BIT(LED_3);

    if (nivel >= 4)
        mascara |= BIT(LED_4);

    return mascara;
}

/* Patron de ALARMA: en las fases pares encienden LED_1 y LED_3; en las impares,
 * LED_2 y LED_4. */
static uint32_t mascara_alarma(unsigned fase)
{
    if ((fase % 2u) == 0u)
        return BIT(LED_1) | BIT(LED_3);

    return BIT(LED_2) | BIT(LED_4);
}


/* Maquina de estados del Centinela.
 *   PAUSADO:   solo sale si hay_pausa (pasa a VIGILANDO). Ignora la temperatura.
 *   VIGILANDO: con hay_pausa pasa a PAUSADO; si temp >= umbral pasa a ALARMA.
 *   ALARMA:    con hay_pausa pasa a PAUSADO; si temp <= umbral - HISTERESIS_C100
 *              vuelve a VIGILANDO.
 * hay_pausa vale 1 solo en el instante en que se presiona el boton. */
static estado_t estado_siguiente(estado_t actual,
                                int32_t temp_c100,
                                int32_t umbral_c100,
                                int hay_pausa)
{
    /* Si esta pausado, solo el boton permite reanudar. */
    if (actual == PAUSADO) {
        if (hay_pausa)
            return VIGILANDO;

        return PAUSADO;
    }

    /* El boton de pausa tiene prioridad. */
    if (hay_pausa)
        return PAUSADO;

    /* Activar alarma cuando se alcanza el umbral. */
    if (actual == VIGILANDO) {
        if (temp_c100 >= umbral_c100)
            return ALARMA;
    }

    /* Desactivar alarma aplicando histeresis. */
    if (actual == ALARMA) {
        if ((int64_t)temp_c100 <=
            (int64_t)umbral_c100 - HISTERESIS_C100)
            return VIGILANDO;
    }

    /* Si no hay transicion, conservar el estado. */
    return actual;
}


/* Escribe en dst (de n bytes) una linea del archivo, con el formato
 *   t_ms,temp_c100,umbral_c100,ESTADO\n
 * por ejemplo "1500,2534,2650,VIGILANDO\n". ESTADO es PAUSADO, VIGILANDO o ALARMA.
 * Nunca escribe mas de n bytes. Devuelve el largo que tendria la linea completa. */
static int formatear_registro(char *dst, size_t n,
                             uint32_t t_ms,
                             int32_t temp_c100,
                             int32_t umbral_c100,
                             estado_t e)
{
    const char *estado;

    if (e == PAUSADO)
        estado = "PAUSADO";
    else if (e == ALARMA)
        estado = "ALARMA";
    else
        estado = "VIGILANDO";

    return snprintf(dst, n, "%u,%d,%d,%s\n",
                    (unsigned)t_ms,
                    (int)temp_c100,
                    (int)umbral_c100,
                    estado);
}

#endif
