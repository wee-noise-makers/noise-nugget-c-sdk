/*
 * Copyright (c) 2024 Fabien Chouteau @ Wee Noise Makers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file nugget_midi_synth.h
 * @brief This small library provides an easy way to run a MIDI synthesizer
 * on the Noise Nugget.
 *
 * Call the nn_ms_start function with the desired sample rate (8000, 16000,
 * 22050, 32000, 44100, or 48000) and at least an audio rendering callback.
 * The audio render callback will be called on CPU core 1 with a pointer to
 * a stereo audio buffer to fill, and its length.
 *
 * Optional MIDI event callbacks can be provided to handle communication
 * from CPU core 0. Use the nn_ms_send_* function to send MIDI messages
 * to these callbacks.
 *
 * You can change the number and size of internal audio buffers by defining
 * the C macros in your cmake file: NN_MS_BUFFER_LEN, and NN_MS_BUFFER_COUNT.
 *
 * target_compile_definitions(my_target_project PUBLIC NN_MS_BUFFER_LEN=128)
 * target_compile_definitions(my_target_project PUBLIC NN_MS_BUFFER_COUNT=3)
 *
 * Increasing the buffer len can improved performance at the expense of more
 * latency. If you are using a fixed buffer length audio DSP libary (like
 * fixdsp), use this MACRO to have the same buffer lengths.
 *
 * In general, there is no need to change the buffer count. In some situation,
 * increasing it can be useful if some rendering pass take more time than
 * others, at the expense of more latency. Don't set it below 2.
 *
 * (nn_ms_ prefix stands for Noise Nugget MIDI Synth)
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#ifndef NN_MS_BUFFER_COUNT
#define NN_MS_BUFFER_COUNT 5
#endif

#ifndef NN_MS_BUFFER_LEN
#define NN_MS_BUFFER_LEN 64
#endif

typedef void (*render_audio_callback)(uint32_t *buffer, int len);
typedef void (*note_on_callback)(uint8_t chan, uint8_t key, uint8_t velocity);
typedef void (*note_off_callback)(uint8_t chan, uint8_t key, uint8_t velocity);
typedef void (*cc_callback)(uint8_t chan, uint8_t contoller, uint8_t value);

bool nn_ms_start (uint32_t sample_rate,
                  render_audio_callback render_cb,
                  note_on_callback note_on_cb,
                  note_off_callback note_off_cb,
                  cc_callback cc_cb);

void nn_ms_send_MIDI(uint32_t msg);
void nn_ms_send_note_on(uint8_t chan, uint8_t key, uint8_t vel);
void nn_ms_send_note_off(uint8_t chan, uint8_t key, uint8_t vel);
void nn_ms_send_CC(uint8_t chan, uint8_t controller, uint8_t val);

#ifdef __cplusplus
}
#endif
