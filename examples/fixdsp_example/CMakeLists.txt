noise_nugget_executable(
    fixed_dsp_example

    main.cc
)

target_link_libraries(fixed_dsp_example
                      pico_multicore
                      hardware_adc
                      noise_nugget
                      nugget_midi_synth
                      fixdsp)

pico_add_extra_outputs(fixed_dsp_example)

pico_enable_stdio_usb(fixed_dsp_example 1)
pico_enable_stdio_uart(fixed_dsp_example 0)
