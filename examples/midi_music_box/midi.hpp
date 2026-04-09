#pragma once

#include <stdint.h>

void sendSynth(uint32_t msg);
void sendExternal(uint32_t msg);

inline uint32_t makeNoteOn(uint8_t chan, uint8_t key, uint8_t vel) {
    const uint32_t kind = 0b1001;
    const uint32_t msg = (chan << 0) | (kind << 4) | ((key & 0xFF) << 8) |
        ((vel & 0xFF) << 16);
    return msg;
}

inline uint32_t makeNoteOff(uint8_t chan, uint8_t key, uint8_t vel) {
    const uint32_t kind = 0b1000;
    const uint32_t msg = (chan << 0) | (kind << 4) | ((key & 0xFF) << 8) |
        ((vel & 0xFF) << 16);
    return msg;
}

inline uint32_t makeCC(uint8_t chan, uint8_t controller, uint8_t val) {
    const uint32_t kind = 0b1011;
    const uint32_t msg = (chan << 0) | (kind << 4) | ((controller & 0xFF) << 8) |
        ((val & 0xFF) << 16);

    return msg;
}
