#include "sockets.h"

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

/* ─── Defines internos ────────────────────────────────────────────────────── */

#define SOCKETS_TCP_BACKLOG    1    /* Cola de conexiones entrantes en servidor */

/* ─── Tipos internos ──────────────────────────────────────────────────────── */

typedef struct {
    SOCKET sock;
} socket_ctx_t;

/* ─── Inicialización de Winsock ───────────────────────────────────────────── */

/*
 * Se inicializa una sola vez, la primera vez que se usa cualquier función
 * del módulo. El alumno no necesita llamar a nada.
 */
static bool winsock_ready = false;

static bool winsock_init(void) {
    if(winsock_ready) return true;
    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
    winsock_ready = true;
    return true;
}

/* ─── Helpers estáticos ───────────────────────────────────────────────────── */

static socket_ctx_t *ctx_create(SOCKET s) {
    socket_ctx_t *ctx = (socket_ctx_t *)malloc(sizeof(socket_ctx_t));
    if(ctx == NULL) return NULL;
    ctx->sock = s;
    return ctx;
}

/*
 * Aplica un timeout de recepción al socket usando setsockopt.
 * Con 0 (SOCKETS_TIMEOUT_INFINITE) se desactiva el timeout.
 */
static void set_recv_timeout(SOCKET s, uint32_t timeout_ms) {
    DWORD tv = timeout_ms;   /* en Windows setsockopt usa DWORD para SO_RCVTIMEO */
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
}

/*
 * Pone el socket en modo no-bloqueante o bloqueante.
 * Usado en tcp_client_connect para implementar timeout de conexión.
 */
static void set_nonblocking(SOCKET s, bool nonblocking) {
    u_long mode = nonblocking ? 1 : 0;
    ioctlsocket(s, FIONBIO, &mode);
}

/* ─── TCP ─────────────────────────────────────────────────────────────────── */

socket_t tcp_server_accept(uint16_t port, uint32_t timeout_ms) {
    if(!winsock_init()) return NULL;

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server == INVALID_SOCKET) return NULL;

    /* Reutilizar puerto si quedó en TIME_WAIT */
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if(bind(server, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(server);
        return NULL;
    }

    if(listen(server, SOCKETS_TCP_BACKLOG) == SOCKET_ERROR) {
        closesocket(server);
        return NULL;
    }

    /* Aplicar timeout al accept usando SO_RCVTIMEO sobre el socket servidor */
    set_recv_timeout(server, timeout_ms);

    SOCKET client = accept(server, NULL, NULL);
    closesocket(server);   /* ya no necesitamos el socket de escucha */

    if(client == INVALID_SOCKET) return NULL;

    /* El socket de cliente hereda configuración limpia — sin timeout por defecto */
    set_recv_timeout(client, SOCKETS_TIMEOUT_INFINITE);

    return (socket_t)ctx_create(client);
}

socket_t tcp_client_connect(const char *ip, uint16_t port, uint32_t timeout_ms) {
    if(!winsock_init() || ip == NULL) return NULL;

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s == INVALID_SOCKET) return NULL;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if(inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
        closesocket(s);
        return NULL;
    }

    if(timeout_ms == SOCKETS_TIMEOUT_INFINITE) {
        /* Conexión bloqueante simple */
        if(connect(s, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
            closesocket(s);
            return NULL;
        }
    } else {
        /*
         * Para aplicar timeout al connect en Windows hay que:
         * 1. Poner el socket en modo no-bloqueante.
         * 2. Llamar a connect (va a devolver WSAEWOULDBLOCK inmediatamente).
         * 3. Usar select() para esperar hasta que el socket sea escribible
         *    (lo que indica que la conexión se estableció) o expire el timeout.
         * 4. Volver a modo bloqueante.
         */
        set_nonblocking(s, true);
        connect(s, (struct sockaddr *)&addr, sizeof(addr));   /* WSAEWOULDBLOCK esperado */

        fd_set write_set;
        FD_ZERO(&write_set);
        FD_SET(s, &write_set);

        struct timeval tv;
        tv.tv_sec  = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        int result = select(0, NULL, &write_set, NULL, &tv);
        set_nonblocking(s, false);

        if(result <= 0 || !FD_ISSET(s, &write_set)) {
            closesocket(s);
            return NULL;
        }

        /* Verificar que la conexión no haya fallado silenciosamente */
        int err = 0;
        int err_len = sizeof(err);
        getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &err_len);
        if(err != 0) {
            closesocket(s);
            return NULL;
        }
    }

    return (socket_t)ctx_create(s);
}

