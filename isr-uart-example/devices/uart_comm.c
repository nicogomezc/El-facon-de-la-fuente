#include "includes.h"

typedef struct
{
    uint8_t data[UART_BUFFER_SIZE];
    uint8_t head; // proxima posicion libre para escribir
    uint8_t tail; // proxima posicion de la que leer
} uart_buffer_t;

typedef struct
{
    uart_buffer_t rx;
    uart_buffer_t tx;
} uart_comm_t;
static uart_comm_t uart_comm[UART_COUNT];

static void uart_buffer_init(uart_buffer_t *buffer)
{
    buffer->head = 0;
    buffer->tail = 0;
}

static bool uart_buffer_is_empty(uart_buffer_t *buffer)
{
    return buffer->head == buffer->tail;
}

static void uart_buffer_push(uart_buffer_t *buffer, uint8_t byte)
{
    buffer->data[buffer->head] = byte;
    buffer->head = (buffer->head + 1) % UART_BUFFER_SIZE;
}

static bool uart_buffer_pop(uart_buffer_t *buffer, uint8_t *byte)
{
    if (uart_buffer_is_empty(buffer))
    {
        return false; // no hay nada para leer
    }
    *byte = buffer->data[buffer->tail];
    buffer->tail = (buffer->tail + 1) % UART_BUFFER_SIZE;
    return true;
}

void uart_comm_init(uart_id_t uart_id, uint32_t baudrate)
{
    uart_driver_init(uart_id, baudrate);

    uart_buffer_init(&uart_comm[uart_id].rx);
    uart_buffer_init(&uart_comm[uart_id].tx);
}

void uart_comm_send_byte(uart_id_t uart_id, uint8_t byte)
{
    uart_hw_t *hw = uart_driver_get_hw(uart_id);
    uart_buffer_t *tx_buffer = &uart_comm[uart_id].tx;

    // La interrupcion de TX solo avisa cuando el registro pasa de ocupado
    // a libre: si la UART ya estaba en silencio, nunca se va a disparar
    // sola. Por eso, si no hay nada en cola, mandamos este byte a mano.
    if (uart_buffer_is_empty(tx_buffer) && (hw->fr & UART_UARTFR_TXFE_BITS))
    {
        hw->dr = byte;
        return;
    }
    uart_buffer_push(tx_buffer, byte);
    // Me aseguro de que la interrupcion de TX este habilitada para
    // que la ISR del driver vaya sacando los bytes del buffer
    hw->imsc |= UART_UARTIMSC_TXIM_BITS;
}

bool uart_comm_data_available(uart_id_t uart_id)
{
    return !uart_buffer_is_empty(&uart_comm[uart_id].rx);
}

uint8_t uart_comm_read_byte(uart_id_t uart_id)
{
    uint8_t byte;
    uart_buffer_pop(&uart_comm[uart_id].rx, &byte);
    return byte;
}

void uart_comm_on_byte_received(uart_id_t uart_id, uint8_t byte)
{
    uart_buffer_push(&uart_comm[uart_id].rx, byte);
}

bool uart_comm_get_next_byte_to_send(uart_id_t uart_id, uint8_t *byte)
{
    return uart_buffer_pop(&uart_comm[uart_id].tx, byte);
}
