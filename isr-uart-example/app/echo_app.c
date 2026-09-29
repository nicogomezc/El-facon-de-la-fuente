#include "includes.h"

// Ejemplo minimo de capa de aplicacion: usa uart_comm (devices/) sin saber
// nada de registros ni de interrupciones.
//
// Convencion de sufijos para nombres de funcion en este proyecto:
//   _init -> inicializacion, corre una sola vez al arrancar el firmware.
//   _step -> corre dentro del for(;;) infinito (loop principal).
//   _tic  -> corre dentro de una interrupcion de timer.
//   _isr  -> corre dentro de una interrupcion que no es de timer.

void echo_app_init(void)
{
    uart_comm_init(UART_0, ECHO_APP_BAUD_RATE);
}

// Reenvia tal cual cada byte que llega por RX
void echo_app_step(void)
{
    if (uart_comm_data_available(UART_0))
    {
        uint8_t byte = uart_comm_read_byte(UART_0);
        uart_comm_send_byte(UART_0, byte);
    }
}
