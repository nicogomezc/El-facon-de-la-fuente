#include <stdio.h>
#include <stdbool.h>
#include "panel.h"
#include "console.h"

void panel_init(void) {
    // \033[2J : Limpia la pantalla completa
    // \033[H  : Posiciona el cursor en la esquina superior izquierda (1,1)
    // \033[?25l : Oculta el cursor para evitar parpadeos
    printf("\033[2J\033[H\033[?25l");
    fflush(stdout);
}

void panel_actualizar(const estado_t *est) {
    if (est == NULL) {
        return;
    }

    // Reposiciona el cursor en (1,1) para sobreescribir sin parpadear ni scrollear
    printf("\033[H");

    printf("=====================================================================\n");
    printf("           EL FACON DE LA FUENTE - PANEL DE ESTADO                   \n");
    printf("=====================================================================\n");

    // --- SECCIÓN 1: ESTADO GENERAL DE LA SALIDA ---
    printf("\n  ESTADO GENERAL: ");
    if (est->salida_activa) {
        // Verde brillante
        printf("\033[1;32m[ ACTIVA / NORMAL ]\033[0m              \n\n");
    } else {
        // Rojo brillante
        printf("\033[1;31m[ CORTADA / PROTECCION ACTIVADA ]\033[0m  \n\n");
    }

    // --- SECCIÓN 2: LECTURAS EN TIEMPO REAL ---
    printf("+---------------------------------+---------------------------------+\n");
    printf("|          MEDICION REAL          |        ESTADO DE CORTE          |\n");
    printf("+---------------------------------+---------------------------------+\n");
    printf("| Tension:     %6.2f V            | Reintentos: %2u / %-2d            |\n", 
           est->tension_actual, 
           est->reintentos_utilizados, 
           est->configuracion.reintentos_maximos);

    printf("| Corriente:   %6.2f A            | Tiempo c/ corte: %6u s      |\n", 
           est->corriente_actual, 
           est->tiempo_desde_ultimo_corte);
    printf("+---------------------------------+---------------------------------+\n");

    // --- SECCIÓN 3: CONFIGURACIÓN VIGENTE ---
    printf("\n  CONFIGURACION VIGENTE:\n");
    printf("  -------------------------------------------------------------------\n");
    printf("  * Tension Minima:         %5.2f V                                 \n", est->configuracion.tension_minima);
    printf("  * Tension Maxima:         %5.2f V                                 \n", est->configuracion.tension_maxima);
    printf("  * Corriente Maxima:       %5.2f A                                 \n", est->configuracion.corriente_maxima);
    printf("  * Tiempo Restablecimiento: %4d s                                  \n", est->configuracion.tiempo_restablecimiento);
    printf("  * Maximo Reintentos:       %4d                                    \n", est->configuracion.reintentos_maximos);
    printf("  -------------------------------------------------------------------\n");
    printf("  Presione [Ctrl + C] para salir de la aplicacion.                   \n");
    fflush(stdout);
}