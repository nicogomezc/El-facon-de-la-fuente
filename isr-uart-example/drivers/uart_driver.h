#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "includes.h"

#define UART_CLOCK_HZ 150000000

// Numero de UART: se usa como indice en los arrays internos del driver,
// asi las mismas funciones sirven para cualquier UART sin repetir codigo.
// NUM_UARTS ya esta definido por el SDK (2), asi que el ultimo valor
// del enum se llama distinto para no pisarlo.
typedef enum
{
    UART_0,
    UART_1,
    UART_COUNT
} uart_id_t;

// Inicializa la UART indicada (saca del reset, configura baudrate,
// trama 8N1, pines TX/RX y la habilita). Ver Apuntes/9-UART.
void uart_driver_init(uart_id_t uart_id, uint32_t baudrate);

// Accessors: en vez de exponer los arrays internos del driver (hw
// registers, numero de IRQ) como variables globales, se piden por funcion.
uart_hw_t *uart_driver_get_hw(uart_id_t uart_id);
uint uart_driver_get_irq_num(uart_id_t uart_id);

#endif
