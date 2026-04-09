noise_nugget_executable(
    midi_music_box

    braids/analog_oscillator.cc
    braids/braids_main.cc
    braids/quantizer.cc
    braids/resources.cc
    braids/digital_oscillator.cc
    braids/macro_oscillator.cc
    braids/random.cc
    braids/settings.cc

    encoder.cpp
    gui.cpp
    sensors.cpp
    sequencer.cpp
    midi.cpp
    screen.cpp
    midi_music_box.c
)

pico_generate_pio_header(midi_music_box ${CMAKE_CURRENT_LIST_DIR}/encoder.pio)

target_link_libraries(midi_music_box pico_multicore fixdsp pico_stdlib)

target_compile_definitions(midi_music_box PUBLIC FIXDSP_SAMPLE_RATE=48000)
target_compile_definitions(midi_music_box PUBLIC SYS_CLK_MHZ=200)

pico_add_extra_outputs(midi_music_box)

pico_enable_stdio_usb(midi_music_box 1)
pico_enable_stdio_uart(midi_music_box 0)
