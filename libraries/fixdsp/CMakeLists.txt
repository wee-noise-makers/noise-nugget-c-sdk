pico_add_library(fixdsp)

target_sources(fixdsp INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/fixdsp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fixdsp-filter-svf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fixdsp-oscillator-waveform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fixdsp-oscillator-phase_distortion.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fixdsp-resources.cpp
    )

target_include_directories(fixdsp_headers SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
