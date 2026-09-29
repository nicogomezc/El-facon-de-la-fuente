#include <stdio.h>
#include <string.h>
#include "protocolo.h"

bool protocolo_parsear_estado(const char *linea, estado_t *estado) {
    if (linea == NULL || estado == NULL) {
        return false;
    }

    float v = 0.0f;
    float i = 0.0f;
    int out = 0;
    unsigned int t_corte = 0;
    unsigned int reint = 0;

    // Formato: EST:V=%.2f,I=%.2f,OUT=%d,TCORTE=%u,REINT=%u
    int parseados = sscanf(linea, "EST:V=%f,I=%f,OUT=%d,TCORTE=%u,REINT=%u",
                           &v, &i, &out, &t_corte, &reint);

    if (parseados == 5) {
        estado->tension_actual = v;
        estado->corriente_actual = i;
        estado->salida_activa = (out != 0);
        estado->tiempo_desde_ultimo_corte = (uint32_t)t_corte;
        estado->reintentos_utilizados = (uint8_t)reint;
        return true;
    }

    return false;
}

bool protocolo_armar_config(const configuracion_t *cfg, char *buffer_salida, size_t max_len) {
    if (cfg == NULL || buffer_salida == NULL) {
        return false;
    }

    // Formato: CFG:VMIN=%.2f,VMAX=%.2f,IMAX=%.2f,TREST=%d,REINT=%d\n
    int res = snprintf(buffer_salida, max_len,
                       "CFG:VMIN=%.2f,VMAX=%.2f,IMAX=%.2f,TREST=%d,REINT=%d\n",
                       cfg->tension_minima,
                       cfg->tension_maxima,
                       cfg->corriente_maxima,
                       cfg->tiempo_restablecimiento,
                       cfg->reintentos_maximos);

    return (res > 0 && (size_t)res < max_len);
}