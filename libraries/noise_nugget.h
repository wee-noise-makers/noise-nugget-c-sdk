/*
 * Copyright (c) 2024 Fabien Chouteau @ Wee Noise Makers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file noise_nugget.h
 * @brief API for the Noise Nugget development board, which offers digital audio
 * processing and synthesis.
 *
 * This file provides the initialization functions and callback mechanisms
 * needed to interface with the Noise Nugget's audio processing capabilities.
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef audio_cb_t
 * @brief Defines a callback type for audio processing.
 *
 * @details This callback function is used by the Noise Nugget development board
 * to handle audio data processing. The function will be called when audio data
 * needs to be output or input through the board. The callback must return as
 * fast as possible to not interrupt the flow of sample being played/recorded.
 * Therefore any audio synthesis or processing must be done separately, for
 * instance using flip buffers, the callbacks are just here to provide pointers
 * to the next fresh buffers.
 *
 * @param buffer Double pointer to the audio data buffer. Set *buffer with the
 *        address of the next stereo audio samples (uint32_t) to be
 *        played/recorded, or set *buffer to NULL if there is no buffer
 *        available.
 *
 * @param stereo_point_count Pointer to the number of stereo points (left-right
 *                            pairs) available in the buffer. Set
 *                            *stereo_point_count to specified the size of the
 *                            buffer to be played/recorded.
 */
typedef void (*audio_cb_t)(uint32_t **buffer, uint32_t *stereo_point_count);

/**
 * @brief Initializes the audio system of the Noise Nugget board.
 *
 * @details This function initializes the audio system with a specified sample
 * rate and sets up the necessary callback functions for audio data input and
 * output.
 *
 * @param sample_rate The audio sample rate to be configured. Typical values
 *                    might include 44100 or 21500 Hz.
 * @param output_callback The callback function to be invoked when an audio
 *                        output buffer is required.
 * @param input_callback The callback function to be invoked when an audio input
 *                        buffer is required.
 *
 * @return Returns true if the audio system was successfully initialized, false
 *         otherwise.
 */
bool audio_init(int sample_rate,
                audio_cb_t output_callback,
                audio_cb_t input_callback);

/**
 * @brief Enable line level output
 *
 * @param left enable left channel
 * @param right enable right channel
 *
 * @return Returns true on success, false otherwise.
 */
bool enable_line_out(bool left, bool right);

/**
 * @brief Enable speaker amp
 *
 * @param left enable left channel
 * @param right enable right channel
 * @param gain Amplification gain between 0 and 3
 *
 * @return Returns true on success, false otherwise.
 */
bool enable_speakers (bool left, bool right, uint8_t gain);

/**
 * @brief Set volume for line output (and speakers)
 *
 * @details Both left and right DAC channels can be routed to left and/or
 * right line outputs. This can used to mix left and right channels into a
 * single speaker for instance.
 *
 * @param L2L left channel into left output
 * @param L2R left channel into right output
 * @param R2L right channel into left output
 * @param R2R right channel into right output
 *
 * @return Returns true on success, false otherwise.
 */
bool set_line_out_volume(float L2L, float L2R, float R2L, float R2R);

/**
 * @brief Set ADC (Analog to Digital Converter) volume
 *
 * @param left left channel volume
 * @param right right channel volume
 *
 * @return Returns true on success, false otherwise.
 */
bool set_adc_volume(float left, float right);

/**
 * @brief Set Headphone output volume
 *
 * @param left left channel volume
 * @param right right channel volume
 *
 * @return Returns true on success, false otherwise.
 */
bool set_hp_volume(float left, float right);

/**
 * @brief Set line input boost
 *
 * @details Both left and right channels of each lines 1 and 3 can be routed to
 * left and/or right line ADC channels. This can used to produce stereo sound
 * from a single microphone.
 *
 * @param line Line identifier between 1 and 3
 * @param L2L left input into left channel between 0 (mute) and 10
 * @param L2R left input into right channel between 0 (mute) and 10
 * @param R2L right input into left channel between 0 (mute) and 10
 * @param R2R right input into right channel between 0 (mute) and 10
 *
 * @return Returns true on success, false otherwise.
 */
bool set_line_in_boost (uint8_t line, uint8_t L2L, uint8_t L2R, uint8_t R2L, uint8_t R2R);

/**
 * @brief Enable microphone bias
 *
 * @details Mircophone bias is a small voltage providing power to a microphone
 * (typically and electret condenser microphone).
 *
 * @return Returns true on success, false otherwise.
 */
bool enable_mic_bias (void);

#ifdef __cplusplus
}
#endif