#ifndef ESTADO_H
#define ESTADO_H

#include <stdbool.h>
#include <stdint.h>
#include "configuracion.h"

typedef struct
{
    float tension_actual;
    float corriente_actual;
    bool salida_activa;
    uint32_t tiempo_desde_ultimo_corte;
    uint8_t reintentos_utilizados;
    configuracion_t configuracion;

} estado_t;

#endif