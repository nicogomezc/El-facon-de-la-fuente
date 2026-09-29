#ifndef _SOCKETS_H_
#define _SOCKETS_H_

#include <stdint.h>
#include <stdbool.h>

/* ─── Constantes ──────────────────────────────────────────────────────────── */

#define SOCKETS_TIMEOUT_INFINITE   0       /* Espera indefinidamente          */
#define SOCKETS_IP_LEN             16      /* "xxx.xxx.xxx.xxx\0"             */
#define SOCKETS_LOCALHOST          "127.0.0.1"

/* ─── Tipos ───────────────────────────────────────────────────────────────── */

/* Handle opaco. Mismo patrón que serial_port_t. */
typedef void *socket_t;

/* ─── Inicialización ──────────────────────────────────────────────────────── */

/*
 * Nota: en Windows, Winsock debe inicializarse antes de usar cualquier función
 * de red. Las funciones de este módulo lo hacen automáticamente la primera vez
 * que se las llama. No hace falta que el alumno llame a nada explícitamente.
 */

/* ─── TCP ─────────────────────────────────────────────────────────────────── */

/**
 * @brief  Crea un servidor TCP y espera a que un cliente se conecte.
 *
 *         Si timeout_ms es SOCKETS_TIMEOUT_INFINITE (0), espera
 *         indefinidamente hasta recibir una conexión entrante.
 *
 * @param  port        Puerto en el que escuchar. Ej: 8080
 * @param  timeout_ms  Tiempo máximo de espera en ms. 0 = infinito.
 * @return Handle de la conexión aceptada, o NULL si venció el timeout o hubo error.
 */
socket_t tcp_server_accept(uint16_t port, uint32_t timeout_ms);

/**
 * @brief  Conecta a un servidor TCP.
 *
 *         Si timeout_ms es SOCKETS_TIMEOUT_INFINITE (0), espera
 *         indefinidamente hasta lograr la conexión.
 *
 * @param  ip          IP del servidor. Ej: "192.168.1.10"
 * @param  port        Puerto del servidor. Ej: 8080
 * @param  timeout_ms  Tiempo máximo para establecer la conexión. 0 = infinito.
 * @return Handle de la conexión, o NULL si venció el timeout o hubo error.
 */
socket_t tcp_client_connect(const char *ip, uint16_t port, uint32_t timeout_ms);

/**
 * @brief  Envía un string por una conexión TCP.
 * @param  conn     Handle de la conexión.
 * @param  message  String a enviar (sin el '\0').
 * @return true si el envío fue exitoso.
 */
bool tcp_send(socket_t conn, const char *message);

/**
 * @brief  Recibe datos por TCP.
 *
 *         Mismo comportamiento de timeout que serial_recv:
 *         - Si vence el timeout con datos parciales, devuelve lo recibido.
 *         - Si vence sin haber recibido nada, devuelve -1.
 *         - Si la conexión fue cerrada por el otro extremo, devuelve 0.
 *
 * @param  conn        Handle de la conexión.
 * @param  buffer      Buffer de destino (se agrega '\0' al final).
 * @param  max_len     Tamaño máximo del buffer.
 * @param  timeout_ms  Tiempo máximo de espera en ms. 0 = infinito.
 * @return Bytes recibidos (>= 1), 0 si la conexión se cerró, -1 si no llegó nada o hubo error.
 */
int32_t tcp_recv(socket_t conn, char *buffer,
                 uint32_t max_len, uint32_t timeout_ms);

/**
 * @brief  Cierra una conexión TCP y libera el handle.
 * @param  conn  Handle de la conexión.
 */
void tcp_close(socket_t conn);

/* ─── UDP ─────────────────────────────────────────────────────────────────── */

/**
 * @brief  Crea un socket UDP listo para enviar y/o recibir.
 * @param  local_port  Puerto local donde escuchar. Usar 0 si solo se va a enviar.
 * @return Handle del socket, o NULL si falló.
 */
socket_t udp_open(uint16_t local_port);

/**
 * @brief  Envía un datagrama UDP.
 * @param  sock      Handle del socket.
 * @param  dest_ip   IP de destino. Ej: "192.168.1.20"
 * @param  dest_port Puerto de destino.
 * @param  message   String a enviar.
 * @return true si el envío fue exitoso.
 */
bool udp_send(socket_t sock, const char *dest_ip,
              uint16_t dest_port, const char *message);

/**
 * @brief  Recibe un datagrama UDP.
 *
 *         Si timeout_ms es SOCKETS_TIMEOUT_INFINITE (0), espera
 *         indefinidamente hasta recibir un datagrama.
 *         Si vence el timeout sin recibir nada, devuelve -1.
 *
 * @param  sock        Handle del socket.
 * @param  buffer      Buffer de destino (se agrega '\0' al final).
 * @param  max_len     Tamaño del buffer.
 * @param  timeout_ms  Tiempo máximo de espera en ms. 0 = infinito.
 * @param  src_ip      Si no es NULL, se llena con la IP del remitente (char[16]).
 * @return Bytes recibidos (>= 1), o -1 si venció el timeout o hubo error.
 */
int32_t udp_recv(socket_t sock, char *buffer, uint32_t max_len,
                 uint32_t timeout_ms, char *src_ip);

/**
 * @brief  Cierra un socket UDP y libera el handle.
 * @param  sock  Handle del socket.
 */
void udp_close(socket_t sock);

#endif /* _SOCKETS_H_ */
