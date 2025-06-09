#include <stdbool.h>
#include <stddef.h>
#include "noise_nugget.h"
#include "nugget_midi_synth.h"
#include "pico/multicore.h"

static render_audio_callback g_render_cb = NULL;
static note_on_callback g_note_on_cb = NULL;
static note_off_callback g_note_off_cb = NULL;
static cc_callback g_cc_cb = NULL;

#define AUDIO_BUFFER_CNT 5
#define AUDIO_BUFFER_LEN 64

uint32_t audio_buffer_tmp[AUDIO_BUFFER_CNT][AUDIO_BUFFER_LEN] = {0};
int playing_buffer_id = -1;

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

void send_in_buffer_id(uint8_t id) {
    const uint32_t data = (id & 0xFF) << 4 | 0x2;

    multicore_fifo_push_blocking(data);
}

void send_out_buffer_id(uint8_t id) {
    const uint32_t data = (id & 0xFF) << 4 | 0x1;

    multicore_fifo_push_blocking(data);
}

void audio_out_cb(uint32_t **buffer, uint32_t *stereo_point_count) {

    if (playing_buffer_id >= 0) {
        /* Send back the buffer we just finished to core1 */
        send_in_buffer_id(playing_buffer_id);
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

void core1_main (void) {
    multicore_fifo_drain();

    while (1) {

        const uint32_t data = multicore_fifo_pop_blocking();
        const uint8_t  kind = (uint8_t)(data & 0b1111);

        switch (kind) {
        case 1:{ // Out_Buffer
            break;
        }
        case 2: { // In_Buffer

            const uint32_t  buffer_id= (data >> 4) & 0xFF;
            const int       buffer_len = AUDIO_BUFFER_LEN;
            uint32_t *ubuffer          = audio_buffer_tmp[buffer_id];

            if (g_render_cb) {
                g_render_cb (ubuffer, buffer_len);
            }

            // Send back the buffer
            send_out_buffer_id(buffer_id);
            break;
        }
        case 3: {// MIDI Msg
            const uint32_t midi = (data >> 8)  & 0xFFFFFF;
            const uint8_t  chan = (midi >> 0) & 0xF;
            const uint8_t  kind = (midi >> 4) & 0xF;
            const uint8_t  key  = (midi >> 8)  & 0xFF;
            const uint8_t  val  = (midi >> 16)  & 0xFF;

            switch (kind) {
            case 0b1000:{// Note off
                if (g_note_off_cb){
                    g_note_off_cb(chan, key, val);
                }
                break;
            }
            case 0b1001:{// Note on
                if (g_note_on_cb) {
                    g_note_on_cb(chan, key, val);
                }
                break;

            }
            case 0b1011:{// Control change
                if (g_cc_cb) {
                    g_cc_cb(chan, key, val);
                }
            }
            default:{
                break;
            }
            }

            break;
        }
        default: {
            continue;
        }
        }
    }
}

bool start_MIDI_Synth (uint32_t sample_rate,
                       render_audio_callback render_cb,
                       note_on_callback note_on_cb,
                       note_off_callback note_off_cb,
                       cc_callback cc_cb)
{
    g_render_cb = render_cb;
    g_note_on_cb = note_on_cb;
    g_note_off_cb = note_off_cb;
    g_cc_cb = cc_cb;

    multicore_fifo_drain();
    sleep_ms(200);
    multicore_reset_core1();
    sleep_ms(200);
    multicore_launch_core1(core1_main);

    if (!audio_init(sample_rate, audio_out_cb, NULL)) {
        return false;
    }

    sleep_ms(200);

    /* Send buffers to core1 */
    for (int i = 0; i < AUDIO_BUFFER_CNT; i++) {
        send_in_buffer_id(i);
    }

    return true;
}

void send_MIDI(uint32_t msg) {
    const uint32_t data = (msg & 0xFFFFFF) << 8 | 0x3;

    multicore_fifo_push_blocking(data);
}

void send_note_on(uint8_t chan, uint8_t key, uint8_t vel) {
    const uint32_t kind = 0b1001;
    const uint32_t msg = (chan << 0) | (kind << 4) | ((key & 0xFF) << 8) |
        ((vel & 0xFF) << 16);

    send_MIDI(msg);
}

void send_note_off(uint8_t chan, uint8_t key, uint8_t vel) {
    const uint32_t kind = 0b1000;
    const uint32_t msg = (chan << 0) | (kind << 4) | ((key & 0xFF) << 8) |
        ((vel & 0xFF) << 16);

    send_MIDI(msg);
}

void send_CC(uint8_t chan, uint8_t controller, uint8_t val) {
    const uint32_t kind = 0b1011;
    const uint32_t msg = (chan << 0) | (kind << 4) | ((controller & 0xFF) << 8) |
        ((val & 0xFF) << 16);

    send_MIDI(msg);
}
