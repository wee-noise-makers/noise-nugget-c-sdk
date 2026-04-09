#include <stdio.h>
#include <stdlib.h>
#include "pico/multicore.h"
#include "pico/stdlib.h"

#include "braids/braids_main.h"
#include "noise_nugget.h"
#include "pgb1.h"
#include "gui.hpp"
#include "sequencer.hpp"

/*
 * FIFO message format for buffers
 *
 * XXXX_XXXX_XXXX_XXXX_XXXX_XXXX_IIII_KKKK
 *                                    ^
 *                                    message kind (out_buffer: 1, in_buffer: 2)
 *                               ^
 *                               Buffer ID
 * ^
 * Not used
 *
 * FIFO message format for MIDI
 *
 * MMMM_MMMM_MMMM_MMMM_MMMM_MMMM_XXXX_KKKK
 *                                    ^
 *                                    message kind (MIDI: 3)
 *                               ^
 *                               Not used
 * ^
 * MIDI Message
 */

uint8_t get_size(uint32_t buffer_size) {
    for (uint32_t i = 0; i < 16; i++) {
        if (buffer_size == (1 << i)) {
            return i;
        }
    }
    return 16;
}

void send_buffer_id(uint8_t id) {
    const uint32_t data = (id & 0xFF) << 4 | 0x2;

    multicore_fifo_push_blocking(data);
}

uint32_t audio_buffer_tmp[AUDIO_BUFFER_CNT][AUDIO_BUFFER_LEN] = {0};
int playing_buffer_id = -1;

void audio_out_cb(uint32_t **buffer, uint32_t *stereo_point_count) {

    if (playing_buffer_id >= 0) {
        /* Send back the buffer we just finished to core1 */
        send_buffer_id(playing_buffer_id);
        playing_buffer_id = -1;
    }

    /* Try to get a buffer from core1 */
    if (multicore_fifo_rvalid()) {
        const uint32_t data       = multicore_fifo_pop_blocking();
        const uint8_t  buffer_id = (data >> 4) & 0xFF;

        playing_buffer_id = buffer_id;

        /* Play buffer from core1 */
        *buffer     = (uint32_t *)audio_buffer_tmp[buffer_id];
        *stereo_point_count = AUDIO_BUFFER_LEN;
    } else {
        /* We don't have a buffer to play... */
        *stereo_point_count = 0;
        *buffer = NULL;
    }
}

void midi_in_cb(uint32_t msg) {
    /* Just send the MIDI message to core1... */
    const uint32_t data = (msg & 0xFFFFFF) << 8 | 0x3;

    multicore_fifo_push_blocking(data);

}

int main(void) {
    stdio_init_all();

    multicore_fifo_drain();
    sleep_ms(200);
    multicore_reset_core1();
    sleep_ms(200);
    multicore_launch_core1(braids_main);

    seq_init();
    gui_init();
    if (!nn_audio_init(44100, audio_out_cb, NULL)) {
        printf("PGB-1 audio init failed");
    }
    if (!nn_set_hp_volume(1.0, 1.0)) {
        printf("PGB-1 HP volume failed");
    }
    nn_set_hp_volume(1.0, 1.0);
    // if (!nn_enable_speakers(true, false, 3)) {
    //     printf("PGB-1 speaker init failed");
    // }
    // if (!nn_set_line_out_volume(1.0, 1.0, 0.0, 0.0)) {
    //     printf("PGB-1 line out volume failed");
    // }

    /* Send buffers to core1 */
    for (int i = 0; i < AUDIO_BUFFER_CNT; i++) {
        send_buffer_id(i);
    }

    midi_init(midi_in_cb);

    for (uint32_t frame = 0; ; frame++) {
        seq_update();

        /* Update the screen every X frames */
        if (frame % 30 == 0) {
            gui_update();
        }
        sleep_ms(1);
    }
}
