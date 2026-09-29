#include "serial.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

/* ─── Defines internos ────────────────────────────────────────────────────── */

#define SERIAL_READ_INTERVAL_MS     1
#define SERIAL_REGISTRY_BASE        "HARDWARE\\DEVICEMAP\\SERIALCOMM"
#define SERIAL_FRIENDLY_NAME_KEY    "SYSTEM\\CurrentControlSet\\Enum"

/* ─── Tipos internos ──────────────────────────────────────────────────────── */

/* Estructura real detrás del handle opaco. */
typedef struct {
    HANDLE handle;
} serial_ctx_t;

/* ─── Helpers estáticos ───────────────────────────────────────────────────── */

/* Construye el path extendido que necesita CreateFile para COM > 9. */
static void build_port_path(const char *port, char *out, uint32_t out_len) {
    snprintf(out, out_len, "\\\\.\\%s", port);
}

/*
 * Intenta obtener el nombre amigable del adaptador para un puerto dado
 * consultando el registro de Windows. Devuelve false si no lo encuentra,
 * en cuyo caso out queda como string vacío.
 *
 * La ruta consultada es:
 *   HKLM\SYSTEM\CurrentControlSet\Enum\<subkeys>\...\FriendlyName
 * Se busca de forma recursiva hasta encontrar un valor FriendlyName cuyo
 * contenido incluya el nombre del puerto (ej. "COM3").
 */
static bool find_friendly_name(HKEY root, const char *port_name,
                                char *out, uint32_t out_len) {
    char subkey_name[256];
    DWORD subkey_len;
    HKEY subkey;
    DWORD index = 0;

    while(true) {
        subkey_len = sizeof(subkey_name);
        if(RegEnumKeyExA(root, index++, subkey_name, &subkey_len,
                         NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
            break;
        }

        if(RegOpenKeyExA(root, subkey_name, 0, KEY_READ, &subkey) != ERROR_SUCCESS) {
            continue;
        }

        /* Intentar leer FriendlyName en este nivel (UTF‑16 → UTF‑8) */
        wchar_t friendly_wide[SERIAL_ADAPTER_NAME_LEN];
        DWORD friendly_len = sizeof(friendly_wide);
        DWORD type;
        if(RegQueryValueExW(subkey, L"FriendlyName", NULL, &type,
                            (LPBYTE)friendly_wide, &friendly_len) == ERROR_SUCCESS) {
            char friendly[SERIAL_ADAPTER_NAME_LEN];
            int len = WideCharToMultiByte(CP_UTF8, 0, friendly_wide, -1,
                                          friendly, sizeof(friendly), NULL, NULL);
            if(len > 0) {
                char *found = strstr(friendly, port_name);
                if(found != NULL) {
                    size_t port_len = strlen(port_name);
                    char next = found[port_len];
                    if(next == '\0' || next < '0' || next > '9') {
                        strncpy(out, friendly, out_len - 1);
                        out[out_len - 1] = '\0';
                        RegCloseKey(subkey);
                        return true;
                    }
                }
            }
        }

        /* Recursión en subniveles */
        if(find_friendly_name(subkey, port_name, out, out_len)) {
            RegCloseKey(subkey);
            return true;
        }

        RegCloseKey(subkey);
    }

    return false;
}

/* ─── Funciones de descubrimiento ─────────────────────────────────────────── */

uint8_t serial_list_ports(serial_info_t list[], uint8_t max_count) {
    HKEY hkey;
    uint8_t count = 0;

    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, SERIAL_REGISTRY_BASE,
                     0, KEY_READ, &hkey) != ERROR_SUCCESS) {
        return 0;
    }

    DWORD index = 0;
    char value_name[256];
    char port_name[SERIAL_PORT_NAME_LEN];
    DWORD value_name_len, port_len, type;

    while(count < max_count) {
        value_name_len = sizeof(value_name);
        port_len       = sizeof(port_name);

        if(RegEnumValueA(hkey, index++, value_name, &value_name_len,
                         NULL, &type, (LPBYTE)port_name,
                         &port_len) != ERROR_SUCCESS) {
            break;
        }

        strncpy(list[count].port, port_name, SERIAL_PORT_NAME_LEN - 1);
        list[count].port[SERIAL_PORT_NAME_LEN - 1] = '\0';

        /* Buscar nombre amigable en el registro */
        HKEY henum;
        list[count].adapter[0] = '\0';

        if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, SERIAL_FRIENDLY_NAME_KEY,
                         0, KEY_READ, &henum) == ERROR_SUCCESS) {
            find_friendly_name(henum, port_name,
                               list[count].adapter,
                               SERIAL_ADAPTER_NAME_LEN);
            RegCloseKey(henum);
        }

        /* Si no se encontró nombre amigable, poner solo el nombre del puerto */
        if(list[count].adapter[0] == '\0') {
            strncpy(list[count].adapter, port_name, SERIAL_ADAPTER_NAME_LEN - 1);
        }

        count++;
    }

    RegCloseKey(hkey);
    return count;
}

void serial_print_ports(void) {
    serial_info_t list[SERIAL_MAX_PORTS];
    uint8_t count = serial_list_ports(list, SERIAL_MAX_PORTS);

    if(count == 0) {
        printf("No se encontraron puertos COM disponibles.\n");
        return;
    }

    printf("Puertos COM disponibles (%d):\n", count);
    for(uint8_t i = 0; i < count; i++) {
        printf("  %-6s  %s\n", list[i].port, list[i].adapter);
    }
}

