#ifndef UART_COMM_H
#define UART_COMM_H

#include "includes.h"

// Con head y tail alcanza para saber si esta vacio (head == tail), pero
// no para saber si esta lleno: por eso el buffer tiene que ser lo bastante
// grande como para que, en la practica, nunca llegue a llenarse.
#define UART_BUFFER_SIZE 64

// Inicializa la UART al baudrate pedido y arma los buffers circulares de RX/TX.
void uart_comm_init(uart_id_t uart_id, uint32_t baudrate);

// Encola un byte para mandar (o lo manda directo si la UART esta libre).
void uart_comm_send_byte(uart_id_t uart_id, uint8_t byte);

// Indica si hay algun byte recibido esperando en el buffer de RX.
bool uart_comm_data_available(uart_id_t uart_id);

// Saca y devuelve el proximo byte recibido. Llamar solo si
// uart_comm_data_available() devolvio true.
uint8_t uart_comm_read_byte(uart_id_t uart_id);

// Usadas por la ISR del driver (uart_driver.c): la IRQ es parte del
// driver, pero el buffer que hay que llenar/vaciar es de este modulo.
void uart_comm_on_byte_received(uart_id_t uart_id, uint8_t byte);
bool uart_comm_get_next_byte_to_send(uart_id_t uart_id, uint8_t *byte);

#endif
