set(PROJECT "hid")
set(BOARD "pico_sdk")
cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
include(tinyusb/hw/bsp/family_support.cmake)

family_get_project_name(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})
project(${PROJECT})

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(main
    main.c
)

target_sources(main PUBLIC
    main.c
    usb_descriptors.c
)

include_directories(
    tinyusb/src
    tinyusb/hw
)

target_include_directories(main PUBLIC
    .
    tinyusb/src
    tinyusb
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main pico_stdlib)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)