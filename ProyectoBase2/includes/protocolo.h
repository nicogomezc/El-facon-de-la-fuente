#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <stdbool.h>
#include "estado.h"
#include "configuracion.h"

// Parsea una línea de texto recibida desde la Pico y actualiza el struct estado
bool protocolo_parsear_estado(const char *linea, estado_t *estado);

// Convierte la configuración a la trama ASCII acordada para enviar a la Pico
bool protocolo_armar_config(const configuracion_t *cfg, char *buffer_salida, size_t max_len);

#endif // PROTOCOLO_H