bool serial_find_by_name(const char *name, char *out_port, uint8_t out_size) {
    serial_info_t list[SERIAL_MAX_PORTS];
    uint8_t count = serial_list_ports(list, SERIAL_MAX_PORTS);

    for(uint8_t i = 0; i < count; i++) {
        if(strstr(list[i].adapter, name) != NULL) {
            strncpy(out_port, list[i].port, out_size - 1);
            out_port[out_size - 1] = '\0';
            return true;
        }
    }

    return false;
}

/* ─── Apertura y cierre ───────────────────────────────────────────────────── */

serial_port_t serial_open(const char *port, uint32_t baudrate) {
    char path[32];
    build_port_path(port, path, sizeof(path));

    HANDLE h = CreateFileA(path,
                           GENERIC_READ | GENERIC_WRITE,
                           0, NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

    if(h == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    /* Parámetros de comunicación */
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if(!GetCommState(h, &dcb)) {
        CloseHandle(h);
        return NULL;
    }

    dcb.BaudRate = baudrate;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary  = TRUE;
    dcb.fParity  = FALSE;

    if(!SetCommState(h, &dcb)) {
        CloseHandle(h);
        return NULL;
    }

    /* Timeouts: se configuran dinámicamente en recv según lo que pida el caller */
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout         = SERIAL_READ_INTERVAL_MS;
    timeouts.ReadTotalTimeoutMultiplier  = 0;
    timeouts.ReadTotalTimeoutConstant    = 0;   /* se sobreescribe en recv */
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant   = 500;
    SetCommTimeouts(h, &timeouts);

    serial_ctx_t *ctx = (serial_ctx_t *)malloc(sizeof(serial_ctx_t));
    if(ctx == NULL) {
        CloseHandle(h);
        return NULL;
    }

    ctx->handle = h;
    return (serial_port_t)ctx;
}

void serial_close(serial_port_t port) {
    if(port == NULL) return;
    serial_ctx_t *ctx = (serial_ctx_t *)port;
    CloseHandle(ctx->handle);
    free(ctx);
}

/* ─── Envío ───────────────────────────────────────────────────────────────── */

bool serial_send(serial_port_t port, const uint8_t *data, uint32_t len) {
    if(port == NULL || data == NULL || len == 0) return false;
    serial_ctx_t *ctx = (serial_ctx_t *)port;

    DWORD written = 0;
    return WriteFile(ctx->handle, data, len, &written, NULL) && (written == len);
}

bool serial_send_string(serial_port_t port, const char *str) {
    if(str == NULL) return false;
    return serial_send(port, (const uint8_t *)str, (uint32_t)strlen(str));
}

/* ─── Recepción ───────────────────────────────────────────────────────────── */

/*
 * Configura el timeout total de lectura en el driver.
 * Con timeout_ms = 0 (SERIAL_TIMEOUT_INFINITE) se pone en 0, lo que en
 * Win32 significa "sin timeout total": ReadFile bloquea hasta tener datos.
 */
static void set_read_timeout(HANDLE h, uint32_t timeout_ms) {
    COMMTIMEOUTS timeouts = {0};

    if(timeout_ms == SERIAL_TIMEOUT_INFINITE) {
        /* Sin timeout: ReadIntervalTimeout = 0 → bloquea hasta llenar el buffer */
        timeouts.ReadIntervalTimeout        = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant   = 0;
    } else {
        /*
         * ReadIntervalTimeout = MAXDWORD + ReadTotalTimeoutMultiplier = 0
         * + ReadTotalTimeoutConstant = timeout_ms
         * → regresa en cuanto llega el primer byte o vence el total.
         */
        timeouts.ReadIntervalTimeout        = MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
        timeouts.ReadTotalTimeoutConstant   = timeout_ms;
    }

    timeouts.WriteTotalTimeoutConstant = 500;
    SetCommTimeouts(h, &timeouts);
}

int32_t serial_recv(serial_port_t port, uint8_t *buffer,
                    uint32_t max_len, uint32_t timeout_ms) {
    if(port == NULL || buffer == NULL || max_len == 0) return -1;
    serial_ctx_t *ctx = (serial_ctx_t *)port;

    set_read_timeout(ctx->handle, timeout_ms);

    DWORD read = 0;
    if(!ReadFile(ctx->handle, buffer, max_len, &read, NULL)) {
        return -1;
    }

    return (read > 0) ? (int32_t)read : -1;
}

int32_t serial_recv_line(serial_port_t port, char *buffer,
                         uint32_t max_len, uint32_t timeout_ms) {
    if(port == NULL || buffer == NULL || max_len < 2) return -1;
    serial_ctx_t *ctx = (serial_ctx_t *)port;

    /*
     * Leemos byte a byte para poder detectar el '\n'.
     * El timeout total se aplica sobre toda la operación, no por byte:
     * calculamos el momento límite con GetTickCount64 y abortamos si se supera.
     */
    uint32_t received  = 0;
    uint64_t deadline  = (timeout_ms == SERIAL_TIMEOUT_INFINITE)
                             ? UINT64_MAX
                             : GetTickCount64() + timeout_ms;

    /* Cada ReadFile individual usa un timeout corto para poder chequear el deadline */
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout        = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant   = 50;   /* 50 ms por intento */
    timeouts.WriteTotalTimeoutConstant  = 500;
    SetCommTimeouts(ctx->handle, &timeouts);

    while(received < max_len - 1) {
        if(GetTickCount64() > deadline) break;

        char c;
        DWORD read = 0;
        if(!ReadFile(ctx->handle, &c, 1, &read, NULL)) break;

        if(read == 0) continue;   /* timeout parcial del ReadFile, reintentar */

        buffer[received++] = c;
        if(c == '\n') break;
    }

    buffer[received] = '\0';
    return (received > 0) ? (int32_t)received : -1;
}
