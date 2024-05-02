noise_nugget_executable(
    basics

    braids/analog_oscillator.cc
    braids/braids_main.cc
    braids/quantizer.cc
    braids/resources.cc
    braids/digital_oscillator.cc
    braids/macro_oscillator.cc
    braids/random.cc
    braids/settings.cc

    basics.c
)

target_link_libraries(basics pico_multicore)

pico_add_extra_outputs(basics)

pico_enable_stdio_usb(basics 1)
pico_enable_stdio_uart(basics 0)
