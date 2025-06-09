pico_add_library(nugget_midi_synth)

target_sources(nugget_midi_synth INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/nugget_midi_synth.c
    )

target_include_directories(nugget_midi_synth_headers SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
