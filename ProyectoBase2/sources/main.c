#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "serial.h"
#include "configuracion.h"
#include "estado.h"
#include "protocolo.h"

#define PUERTO_SERIE  "COM4"
#define BAUDRATE      115200
#define BUFFER_TAM    128

int main(void) {
    printf("==========================================\n");
    printf("   TEST COMPLETO EJERCICIO 3 (LOOPBACK)   \n");
    printf("==========================================\n\n");

    serial_port_t sp = serial_open(PUERTO_SERIE, BAUDRATE);
    if (sp == NULL) {
        printf("Error: No se pudo abrir %s.\n", PUERTO_SERIE);
        return 1;
    }
    printf("Conectado con exito a %s (%d bps).\n\n", PUERTO_SERIE, BAUDRATE);

    char buffer_tx[BUFFER_TAM];
    char buffer_rx[BUFFER_TAM];

    // -------------------------------------------------------------
    // PRUEBA 1: PC -> Pico (Armado y transmision de configuracion)
    // -------------------------------------------------------------
    configuracion_t mi_config = {
        .tension_minima = 11.50f,
        .tension_maxima = 12.50f,
        .corriente_maxima = 1.00f,
        .tiempo_restablecimiento = 10,
        .reintentos_maximos = 3
    };

    if (protocolo_armar_config(&mi_config, buffer_tx, sizeof(buffer_tx))) {
        printf("[1] Transmitiendo configuracion (PC -> TX):\n    %s", buffer_tx);
        serial_send_string(sp, buffer_tx);
    }

    // Vaciamos el eco que vuelve por el jumper de loopback
    int32_t bytes = serial_recv_line(sp, buffer_rx, sizeof(buffer_rx), 1000);
    if (bytes > 0) {
        printf("    [RX Loopback]: %s\n", buffer_rx);
    }

    // -------------------------------------------------------------
    // PRUEBA 2: Pico -> PC (Simulacion de recepcion y parseo de estado)
    // -------------------------------------------------------------
    const char *trama_pico = "EST:V=12.18,I=0.42,OUT=1,TCORTE=0,REINT=0\n";
    printf("[2] Simulando envio periodico de la Pico por TX:\n    %s", trama_pico);
    serial_send_string(sp, trama_pico);

    bytes = serial_recv_line(sp, buffer_rx, sizeof(buffer_rx), 1000);
    if (bytes > 0) {
        estado_t estado_recibido = {0};
        if (protocolo_parsear_estado(buffer_rx, &estado_recibido)) {
            printf("\n[3] PARSEO EXITOSO: Estructura estado_t completada:\n");
            printf("    * Tension medida:        %.2f V\n", estado_recibido.tension_actual);
            printf("    * Corriente medida:      %.2f A\n", estado_recibido.corriente_actual);
            printf("    * Estado de la salida:   %s\n", estado_recibido.salida_activa ? "ACTIVA" : "CORTADA");
            printf("    * Tiempo desde corte:    %u s\n", estado_recibido.tiempo_desde_ultimo_corte);
            printf("    * Reintentos usados:     %u\n\n", estado_recibido.reintentos_utilizados);
            printf("==> EJERCICIO 3 VALIDADO CORRECTAMENTE <==\n");
        } else {
            printf("[ERROR] No se pudo parsear la trama de estado.\n");
        }
    } else {
        printf("[ERROR] Timeout en recepcion de la trama.\n");
    }

    serial_close(sp);
    return 0;
}