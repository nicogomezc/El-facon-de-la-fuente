#ifndef _JSON_H_
#define _JSON_H_

#include <stdint.h>
#include <stdbool.h>
#include "fields.h"

/* ─── Arrays de structs ───────────────────────────────────────────────────── */

/**
 * @brief  Lee un archivo JSON que contiene un array de objetos y llena
 *         un vector de structs.
 *
 *         El archivo debe tener la forma:
 *         [ { "campo": valor, ... }, { ... }, ... ]
 *
 *         Los campos del JSON que no están en el descriptor se ignoran.
 *         Los campos del descriptor que no están en el JSON quedan en 0.
 *
 * @param  filepath     Ruta al archivo .json.
 * @param  array        Puntero al vector de structs destino.
 * @param  max_count    Capacidad máxima del vector.
 * @param  struct_size  sizeof del struct (usar sizeof(mi_struct_t)).
 * @param  fields       Descriptor de campos.
 * @return Cantidad de elementos leídos, o 0 si hubo error.
 */
uint8_t json_load_array(const char *filepath,
                        void *array, uint8_t max_count, size_t struct_size,
                        const field_desc_t fields[]);

/**
 * @brief  Guarda un vector de structs en un archivo JSON como array de objetos.
 *
 * @param  filepath     Ruta al archivo .json (se crea o sobreescribe).
 * @param  array        Puntero al vector de structs.
 * @param  count        Cantidad de elementos a guardar.
 * @param  struct_size  sizeof del struct.
 * @param  fields       Descriptor de campos.
 * @return true si se guardó correctamente.
 */
bool json_save_array(const char *filepath,
                     const void *array, uint8_t count, size_t struct_size,
                     const field_desc_t fields[]);

/* ─── Objeto único ────────────────────────────────────────────────────────── */

/**
 * @brief  Lee un archivo JSON que contiene un objeto único y lo mapea
 *         a un struct.
 *
 *         El archivo debe tener la forma:
 *         { "campo": valor, ... }
 *
 * @param  filepath  Ruta al archivo .json.
 * @param  out       Puntero al struct destino.
 * @param  fields    Descriptor de campos.
 * @return true si se leyó correctamente.
 */
bool json_load_object(const char *filepath,
                      void *out, const field_desc_t fields[]);

/**
 * @brief  Guarda un struct en un archivo JSON como objeto único.
 *
 * @param  filepath  Ruta al archivo .json (se crea o sobreescribe).
 * @param  in        Puntero al struct.
 * @param  fields    Descriptor de campos.
 * @return true si se guardó correctamente.
 */
bool json_save_object(const char *filepath,
                      const void *in, const field_desc_t fields[]);

#endif /* _JSON_H_ */