bool tcp_send(socket_t conn, const char *message) {
    if(conn == NULL || message == NULL) return false;
    socket_ctx_t *ctx = (socket_ctx_t *)conn;

    int len    = (int)strlen(message);
    int result = send(ctx->sock, message, len, 0);
    return result == len;
}

int32_t tcp_recv(socket_t conn, char *buffer,
                 uint32_t max_len, uint32_t timeout_ms) {
    if(conn == NULL || buffer == NULL || max_len < 2) return -1;
    socket_ctx_t *ctx = (socket_ctx_t *)conn;

    set_recv_timeout(ctx->sock, timeout_ms);

    int received = recv(ctx->sock, buffer, (int)(max_len - 1), 0);

    if(received > 0) {
        buffer[received] = '\0';
        return (int32_t)received;
    }

    if(received == 0) {
        buffer[0] = '\0';
        return 0;   /* conexión cerrada por el otro extremo */
    }

    return -1;
}

void tcp_close(socket_t conn) {
    if(conn == NULL) return;
    socket_ctx_t *ctx = (socket_ctx_t *)conn;
    shutdown(ctx->sock, SD_BOTH);
    closesocket(ctx->sock);
    free(ctx);
}

/* ─── UDP ─────────────────────────────────────────────────────────────────── */

socket_t udp_open(uint16_t local_port) {
    if(!winsock_init()) return NULL;

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(s == INVALID_SOCKET) return NULL;

    /* Permitir recibir broadcasts */
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(local_port);

    if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(s);
        return NULL;
    }

    return (socket_t)ctx_create(s);
}

bool udp_send(socket_t sock, const char *dest_ip,
              uint16_t dest_port, const char *message) {
    if(sock == NULL || dest_ip == NULL || message == NULL) return false;
    socket_ctx_t *ctx = (socket_ctx_t *)sock;

    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_port   = htons(dest_port);
    if(inet_pton(AF_INET, dest_ip, &dest.sin_addr) != 1) return false;

    int len    = (int)strlen(message);
    int result = sendto(ctx->sock, message, len, 0,
                        (struct sockaddr *)&dest, sizeof(dest));
    return result == len;
}

int32_t udp_recv(socket_t sock, char *buffer, uint32_t max_len,
                 uint32_t timeout_ms, char *src_ip) {
    if(sock == NULL || buffer == NULL || max_len < 2) return -1;
    socket_ctx_t *ctx = (socket_ctx_t *)sock;

    set_recv_timeout(ctx->sock, timeout_ms);

    struct sockaddr_in sender = {0};
    int sender_len = sizeof(sender);

    int received = recvfrom(ctx->sock, buffer, (int)(max_len - 1), 0,
                            (struct sockaddr *)&sender, &sender_len);

    if(received <= 0) return -1;

    buffer[received] = '\0';

    if(src_ip != NULL) {
        inet_ntop(AF_INET, &sender.sin_addr, src_ip, SOCKETS_IP_LEN);
    }

    return (int32_t)received;
}

void udp_close(socket_t sock) {
    if(sock == NULL) return;
    socket_ctx_t *ctx = (socket_ctx_t *)sock;
    closesocket(ctx->sock);
    free(ctx);
}
