noise_nugget_executable(
    basics
    basics.c
)

pico_enable_stdio_usb(basics 1)
pico_enable_stdio_uart(basics 0)
