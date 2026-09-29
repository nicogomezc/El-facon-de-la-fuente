#include "json.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* ─── Defines internos ────────────────────────────────────────────────────── */

#define JSON_FILE_MAX_SIZE   (256 * 1024)   /* 256 KB — más que suficiente    */

/* ─── Helpers estáticos ───────────────────────────────────────────────────── */

/* Lee un archivo completo y devuelve su contenido en un buffer dinámico.
 * El llamador es responsable de liberar la memoria con free(). */
static char *read_file(const char *filepath) {
    FILE *f = fopen(filepath, "rb");
    if(f == NULL) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if(size <= 0 || size > JSON_FILE_MAX_SIZE) {
        fclose(f);
        return NULL;
    }

    char *buf = (char *)malloc((size_t)size + 1);
    if(buf == NULL) {
        fclose(f);
        return NULL;
    }

    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

/* Escribe un string en un archivo, sobreescribiéndolo si ya existe. */
static bool write_file(const char *filepath, const char *content) {
    FILE *f = fopen(filepath, "wb");
    if(f == NULL) return false;
    size_t len = strlen(content);
    bool ok = fwrite(content, 1, len, f) == len;
    fclose(f);
    return ok;
}

/* Obtiene un puntero al campo de un struct usando su offset. */
static void *field_ptr(void *base, size_t offset) {
    return (uint8_t *)base + offset;
}

static const void *field_ptr_const(const void *base, size_t offset) {
    return (const uint8_t *)base + offset;
}

/*
 * Lee un objeto cJSON y mapea sus valores al struct apuntado por dest,
 * usando el descriptor de campos.
 */
static void object_to_struct(const cJSON *obj, void *dest,
                              const field_desc_t fields[]) {
    for(int i = 0; fields[i].name != NULL; i++) {
        const field_desc_t *f = &fields[i];
        cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, f->name);
        if(item == NULL) continue;   /* campo no presente → queda en 0 */

        void *ptr = field_ptr(dest, f->offset);

        switch(f->type) {
            case FIELD_STRING:
                if(cJSON_IsString(item)) {
                    strncpy((char *)ptr, item->valuestring, f->size - 1);
                    ((char *)ptr)[f->size - 1] = '\0';
                }
                break;

            case FIELD_INT8:
                if(cJSON_IsNumber(item))
                    *(int8_t *)ptr = (int8_t)item->valueint;
                break;

            case FIELD_UINT8:
                if(cJSON_IsNumber(item))
                    *(uint8_t *)ptr = (uint8_t)item->valueint;
                break;

            case FIELD_INT16:
                if(cJSON_IsNumber(item))
                    *(int16_t *)ptr = (int16_t)item->valueint;
                break;

            case FIELD_UINT16:
                if(cJSON_IsNumber(item))
                    *(uint16_t *)ptr = (uint16_t)item->valueint;
                break;

            case FIELD_INT32:
                if(cJSON_IsNumber(item))
                    *(int32_t *)ptr = (int32_t)item->valueint;
                break;

            case FIELD_UINT32:
                if(cJSON_IsNumber(item))
                    *(uint32_t *)ptr = (uint32_t)(unsigned int)item->valueint;
                break;

            case FIELD_FLOAT:
                if(cJSON_IsNumber(item))
                    *(float *)ptr = (float)item->valuedouble;
                break;

            case FIELD_BOOL:
                if(cJSON_IsBool(item))
                    *(bool *)ptr = cJSON_IsTrue(item);
                break;

            case FIELD_ARRAY:
                if(cJSON_IsArray(item)) {
                    uint8_t idx = 0;
                    cJSON *elem;
                    cJSON_ArrayForEach(elem, item) {
                        if(idx >= f->array_count) break;
                        if(!cJSON_IsObject(elem)) { idx++; continue; }
                        void *inner = (uint8_t *)ptr + idx * f->size;
                        object_to_struct(elem, inner, f->nested_fields);
                        idx++;
                    }
                }
                break;
        }
    }
}

/*
 * Construye un objeto cJSON a partir de un struct,
 * usando el descriptor de campos.
 */
