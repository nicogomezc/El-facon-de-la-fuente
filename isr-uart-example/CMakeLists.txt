# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(isr-uart-example C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# El proyecto esta organizado en carpetas por capa: sources/ (main.c y el
# includes.h que agrupa todo), drivers/ (acceso a registros de hardware),
# devices/ (lógica de más alto nivel sobre un driver) y app/ (lógica de
# aplicación, sin saber nada de registros ni interrupciones). Con GLOB no
# hay que tocar este archivo al agregar un .c nuevo dentro de una carpeta
# que ya existe (por ej. un driver de ADC en drivers/).
file(GLOB SOURCES_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/sources/*.c)
file(GLOB DRIVERS_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/drivers/*.c)
file(GLOB DEVICES_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/devices/*.c)
file(GLOB APP_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/app/*.c)

# Add executable. Default name is the project name, version 0.1

add_executable(isr-uart-example
        ${SOURCES_SOURCES}
        ${DRIVERS_SOURCES}
        ${DEVICES_SOURCES}
        ${APP_SOURCES}
)

pico_set_program_name(isr-uart-example "isr-uart-example")
pico_set_program_version(isr-uart-example "0.1")

# Modify the below lines to enable/disable output over UART/USB
# (para agregar logs de debug por USB de nuevo: pico_enable_stdio_usb 1
# y volver a usar printf() en el codigo)
pico_enable_stdio_uart(isr-uart-example 0)
pico_enable_stdio_usb(isr-uart-example 0)

# Add the standard library to the build
target_link_libraries(isr-uart-example
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(isr-uart-example PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/sources
        ${CMAKE_CURRENT_LIST_DIR}/drivers
        ${CMAKE_CURRENT_LIST_DIR}/devices
        ${CMAKE_CURRENT_LIST_DIR}/app
)

pico_add_extra_outputs(isr-uart-example)

