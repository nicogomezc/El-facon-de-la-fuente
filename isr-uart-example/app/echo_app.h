#ifndef ECHO_APP_H
#define ECHO_APP_H

#include "includes.h"

#define ECHO_APP_BAUD_RATE 9600

// Ejemplo minimo de capa de aplicacion: usa uart_comm (devices/) sin saber
// nada de registros ni de interrupciones.
//
// Convencion de sufijos para nombres de funcion en este proyecto:
//   _init -> inicializacion, corre una sola vez al arrancar el firmware.
//   _step -> corre dentro del for(;;) infinito (loop principal).
//   _tic  -> corre dentro de una interrupcion de timer.
//   _isr  -> corre dentro de una interrupcion que no es de timer.
void echo_app_init(void);
void echo_app_step(void);

#endif
