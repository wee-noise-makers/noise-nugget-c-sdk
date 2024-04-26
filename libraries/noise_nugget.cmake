add_library(noise_nugget INTERFACE)

pico_generate_pio_header(noise_nugget ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)
# pico_generate_pio_header(picosystem ${CMAKE_CURRENT_LIST_DIR}/screen_double.pio)

target_sources(noise_nugget INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/noise_nugget.c
  ${CMAKE_CURRENT_LIST_DIR}/pgb1.c
)

set(NOISE_NUGGET_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/noise_nugget_memmap.ld)

target_include_directories(noise_nugget INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(noise_nugget INTERFACE pico_stdlib hardware_pio hardware_spi hardware_pwm hardware_dma hardware_irq hardware_adc hardware_interp)

function(noise_nugget_executable NAME SOURCES)
  add_executable(
    ${NAME}
    ${SOURCES}
    ${ARGN}
  )

  # Pull in pico libraries that we need
  target_link_libraries(${NAME} noise_nugget)

  # create map/bin/hex file etc.
  pico_add_extra_outputs(${NAME})

  pico_set_linker_script(${NAME} ${NOISE_NUGGET_LINKER_SCRIPT})

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2 DESTINATION .)
endfunction()
