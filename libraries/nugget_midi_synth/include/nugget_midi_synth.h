/*
 * Copyright (c) 2024 Fabien Chouteau @ Wee Noise Makers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file nugget_midi_synth.h
 * @brief 
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef void (*render_audio_callback)(uint32_t *buffer, int len);
typedef void (*note_on_callback)(uint8_t chan, uint8_t key, uint8_t velocity);
typedef void (*note_off_callback)(uint8_t chan, uint8_t key, uint8_t velocity);
typedef void (*cc_callback)(uint8_t chan, uint8_t contoller, uint8_t value);

bool start_MIDI_Synth (uint32_t sample_rate,
                       render_audio_callback render_cb,
                       note_on_callback note_on_cb,
                       note_off_callback note_off_cb,
                       cc_callback cc_cb);

void send_MIDI(uint32_t msg);
void send_note_on(uint8_t chan, uint8_t key, uint8_t vel);
void send_note_off(uint8_t chan, uint8_t key, uint8_t vel);
void send_CC(uint8_t chan, uint8_t controller, uint8_t val);

#ifdef __cplusplus
}
#endif
