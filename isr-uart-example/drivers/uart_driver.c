#include "includes.h"

// Tablas indexadas por uart_id_t: reset bit, pines TX/RX (primer par de la
// tabla de la Seccion 3 del apunte) y punteros a periferico/IRQ.
static const uint32_t uart_reset_bits[UART_COUNT] = {RESETS_RESET_UART0_BITS, RESETS_RESET_UART1_BITS};
static const uint uart_tx_pin[UART_COUNT] = {0, 4};
static const uint uart_rx_pin[UART_COUNT] = {1, 5};
static uart_hw_t *const uart_hw_inst[UART_COUNT] = {uart0_hw, uart1_hw};
static const uint uart_irq_num[UART_COUNT] = {UART0_IRQ, UART1_IRQ};

uart_hw_t *uart_driver_get_hw(uart_id_t uart_id)
{
    return uart_hw_inst[uart_id];
}

uint uart_driver_get_irq_num(uart_id_t uart_id)
{
    return uart_irq_num[uart_id];
}

// La IRQ es parte del driver (registros ris/dr/icr/imsc), pero el dato en
// si (el buffer de RX/TX) es de uart_comm: por eso el handler llama
// directo a una funcion de uart_comm para guardar/sacar el byte. Esto
// cruza las capas (el driver termina conociendo a uart_comm) y no es lo
// mas prolijo: la forma correcta seria que uart_comm le pase al driver un
// puntero a funcion (callback) para no depender de el. Se hace asi, cruzado,
// unicamente porque todavia no vimos punteros a funcion en la materia.
static void uart_common_isr(uart_id_t uart_id)
{
    uart_hw_t *hw = uart_hw_inst[uart_id];
    // Recepcion: guardo el byte que llego
    if (hw->ris & UART_UARTRIS_RXRIS_BITS)
    {
        uint8_t byte = (uint8_t)(hw->dr & 0xFF);
        uart_comm_on_byte_received(uart_id, byte);
        hw->icr = UART_UARTICR_RXIC_BITS;
    }
    // Transmision: saco el proximo byte para mandar, si hay
    if (hw->ris & UART_UARTRIS_TXRIS_BITS)
    {
        uint8_t byte;
        if (uart_comm_get_next_byte_to_send(uart_id, &byte))
        {
            hw->dr = byte;
        }
        else
        {
            // No queda nada para mandar: apago la interrupcion de TX
            hw->imsc &= ~UART_UARTIMSC_TXIM_BITS;
        }
        hw->icr = UART_UARTICR_TXIC_BITS;
    }
}
// irq_set_exclusive_handler exige una funcion sin parametros por UART:
// cada wrapper solo le pasa el numero fijo de su UART al handler comun.
static void uart0_isr(void) { uart_common_isr(UART_0); }
static void uart1_isr(void) { uart_common_isr(UART_1); }
static void (*const uart_irq_handlers[UART_COUNT])(void) = {uart0_isr, uart1_isr};

void uart_driver_init(uart_id_t uart_id, uint32_t baudrate)
{
    uart_hw_t *hw = uart_hw_inst[uart_id];
    uint32_t reset_bits = uart_reset_bits[uart_id];
    uint tx_pin = uart_tx_pin[uart_id];
    uint rx_pin = uart_rx_pin[uart_id];

    // 1. Saco la UART del estado de reset
    resets_hw->reset &= ~reset_bits;
    while (!(resets_hw->reset_done & reset_bits))
    {
        // espera activa
    }

    // 2. Baud rate: Divisor = UARTCLK / (16 x baudrate)
    double divisor = (double)UART_CLOCK_HZ / (16.0 * baudrate);
    uint32_t ibrd = (uint32_t)divisor;
    uint32_t fbrd = (uint32_t)((divisor - ibrd) * 64.0 + 0.5);
    hw->ibrd = ibrd;
    hw->fbrd = fbrd;

    // 3. Trama: 8 bits, sin paridad, 1 bit de stop, FIFOs deshabilitadas
    hw->lcr_h = (hw->lcr_h & ~UART_UARTLCR_H_WLEN_BITS) |
                (0b11 << UART_UARTLCR_H_WLEN_LSB);
    hw->lcr_h = (hw->lcr_h & ~UART_UARTLCR_H_STP2_BITS) |
                (0b0 << UART_UARTLCR_H_STP2_LSB);
    hw->lcr_h = (hw->lcr_h & ~UART_UARTLCR_H_PEN_BITS) |
                (0b0 << UART_UARTLCR_H_PEN_LSB);
    hw->lcr_h &= ~UART_UARTLCR_H_FEN_BITS;

    // 4. Pines TX y RX en funcion UART
    // Las macros PADS_BANK0_GPIO0_..._BITS valen lo mismo para cualquier
    // pin (el numero en el nombre es solo historico), asi que sirven
    // igual sin importar el pin real que se este configurando.
    pads_bank0_hw->io[tx_pin] |= PADS_BANK0_GPIO0_IE_BITS;
    pads_bank0_hw->io[tx_pin] &= ~PADS_BANK0_GPIO0_OD_BITS;
    io_bank0_hw->io[tx_pin].ctrl = GPIO_FUNC_UART;
    pads_bank0_hw->io[rx_pin] |= PADS_BANK0_GPIO0_IE_BITS;
    pads_bank0_hw->io[rx_pin] &= ~PADS_BANK0_GPIO0_OD_BITS;
    io_bank0_hw->io[rx_pin].ctrl = GPIO_FUNC_UART;
#if HAS_PADS_BANK0_ISOLATION
    // En el RP2350 los pines arrancan "desconectados" para ahorrar energia.
    // Recien que ya elegimos su funcion los reconectamos, si no la UART
    // no anda aunque el resto de la configuracion este bien.
    pads_bank0_hw->io[tx_pin] &= ~PADS_BANK0_GPIO0_ISO_BITS;
    pads_bank0_hw->io[rx_pin] &= ~PADS_BANK0_GPIO0_ISO_BITS;
#endif

    // 5. Habilito la UART, TX y RX
    hw->cr = UART_UARTCR_UARTEN_BITS |
             UART_UARTCR_TXE_BITS |
             UART_UARTCR_RXE_BITS;

    // Interrupcion de RX (la de TX se habilita desde uart_comm al enviar)
    hw->imsc |= UART_UARTIMSC_RXIM_BITS;
    irq_set_exclusive_handler(uart_irq_num[uart_id], uart_irq_handlers[uart_id]);
    irq_set_enabled(uart_irq_num[uart_id], true);
}
