cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(rockband_guitar C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(rockband_guitar
        main.c
        usb_descriptors.c
)

target_include_directories(rockband_guitar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(rockband_guitar
        pico_stdlib
        tinyusb_device
        tinyusb_board
)

# Generate the .uf2 file to flash, plus .bin/.hex/.map
pico_add_extra_outputs(rockband_guitar)

# USB is taken by the guitar, so stdio goes to UART
pico_enable_stdio_usb(rockband_guitar 0)
pico_enable_stdio_uart(rockband_guitar 1)
