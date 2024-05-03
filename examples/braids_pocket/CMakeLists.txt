noise_nugget_executable(
    braids_pocket

    braids/analog_oscillator.cc
    braids/braids_main.cc
    braids/quantizer.cc
    braids/resources.cc
    braids/digital_oscillator.cc
    braids/macro_oscillator.cc
    braids/random.cc
    braids/settings.cc

    braids_pocket.c
)

target_link_libraries(braids_pocket pico_multicore)

pico_add_extra_outputs(braids_pocket)

pico_enable_stdio_usb(braids_pocket 1)
pico_enable_stdio_uart(braids_pocket 0)
