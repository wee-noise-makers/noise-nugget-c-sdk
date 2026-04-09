#include "midi.hpp"

#include "pico/multicore.h"

void sendSynth(uint32_t msg) {
    const uint32_t data = (msg & 0xFFFFFF) << 8 | 0x3;

    multicore_fifo_push_blocking(data);

}

void sendExternal(uint32_t msg) {

}