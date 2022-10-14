cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(my_project)
pico_sdk_init()

add_executable(clock
    main.cpp epaper.h epaper.cpp
)

pico_enable_stdio_usb(clock 1)
pico_enable_stdio_uart(clock 0)

target_link_libraries(clock pico_stdlib hardware_spi hardware_rtc)
pico_add_extra_outputs(clock)