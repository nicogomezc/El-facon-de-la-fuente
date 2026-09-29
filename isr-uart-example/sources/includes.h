#ifndef INCLUDES_H
#define INCLUDES_H

// Librerías genéricas
#include "pico/stdlib.h"

// Capa de Drivers (primera, son el código de más bajo nivel)
#include "uart_driver.h"

// Capa de Devices
#include "uart_comm.h"

// Capa de Aplicación (última, código de más alto nivel)
#include "echo_app.h"

#endif
