cmake_minimum_required(VERSION 3.13)

# Include the Pico SDK
include(pico_sdk_import.cmake)

project(hid C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

# Create the executable
add_executable(hid
        main.c
        usb_descriptors.c
)

# Include directories
target_include_directories(hid PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against Pico SDK libraries
target_link_libraries(hid
        pico_stdlib
        tinyusb_device
        tinyusb_board
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(hid)

# Enable USB output, disable UART output
pico_enable_stdio_usb(hid 0)
pico_enable_stdio_uart(hid 1)