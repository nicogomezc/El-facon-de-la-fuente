#ifndef EVENTO_H
#define EVENTO_H

#include <stdint.h>

typedef enum
{
    SOBRETENSION,
    BAJA_TENSION,
    SOBRECORRIENTE,
    REINTENTO,
    BLOQUEO
} tipo_evento_t;


typedef struct
{
    tipo_evento_t tipo;
    uint32_t tiempo;
    float valor;
} evento_t;

#endif