static cJSON *struct_to_object(const void *src, const field_desc_t fields[]) {
    cJSON *obj = cJSON_CreateObject();
    if(obj == NULL) return NULL;

    for(int i = 0; fields[i].name != NULL; i++) {
        const field_desc_t *f = &fields[i];
        const void *ptr = field_ptr_const(src, f->offset);

        switch(f->type) {
            case FIELD_STRING:
                cJSON_AddStringToObject(obj, f->name, (const char *)ptr);
                break;

            case FIELD_INT8:
                cJSON_AddNumberToObject(obj, f->name, *(const int8_t *)ptr);
                break;

            case FIELD_UINT8:
                cJSON_AddNumberToObject(obj, f->name, *(const uint8_t *)ptr);
                break;

            case FIELD_INT16:
                cJSON_AddNumberToObject(obj, f->name, *(const int16_t *)ptr);
                break;

            case FIELD_UINT16:
                cJSON_AddNumberToObject(obj, f->name, *(const uint16_t *)ptr);
                break;

            case FIELD_INT32:
                cJSON_AddNumberToObject(obj, f->name, *(const int32_t *)ptr);
                break;

            case FIELD_UINT32:
                cJSON_AddNumberToObject(obj, f->name, *(const uint32_t *)ptr);
                break;

            case FIELD_FLOAT:
                cJSON_AddNumberToObject(obj, f->name, *(const float *)ptr);
                break;

            case FIELD_BOOL:
                cJSON_AddBoolToObject(obj, f->name, *(const bool *)ptr);
                break;

            case FIELD_ARRAY: {
                cJSON *arr = cJSON_CreateArray();
                if(arr == NULL) { cJSON_Delete(obj); return NULL; }
                for(uint8_t idx = 0; idx < f->array_count; idx++) {
                    const void *inner = (const uint8_t *)ptr + idx * f->size;
                    cJSON *child = struct_to_object(inner, f->nested_fields);
                    if(child == NULL) { cJSON_Delete(arr); cJSON_Delete(obj); return NULL; }
                    cJSON_AddItemToArray(arr, child);
                }
                cJSON_AddItemToObject(obj, f->name, arr);
                break;
            }
        }
    }

    return obj;
}

/* ─── Arrays de structs ───────────────────────────────────────────────────── */

uint8_t json_load_array(const char *filepath,
                        void *array, uint8_t max_count, size_t struct_size,
                        const field_desc_t fields[]) {
    if(filepath == NULL || array == NULL || fields == NULL) return 0;

    char *buf = read_file(filepath);
    if(buf == NULL) return 0;

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if(root == NULL || !cJSON_IsArray(root)) {
        cJSON_Delete(root);
        return 0;
    }

    uint8_t count = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, root) {
        if(count >= max_count) break;
        if(!cJSON_IsObject(item)) continue;

        void *dest = (uint8_t *)array + count * struct_size;
        object_to_struct(item, dest, fields);
        count++;
    }

    cJSON_Delete(root);
    return count;
}

bool json_save_array(const char *filepath,
                     const void *array, uint8_t count, size_t struct_size,
                     const field_desc_t fields[]) {
    if(filepath == NULL || array == NULL || fields == NULL) return false;

    cJSON *root = cJSON_CreateArray();
    if(root == NULL) return false;

    for(uint8_t i = 0; i < count; i++) {
        const void *src = (const uint8_t *)array + i * struct_size;
        cJSON *obj = struct_to_object(src, fields);
        if(obj == NULL) {
            cJSON_Delete(root);
            return false;
        }
        cJSON_AddItemToArray(root, obj);
    }

    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);
    if(json_str == NULL) return false;

    bool ok = write_file(filepath, json_str);
    free(json_str);
    return ok;
}

/* ─── Objeto único ────────────────────────────────────────────────────────── */

bool json_load_object(const char *filepath,
                      void *out, const field_desc_t fields[]) {
    if(filepath == NULL || out == NULL || fields == NULL) return false;

    char *buf = read_file(filepath);
    if(buf == NULL) return false;

    cJSON *root = cJSON_Parse(buf);
    free(buf);
    if(root == NULL || !cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return false;
    }

    object_to_struct(root, out, fields);
    cJSON_Delete(root);
    return true;
}

bool json_save_object(const char *filepath,
                      const void *in, const field_desc_t fields[]) {
    if(filepath == NULL || in == NULL || fields == NULL) return false;

    cJSON *obj = struct_to_object(in, fields);
    if(obj == NULL) return false;

    char *json_str = cJSON_Print(obj);
    cJSON_Delete(obj);
    if(json_str == NULL) return false;

    bool ok = write_file(filepath, json_str);
    free(json_str);
    return ok;
}
