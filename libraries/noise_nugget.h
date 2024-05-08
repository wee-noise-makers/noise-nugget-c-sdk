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
