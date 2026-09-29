#ifndef PANEL_H
#define PANEL_H

#include "estado.h"

// Inicializa la consola (ocultar cursor, configurar pantalla si aplica)
void panel_init(void);

// Dibuja o refresca el panel completo con los datos del struct estado_t
void panel_actualizar(const estado_t *est);

#endif // PANEL_H