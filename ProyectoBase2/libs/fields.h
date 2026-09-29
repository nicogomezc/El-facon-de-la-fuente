#ifndef _FIELDS_H_
#define _FIELDS_H_

#include <stddef.h>   /* offsetof, size_t */

/*
 * fields.h — Descriptor de campos para serialización JSON y SQL
 * ─────────────────────────────────────────────────────────────
 *
 * Este archivo define el mecanismo que permite a json.h y db.h mapear
 * automáticamente entre structs de C y sus representaciones en JSON o SQL.
 *
 * El alumno define su struct normalmente y luego escribe un descriptor
 * que indica qué campos serializar, de qué tipo son, y con qué nombre
 * aparecen en el JSON o en la tabla SQL.
 *
 * El mismo descriptor sirve para json.h y para db.h — no hay que
 * escribirlo dos veces.
 *
 * ── EJEMPLO 1: struct plano ───────────────────────────────────────────────
 *
 *   typedef struct {
 *       char    nombre[50];
 *       uint8_t edad;
 *       float   promedio;
 *       bool    activo;
 *   } alumno_t;
 *
 *   static const field_desc_t alumno_fields[] = {
 *       FIELD_DEF_STRING(alumno_t, nombre,   "nombre"),
 *       FIELD_DEF       (alumno_t, edad,     FIELD_UINT8, "edad"),
 *       FIELD_DEF       (alumno_t, promedio, FIELD_FLOAT, "promedio"),
 *       FIELD_DEF       (alumno_t, activo,   FIELD_BOOL,  "activo"),
 *       FIELD_END
 *   };
 *
 *   JSON generado para un vector de alumnos:
 *   [
 *     { "nombre": "Garcia Juan", "edad": 17, "promedio": 8.5, "activo": true },
 *     { "nombre": "Lopez Maria", "edad": 18, "promedio": 6.25,"activo": true }
 *   ]
 *
 *   Tabla SQL generada por db_create_table():
 *   CREATE TABLE alumnos (
 *       nombre   TEXT,
 *       edad     INTEGER,
 *       promedio REAL,
 *       activo   INTEGER
 *   );
 *
 * ── EJEMPLO 2: struct con array de structs anidado ────────────────────────
 *
 *   El campo anidado debe ser un array de tamaño fijo dentro del struct.
 *   Se define el descriptor del struct interno primero, luego se referencia
 *   desde el externo con FIELD_DEF_ARRAY.
 *
 *   typedef struct {
 *       uint32_t timestamp;
 *       float    valor;
 *       bool     valido;
 *   } lectura_t;
 *
 *   typedef struct {
 *       char      nombre[32];
 *       uint16_t  id;
 *       float     umbral_alarma;
 *       lectura_t historial[5];    // array de structs anidado
 *   } sensor_t;
 *
 *   // Descriptor del struct interno (igual que siempre)
 *   static const field_desc_t lectura_fields[] = {
 *       FIELD_DEF(lectura_t, timestamp, FIELD_UINT32, "timestamp"),
 *       FIELD_DEF(lectura_t, valor,     FIELD_FLOAT,  "valor"),
 *       FIELD_DEF(lectura_t, valido,    FIELD_BOOL,   "valido"),
 *       FIELD_END
 *   };
 *
 *   // Descriptor del struct externo: referencia al interno con FIELD_DEF_ARRAY
 *   //
 *   //   FIELD_DEF_ARRAY(struct_externo, campo, cantidad,
 *   //                   tipo_interno, descriptor_interno, "nombre_json")
 *   //
 *   static const field_desc_t sensor_fields[] = {
 *       FIELD_DEF_STRING(sensor_t, nombre,        "nombre"),
 *       FIELD_DEF       (sensor_t, id,            FIELD_UINT16, "id"),
 *       FIELD_DEF       (sensor_t, umbral_alarma, FIELD_FLOAT,  "umbral_alarma"),
 *       FIELD_DEF_ARRAY (sensor_t, historial, 5,
 *                        lectura_t, lectura_fields, "historial"),
 *       FIELD_END
 *   };
 *
 *   JSON generado:
 *   [
 *     {
 *       "nombre": "Temperatura sala",
 *       "id": 101,
 *       "umbral_alarma": 35.5,
 *       "historial": [
 *         { "timestamp": 1700000000, "valor": 22.3, "valido": true  },
 *         { "timestamp": 1700000060, "valor": 22.8, "valido": true  },
 *         { "timestamp": 1700000120, "valor": 36.1, "valido": true  },
 *         { "timestamp": 1700000180, "valor": 0.0,  "valido": false },
 *         { "timestamp": 1700000240, "valor": 23.0, "valido": true  }
 *       ]
 *     }
 *   ]
 *
 *   Nota: FIELD_DEF_ARRAY no tiene soporte en db.h — SQL no tiene un tipo
 *   nativo para arrays anidados. Para ese caso, usar dos tablas separadas.
 */

