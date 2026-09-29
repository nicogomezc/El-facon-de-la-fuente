#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include <stdbool.h>
#include <stdint.h>
#include "configuracion.h"
#include "eventos.h"

#define ARCHIVO_CONFIG   "config.json"
#define ARCHIVO_EVENTOS  "eventos.json"
#define MAX_EVENTOS      50

// Carga la configuración desde config.json. Si no existe, asigna valores por defecto y lo crea.
bool persistencia_cargar_config(configuracion_t *cfg);

// Guarda la configuración actual en config.json
bool persistencia_guardar_config(const configuracion_t *cfg);

// Carga los eventos acumulados desde eventos.json y devuelve la cantidad leída
uint8_t persistencia_cargar_eventos(evento_t eventos[], uint8_t max_cantidad);

// Agrega un nuevo evento al historial en RAM y persiste el array actualizado en eventos.json
bool persistencia_registrar_evento(evento_t eventos[], uint8_t *cantidad, evento_t nuevo_evento);

#endif // PERSISTENCIA_H