#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "serial.h"
#include "configuracion.h"
#include "estado.h"
#include "protocolo.h"
#include "persistencia.h"
#include "panel.h"

#define PUERTO_SERIE  "COM4"
#define BAUDRATE      115200
#define BUFFER_TAM    128

int main(void) {
    // 1. Cargar configuracion desde disco JSON (Ejercicio 4)
    configuracion_t config_activa = {0};
    persistencia_cargar_config(&config_activa);

    // 2. Conectar al puerto serie COM4
    serial_port_t sp = serial_open(PUERTO_SERIE, BAUDRATE);
    if (sp == NULL) {
        printf("Error: No se pudo abrir %s.\n", PUERTO_SERIE);
        return 1;
    }

    // 3. Enviar la configuracion vigente a la Pico al iniciar
    char trama_tx[BUFFER_TAM];
    if (protocolo_armar_config(&config_activa, trama_tx, sizeof(trama_tx))) {
        serial_send_string(sp, trama_tx);
    }

    // 4. Inicializar estado y limpiar pantalla para el panel
    estado_t estado = {0};
    estado.configuracion = config_activa;

    panel_init();
    panel_actualizar(&estado);

    char buffer_rx[BUFFER_TAM];

    // 5. Bucle de recepcion y refresco continuo en tiempo real
    while (1) {
        int32_t bytes = serial_recv_line(sp, buffer_rx, sizeof(buffer_rx), 500);

        if (bytes > 0) {
            if (protocolo_parsear_estado(buffer_rx, &estado)) {
                // Mantener los valores de configuracion cargados en memoria
                estado.configuracion = config_activa;
                panel_actualizar(&estado);
            }
        }
    }

    serial_close(sp);
    return 0;
}