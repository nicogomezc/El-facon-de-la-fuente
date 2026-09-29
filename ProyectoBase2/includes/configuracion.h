#ifndef CONFIGURACION_H
#define CONFIGURACION_H

typedef struct {
    float tension_minima;
    float tension_maxima;
    float corriente_maxima;
    int tiempo_restablecimiento;
    int reintentos_maximos;
}configuracion_t;

#endif