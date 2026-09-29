#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdint.h>
#include <stdbool.h>

/* ─── Constantes ──────────────────────────────────────────────────────────── */

#define SERIAL_TIMEOUT_INFINITE   0       /* Espera indefinidamente           */
#define SERIAL_MAX_PORTS          32      /* Máximo de puertos a listar       */
#define SERIAL_PORT_NAME_LEN      8       /* Ej: "COM10\0" + margen           */
#define SERIAL_ADAPTER_NAME_LEN   128     /* Nombre del adaptador en el SO    */

/* ─── Tipos ───────────────────────────────────────────────────────────────── */

/* Handle opaco. Los chicos no necesitan saber qué hay adentro. */
typedef void *serial_port_t;

typedef struct {
    char port[SERIAL_PORT_NAME_LEN];         /* Ej: "COM3"                    */
    char adapter[SERIAL_ADAPTER_NAME_LEN];   /* Ej: "USB-SERIAL CH340 (COM3)" */
} serial_info_t;

/* ─── Funciones de descubrimiento ─────────────────────────────────────────── */

/**
 * @brief  Lista los puertos COM disponibles en el sistema.
 * @param  list      Array de serial_info_t donde guardar los resultados.
 * @param  max_count Tamaño del array (máximos puertos a devolver).
 * @return Cantidad de puertos encontrados.
 */
uint8_t serial_list_ports(serial_info_t list[], uint8_t max_count);

/**
 * @brief  Imprime en consola los puertos COM disponibles con su nombre de adaptador.
 *         Útil para debug y para que el alumno identifique el puerto correcto.
 */
void serial_print_ports(void);

/**
 * @brief  Busca el primer puerto COM cuyo nombre de adaptador contenga el substring dado.
 * @param  name     Substring a buscar en el nombre del adaptador.
 * @param  out_port Buffer donde se copia el nombre del puerto (ej. "COM3").
 * @param  out_size Tamaño del buffer out_port.
 * @return true si encontró un puerto, false si no.
 */
bool serial_find_by_name(const char *name, char *out_port, uint8_t out_size);

/* ─── Apertura y cierre ───────────────────────────────────────────────────── */

/**
 * @brief  Abre y configura un puerto serie.
 * @param  port      Nombre del puerto. Ej: "COM3"
 * @param  baudrate  Velocidad en bps. Ej: 9600, 115200
 * @return Handle del puerto abierto, o NULL si falló.
 */
serial_port_t serial_open(const char *port, uint32_t baudrate);

/**
 * @brief  Cierra el puerto serie y libera el handle.
 * @param  port  Handle obtenido con serial_open.
 */
void serial_close(serial_port_t port);

/* ─── Envío ───────────────────────────────────────────────────────────────── */

/**
 * @brief  Envía un bloque de bytes por el puerto.
 * @param  port  Handle del puerto.
 * @param  data  Buffer con los datos a enviar.
 * @param  len   Cantidad de bytes a enviar.
 * @return true si el envío fue exitoso.
 */
bool serial_send(serial_port_t port, const uint8_t *data, uint32_t len);

/**
 * @brief  Envía un string (sin el '\0') por el puerto.
 * @param  port  Handle del puerto.
 * @param  str   String terminado en '\0'.
 * @return true si el envío fue exitoso.
 */
bool serial_send_string(serial_port_t port, const char *str);

/* ─── Recepción ───────────────────────────────────────────────────────────── */

/**
 * @brief  Recibe hasta max_len bytes del puerto.
 *
 *         Si timeout_ms es SERIAL_TIMEOUT_INFINITE (0), espera indefinidamente
 *         hasta recibir al menos un byte.
 *         Si vence el timeout habiendo recibido datos parciales, devuelve
 *         lo recibido hasta ese momento.
 *         Si vence el timeout sin haber recibido nada, devuelve -1.
 *
 * @param  port        Handle del puerto.
 * @param  buffer      Buffer donde guardar los datos recibidos.
 * @param  max_len     Tamaño máximo del buffer.
 * @param  timeout_ms  Tiempo máximo de espera en ms. 0 = infinito.
 * @return Bytes recibidos (>= 1), o -1 si no llegó nada / hubo error.
 */
int32_t serial_recv(serial_port_t port, uint8_t *buffer,
                    uint32_t max_len, uint32_t timeout_ms);

/**
 * @brief  Recibe caracteres hasta encontrar '\n' o agotar el buffer.
 *
 *         Mismo comportamiento de timeout que serial_recv.
 *         El '\n' se incluye en el buffer. Se agrega '\0' al final.
 *
 * @param  port        Handle del puerto.
 * @param  buffer      Buffer donde guardar la línea.
 * @param  max_len     Tamaño máximo del buffer (incluyendo '\0').
 * @param  timeout_ms  Tiempo máximo de espera en ms. 0 = infinito.
 * @return Bytes recibidos (>= 1), o -1 si no llegó nada / hubo error.
 */
int32_t serial_recv_line(serial_port_t port, char *buffer,
                         uint32_t max_len, uint32_t timeout_ms);

#endif /* _SERIAL_H_ */
