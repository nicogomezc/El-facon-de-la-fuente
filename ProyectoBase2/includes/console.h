#ifndef CONSOLE_H
#define CONSOLE_H

/*
 * console.h
 * Librería para manejo de consola en modo texto.
 * Compatible con Windows y sistemas tipo Unix (Linux, macOS).
 */

/* ------------------------------------------------------------------ */
/*  Tipos                                                               */
/* ------------------------------------------------------------------ */

/* Los 16 colores clásicos de consola.                                  */
/* Los valores 0–15 coinciden con los de la API de Windows,             */
/* y se traducen a los códigos ANSI equivalentes en Unix.               */
typedef enum {
    COLOR_BLACK        = 0,
    COLOR_DARK_BLUE    = 1,
    COLOR_DARK_GREEN   = 2,
    COLOR_DARK_CYAN    = 3,
    COLOR_DARK_RED     = 4,
    COLOR_DARK_MAGENTA = 5,
    COLOR_DARK_YELLOW  = 6,
    COLOR_GRAY         = 7,
    COLOR_DARK_GRAY    = 8,
    COLOR_BLUE         = 9,
    COLOR_GREEN        = 10,
    COLOR_CYAN         = 11,
    COLOR_RED          = 12,
    COLOR_MAGENTA      = 13,
    COLOR_YELLOW       = 14,
    COLOR_WHITE        = 15
} color_t;

/* ------------------------------------------------------------------ */
/*  Inicialización                                                       */
/* ------------------------------------------------------------------ */

/* Habilita UTF-8 en la consola para poder mostrar acentos, ñ, etc.    */
/* En Windows configura el code page de entrada y salida a UTF-8.      */
/* En Unix no es necesario: la terminal ya usa UTF-8 por defecto.      */
void scr_enable_utf8(void);

/* ------------------------------------------------------------------ */
/*  Dimensiones de la consola                                           */
/* ------------------------------------------------------------------ */

/* Devuelve el ancho máximo de la consola en columnas */
int scr_max_x(void);

/* Devuelve el alto máximo de la consola en filas */
int scr_max_y(void);

/* ------------------------------------------------------------------ */
/*  Cursor                                                              */
/* ------------------------------------------------------------------ */

/* Mueve el cursor a la posición (x, y). Origen en (0, 0), esquina     */
/* superior izquierda. No hace nada si la posición está fuera de rango. */
void scr_go_to_xy(int x, int y);

/* Devuelve la columna actual del cursor */
int scr_get_cursor_x(void);

/* Devuelve la fila actual del cursor */
int scr_get_cursor_y(void);

/* Limpia toda la pantalla y mueve el cursor al origen */
void scr_clear_screen(void);

/* Oculta el cursor parpadeante */
void scr_hide_cursor(void);

/* Restaura la visibilidad del cursor */
void scr_show_cursor(void);

/* ------------------------------------------------------------------ */
/*  Colores                                                             */
/* ------------------------------------------------------------------ */

/* Cambia el color del texto */
void scr_set_text_color(color_t c);

/* Cambia el color del fondo */
void scr_set_background_color(color_t c);

/* Restaura el color de texto y fondo a los valores por defecto         */
/* de la terminal                                                        */
void scr_reset_color(void);

/* ------------------------------------------------------------------ */
/*  Ventana                                                             */
/* ------------------------------------------------------------------ */

/* Cambia el título de la ventana de la consola */
void scr_set_title(const char *title);

/* ------------------------------------------------------------------ */
/*  Dibujo                                                              */
/* ------------------------------------------------------------------ */

/* Dibuja un rectángulo con caracteres de borde en la posición (x, y)  */
/* con el ancho w y alto h indicados.                                   */
void scr_draw_box(int x, int y, int w, int h);

#endif /* CONSOLE_H */