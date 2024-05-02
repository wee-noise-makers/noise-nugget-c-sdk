#pragma once

#include "pico/stdlib.h"

typedef void (*audio_cb_t)(uint32_t **buffer, uint32_t *stereo_point_count);

bool audio_init(int sample_rate,
                audio_cb_t output_callback,
                audio_cb_t input_callback);
