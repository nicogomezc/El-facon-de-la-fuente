/*
 * console.c
 * Librería para manejo de consola en modo texto.
 * Compatible con Windows y sistemas tipo Unix (Linux, macOS).
 */

#include "console.h"
#include <stdio.h>

/* ------------------------------------------------------------------ */
/*  Windows                                                             */
/* ------------------------------------------------------------------ */
#ifdef _WIN32
#include <windows.h>
// hola prueba 23:25
void scr_enable_utf8(void) {
    /* Configura el code page de salida y entrada a UTF-8 (65001) */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
}

int scr_max_x(void) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    return info.srWindow.Right - info.srWindow.Left;
}

int scr_max_y(void) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    return info.srWindow.Bottom - info.srWindow.Top;
}

void scr_go_to_xy(int x, int y) {
    if (x < 0 || x > scr_max_x() || y < 0 || y > scr_max_y()) return;
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

int scr_get_cursor_x(void) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    return info.dwCursorPosition.X;
}

int scr_get_cursor_y(void) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    return info.dwCursorPosition.Y;
}

void scr_clear_screen(void) {
    system("cls");
}

void scr_hide_cursor(void) {
    CONSOLE_CURSOR_INFO info;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    info.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void scr_show_cursor(void) {
    CONSOLE_CURSOR_INFO info;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    info.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

/* Estado interno: guarda el atributo actual para poder modificar solo  */
/* el texto o solo el fondo sin pisar el otro.                          */
static WORD current_attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

void scr_set_text_color(color_t c) {
    /* Los 4 bits bajos del atributo de Windows corresponden al color    */
    /* de texto; los 4 bits altos al color de fondo.                     */
    current_attributes = (current_attributes & 0xF0) | (WORD)c;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), current_attributes);
}

void scr_set_background_color(color_t c) {
    /* Desplazamos el color 4 bits a la izquierda para ubicarlo en los   */
    /* bits de fondo del atributo de Windows.                            */
    current_attributes = (current_attributes & 0x0F) | ((WORD)c << 4);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), current_attributes);
}

void scr_reset_color(void) {
    /* Blanco sobre negro: el estado por defecto de la consola Windows */
    current_attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), current_attributes);
}

void scr_set_title(const char *title) {
    SetConsoleTitleA(title);
}

/* ------------------------------------------------------------------ */
/*  Unix (Linux / macOS)                                                */
/* ------------------------------------------------------------------ */
#else
#include <sys/ioctl.h>
#include <unistd.h>

void scr_enable_utf8(void) {
    /* En Unix la terminal ya usa UTF-8 por defecto; no se requiere      */
    /* ninguna acción. La función existe para mantener compatibilidad     */
    /* con el código escrito para Windows.                               */
}

int scr_max_x(void) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    return ws.ws_col - 1;
}

int scr_max_y(void) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    return ws.ws_row - 1;
}

void scr_go_to_xy(int x, int y) {
    if (x < 0 || x > scr_max_x() || y < 0 || y > scr_max_y()) return;
    /* Las secuencias ANSI usan base 1; x e y son base 0 desde afuera */
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

int scr_get_cursor_x(void) {
    /* Pedimos la posición del cursor con la secuencia CPR (Cursor         */
    /* Position Report). La terminal responde con \033[fila;columnaR       */
    int x, y;
    printf("\033[6n");
    fflush(stdout);
    scanf("\033[%d;%dR", &y, &x);
    return x - 1;   /* convertimos de base 1 a base 0 */
}

int scr_get_cursor_y(void) {
    int x, y;
    printf("\033[6n");
    fflush(stdout);
    scanf("\033[%d;%dR", &y, &x);
    return y - 1;   /* convertimos de base 1 a base 0 */
}

void scr_clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void scr_hide_cursor(void) {
    printf("\033[?25l");
    fflush(stdout);
}

void scr_show_cursor(void) {
    printf("\033[?25h");
    fflush(stdout);
}

/* Tabla de traducción: índice = valor del enum color_t (0–15),         */
/* valor = código ANSI correspondiente.                                  */
/* Colores normales  (0–7)  → códigos 30–37 / 40–47                     */
/* Colores brillantes (8–15) → códigos 90–97 / 100–107                  */
static const int ansi_color_table[16] = {
    30,  /* COLOR_BLACK        → negro                        */
    34,  /* COLOR_DARK_BLUE    → azul oscuro                  */
    32,  /* COLOR_DARK_GREEN   → verde oscuro                 */
    36,  /* COLOR_DARK_CYAN    → cian oscuro                  */
    31,  /* COLOR_DARK_RED     → rojo oscuro                  */
    35,  /* COLOR_DARK_MAGENTA → magenta oscuro               */
    33,  /* COLOR_DARK_YELLOW  → amarillo oscuro              */
    37,  /* COLOR_GRAY         → gris (blanco normal)         */
    90,  /* COLOR_DARK_GRAY    → gris oscuro (negro brillante)*/
    94,  /* COLOR_BLUE         → azul brillante               */
    92,  /* COLOR_GREEN        → verde brillante              */
    96,  /* COLOR_CYAN         → cian brillante               */
    91,  /* COLOR_RED          → rojo brillante               */
    95,  /* COLOR_MAGENTA      → magenta brillante            */
    93,  /* COLOR_YELLOW       → amarillo brillante           */
    97   /* COLOR_WHITE        → blanco brillante             */
};

void scr_set_text_color(color_t c) {
    printf("\033[%dm", ansi_color_table[c]);
    fflush(stdout);
}

void scr_set_background_color(color_t c) {
    /* El código de fondo ANSI es siempre el de texto + 10 */
    printf("\033[%dm", ansi_color_table[c] + 10);
    fflush(stdout);
}

void scr_reset_color(void) {
    /* La secuencia \033[0m restaura todos los atributos al estado        */
    /* por defecto de la terminal                                          */
    printf("\033[0m");
    fflush(stdout);
}

void scr_set_title(const char *title) {
    /* Secuencia OSC 2: cambia el título de la ventana de la terminal */
    printf("\033]2;%s\007", title);
    fflush(stdout);
}

#endif /* _WIN32 */

/* ------------------------------------------------------------------ */
/*  Dibujo — igual en ambas plataformas                                 */
/* ------------------------------------------------------------------ */

void scr_draw_box(int x, int y, int w, int h) {
    int i;

    /* Esquinas y bordes en Unicode box-drawing */
    /* ┌ ─ ┐ │ └ ┘                              */

    /* Borde superior */
    scr_go_to_xy(x, y);
    printf("\u250C");
    for (i = 1; i < w - 1; i++) printf("\u2500");
    printf("\u2510");

    /* Bordes laterales */
    for (i = 1; i < h - 1; i++) {
        scr_go_to_xy(x, y + i);
        printf("\u2502");
        scr_go_to_xy(x + w - 1, y + i);
        printf("\u2502");
    }

    /* Borde inferior */
    scr_go_to_xy(x, y + h - 1);
    printf("\u2514");
    for (i = 1; i < w - 1; i++) printf("\u2500");
    printf("\u2518");

    fflush(stdout);
}