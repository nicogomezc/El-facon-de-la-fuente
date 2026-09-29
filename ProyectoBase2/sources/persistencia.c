#include <stdio.h>
#include <string.h>
#include "persistencia.h"
#include "fields.h"
#include "json.h"

/* Descriptor para el struct configuracion_t */
static const field_desc_t configuracion_fields[] = {
    FIELD_DEF(configuracion_t, tension_minima,          FIELD_FLOAT, "tension_minima"),
    FIELD_DEF(configuracion_t, tension_maxima,          FIELD_FLOAT, "tension_maxima"),
    FIELD_DEF(configuracion_t, corriente_maxima,        FIELD_FLOAT, "corriente_maxima"),
    FIELD_DEF(configuracion_t, tiempo_restablecimiento, FIELD_INT32, "tiempo_restablecimiento"),
    FIELD_DEF(configuracion_t, reintentos_maximos,      FIELD_INT32, "reintentos_maximos"),
    FIELD_END
};

/* Descriptor para el struct evento_t */
static const field_desc_t evento_fields[] = {
    FIELD_DEF(evento_t, tipo,   FIELD_INT32,  "tipo"),
    FIELD_DEF(evento_t, tiempo, FIELD_UINT32, "tiempo"),
    FIELD_DEF(evento_t, valor,  FIELD_FLOAT,  "valor"),
    FIELD_END
};

bool persistencia_cargar_config(configuracion_t *cfg) {
    if (cfg == NULL) {
        return false;
    }

    // Si no existe config.json o falla la lectura, se cargan valores seguros por defecto
    if (!json_load_object(ARCHIVO_CONFIG, cfg, configuracion_fields)) {
        printf("[PERSISTENCIA] No se encontro %s. Generando valores por defecto...\n", ARCHIVO_CONFIG);
        cfg->tension_minima = 11.0f;
        cfg->tension_maxima = 13.0f;
        cfg->corriente_maxima = 1.2f;
        cfg->tiempo_restablecimiento = 3000;
        cfg->reintentos_maximos = 3;

        return persistencia_guardar_config(cfg);
    }

    return true;
}

bool persistencia_guardar_config(const configuracion_t *cfg) {
    if (cfg == NULL) {
        return false;
    }
    return json_save_object(ARCHIVO_CONFIG, cfg, configuracion_fields);
}

uint8_t persistencia_cargar_eventos(evento_t eventos[], uint8_t max_cantidad) {
    if (eventos == NULL || max_cantidad == 0) {
        return 0;
    }
    return json_load_array(ARCHIVO_EVENTOS, eventos, max_cantidad, sizeof(evento_t), evento_fields);
}

bool persistencia_registrar_evento(evento_t eventos[], uint8_t *cantidad, evento_t nuevo_evento) {
    if (eventos == NULL || cantidad == NULL) {
        return false;
    }

    if (*cantidad < MAX_EVENTOS) {
        eventos[*cantidad] = nuevo_evento;
        (*cantidad)++;
    } else {
        // Desplazamiento FIFO si el vector se llena
        for (uint8_t i = 0; i < MAX_EVENTOS - 1; i++) {
            eventos[i] = eventos[i + 1];
        }
        eventos[MAX_EVENTOS - 1] = nuevo_evento;
    }

    return json_save_array(ARCHIVO_EVENTOS, eventos, *cantidad, sizeof(evento_t), evento_fields);
}