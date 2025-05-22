# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(TAREFA_WEB_SERVER C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(TAREFA_WEB_SERVER TAREFA_WEB_SERVER.c
lib/buzzer.c
lib/led_matrix.c
lib/ssd1306.c)

target_include_directories(TAREFA_WEB_SERVER PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_compile_definitions(TAREFA_WEB_SERVER PRIVATE
    PICO_WIFI_COUNTRY=BR
)

pico_set_program_name(TAREFA_WEB_SERVER "TAREFA_WEB_SERVER")
pico_set_program_version(TAREFA_WEB_SERVER "0.1")

# Enable output over UART and USB
pico_enable_stdio_uart(TAREFA_WEB_SERVER 1)
pico_enable_stdio_usb(TAREFA_WEB_SERVER 1)

# Link libraries (CORRETO)
target_link_libraries(TAREFA_WEB_SERVER
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_pwm
    hardware_pio
    hardware_clocks
    hardware_i2c
)

pico_add_extra_outputs(TAREFA_WEB_SERVER)