/* ─── Tipos de campo soportados ───────────────────────────────────────────── */

typedef enum {
    FIELD_STRING,   /* char[]   — tamaño indicado en el campo size            */
    FIELD_INT8,     /* int8_t                                                  */
    FIELD_UINT8,    /* uint8_t                                                 */
    FIELD_INT16,    /* int16_t                                                 */
    FIELD_UINT16,   /* uint16_t                                                */
    FIELD_INT32,    /* int32_t                                                 */
    FIELD_UINT32,   /* uint32_t                                                */
    FIELD_FLOAT,    /* float                                                   */
    FIELD_BOOL,     /* bool                                                    */
    FIELD_ARRAY,    /* array de structs anidado — solo json.h                 */
} field_type_t;

/* ─── Descriptor de un campo ──────────────────────────────────────────────── */

/*
 * Declaración anticipada: field_desc_t se referencia a sí mismo en el
 * campo nested_fields, necesario para describir arrays anidados.
 */
typedef struct field_desc_t field_desc_t;

struct field_desc_t {
    const char         *name;           /* Nombre en JSON / columna en SQL    */
    field_type_t        type;           /* Tipo de dato                       */
    size_t              offset;         /* offsetof(struct, campo)            */
    size_t              size;           /* FIELD_STRING: sizeof(char[])
                                           FIELD_ARRAY:  sizeof(struct interno) */
    uint8_t             array_count;    /* FIELD_ARRAY: cantidad de elementos */
    const field_desc_t *nested_fields;  /* FIELD_ARRAY: descriptor del interno */
};

/* ─── Centinela de fin de descriptor ──────────────────────────────────────── */

#define FIELD_END   { NULL, 0, 0, 0, 0, NULL }

/* ─── Macros de definición ────────────────────────────────────────────────── */

/*
 * Campo de tipo char[].
 *
 * Uso: FIELD_DEF_STRING(alumno_t, nombre, "nombre")
 */
#define FIELD_DEF_STRING(struct_t, campo, json_name)      \
    { (json_name), FIELD_STRING,                          \
      offsetof(struct_t, campo),                          \
      sizeof(((struct_t *)0)->campo),                     \
      0, NULL }

/*
 * Campo numérico o booleano.
 *
 * Uso: FIELD_DEF(alumno_t, edad, FIELD_UINT8, "edad")
 */
#define FIELD_DEF(struct_t, campo, tipo, json_name)       \
    { (json_name), (tipo),                                \
      offsetof(struct_t, campo),                          \
      0, 0, NULL }

/*
 * Campo que es un array de structs anidado. Solo compatible con json.h.
 *
 * Uso: FIELD_DEF_ARRAY(sensor_t, historial, 5,
 *                      lectura_t, lectura_fields, "historial")
 *
 * Parámetros:
 *   struct_externo  — tipo del struct que contiene el array
 *   campo           — nombre del campo array dentro del struct
 *   cantidad        — número de elementos del array (tamaño fijo)
 *   tipo_interno    — tipo del struct de cada elemento
 *   desc_interno    — descriptor field_desc_t[] del struct interno
 *   json_name       — nombre del campo en el JSON
 */
#define FIELD_DEF_ARRAY(struct_externo, campo, cantidad,              \
                        tipo_interno, desc_interno, json_name)        \
    { (json_name), FIELD_ARRAY,                                       \
      offsetof(struct_externo, campo),                                \
      sizeof(tipo_interno),                                           \
      (cantidad), (desc_interno) }

#endif /* _FIELDS_H_ */
