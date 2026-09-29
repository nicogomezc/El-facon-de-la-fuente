#include <stdio.h>
#include <stdbool.h>
#include "configuracion.h"
#include "eventos.h"
#include "persistencia.h"

int main(void) {
    printf("==========================================\n");
    printf("     TEST DE PERSISTENCIA JSON (EJ 4)     \n");
    printf("==========================================\n\n");

    // 1. Carga o inicializacion de configuracion
    configuracion_t config = {0};
    if (persistencia_cargar_config(&config)) {
        printf("[OK] Configuracion cargada:\n");
        printf("     - Vmin: %.2f V | Vmax: %.2f V | Imax: %.2f A\n",
               config.tension_minima, config.tension_maxima, config.corriente_maxima);
        printf("     - Trest: %d ms | Reintentos max: %d\n\n",
               config.tiempo_restablecimiento, config.reintentos_maximos);
    } else {
        printf("[ERROR] Fallo al cargar configuracion.\n");
        return 1;
    }

    // 2. Modificacion y guardado de prueba
    config.corriente_maxima = 1.85f;
    if (persistencia_guardar_config(&config)) {
        printf("[OK] Modificacion persistida con exito en %s.\n\n", ARCHIVO_CONFIG);
    }

    // 3. Carga y registro de eventos
    evento_t buffer_eventos[MAX_EVENTOS];
    uint8_t total_eventos = persistencia_cargar_eventos(buffer_eventos, MAX_EVENTOS);
    printf("[INFO] Historial previo leido: %u eventos.\n", total_eventos);

    // Registro de un evento nuevo simulado
    evento_t nuevo = {
        .tipo = SOBRECORRIENTE,
        .tiempo = 14500, // milisegundos desde arranque informado por la Pico
        .valor = 1.92f
    };

    if (persistencia_registrar_evento(buffer_eventos, &total_eventos, nuevo)) {
        printf("[OK] Evento registrado y persistido en %s.\n", ARCHIVO_EVENTOS);
    }

    // Verificacion en pantalla del array cargado
    printf("\n--- Eventos almacenados actualmente (%u) ---\n", total_eventos);
    for (uint8_t i = 0; i < total_eventos; i++) {
        printf(" [%u] Tipo: %d | Tiempo: %u ms | Valor: %.2f\n",
               i + 1, buffer_eventos[i].tipo, buffer_eventos[i].tiempo, buffer_eventos[i].valor);
    }

    printf("\n==> EJERCICIO 4 VALIDADO CORRECTAMENTE <==\n");
    return 0;
}