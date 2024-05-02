#include <stdio.h>
#include <stdlib.h>
#include "pico/multicore.h"
#include "pico/stdlib.h"

#include "noise_nugget.h"
#include "pgb1.h"
#include "braids/braids_main.h"

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

void send_CC(uint8_t chan, uint8_t controller, uint8_t val) {
    const uint32_t kind = 0b1011;
    const uint32_t msg = (chan << 0) | (kind << 4) | ((controller & 0xFF) << 8) |
        ((val & 0xFF) << 16);

    send_MIDI(msg);
}

typedef enum synth_param {
    Shape = 0,
    Timbre,
    AD_Timbre,
    Color,
    AD_Color,
    Attack,
    Decay,
    Volume,
    PARAM_COUNT
} synth_param;

uint8_t param_value[PARAM_COUNT] = {0, 6, 0, 6, 0, 0, 6, 10};

const char *param_name[PARAM_COUNT] =
{
    "Shape",
    "Timbre",
    "Timbre Env",
    "Color",
    "Color Env",
    "Attack",
    "Decay",
    "Volume"
};

const char *shape_name[15] =
{
    "Saw Swarm",
    "Saw Comb",
    "Triple Saw",
    "Triple Square",
    "Triple Triangle",
    "Triple Sine",
    "Filter BP",
    "Vosim",
    "Feedback FM",
    "Plucked",
    "Bowed",
    "Blown",
    "Paraphonic",
    "Twin Peaks",
    "Kick",
};

const char *value_name[15] =
{
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
};

synth_param selected_param = Shape;

int main(void) {
    stdio_init_all();
    printf("PGB-1 basic example\n");

    multicore_fifo_drain();
    sleep_ms(200);
    multicore_reset_core1();
    sleep_ms(200);
    multicore_launch_core1(braids_main);

    keyboard_init();
    leds_init();
    screen_init();
    if (!audio_init(44100, audio_out_cb, NULL)) {
        printf("PGB-1 audio init failed!");
    }

    /* Set synth parameters */
    for (int i = 0; i < PARAM_COUNT; i++) {
        send_CC(0, i, param_value[i]);
    }

    /* Send buffers to core1 */
    for (int i = 0; i < AUDIO_BUFFER_CNT; i++) {
        send_buffer_id(i);
    }

    while (1) {
        keyboard_scan();

        leds_clear();
        screen_clear();

        if (falling (K_LEFT)) {
            if (selected_param > 0) {
                selected_param -= 1;
            }
        } else if (raising (K_RIGHT)) {
            if (selected_param < PARAM_COUNT - 1) {
                selected_param += 1;
            }
        } else if (falling (K_UP)) {
            if (selected_param > 3) {
                selected_param -= 4;
            }
        } else if (raising (K_DOWN)) {
            if (selected_param <= 3) {
                selected_param += 4;
            }
        }

        if (falling (K_B)) {
            if (param_value[selected_param] > 0) {
                param_value[selected_param] -= 1;
                send_CC(0, selected_param, param_value[selected_param]);
            }
        } else if (raising (K_A)) {
            if (param_value[selected_param] < 15) {
                param_value[selected_param] += 1;
                send_CC(0, selected_param, param_value[selected_param]);
            }
        }

        if (falling (K_9)) {
            send_note_on (0, 60, 127);
        }
        if (falling (K_2)) {
            send_note_on (0, 61, 127);
        }
        if (falling (K_10)) {
            send_note_on (0, 62, 127);
        }
        if (falling (K_3)) {
            send_note_on (0, 63, 127);
        }
        if (falling (K_11)) {
            send_note_on (0, 64, 127);
        }
        if (falling (K_12)) {
            send_note_on (0, 65, 127);
        }
        if (falling (K_5)) {
            send_note_on (0, 66, 127);
        }
        if (falling (K_13)) {
            send_note_on (0, 67, 127);
        }
        if (falling (K_6)) {
            send_note_on (0, 68, 127);
        }
        if (falling (K_14)) {
            send_note_on (0, 69, 127);
        }
        if (falling (K_7)) {
            send_note_on (0, 70, 127);
        }
        if (falling (K_15)) {
            send_note_on (0, 71, 127);
        }
        if (falling (K_16)) {
            send_note_on (0, 72, 127);
        }

        screen_print(4, 4, param_name[selected_param]);
        switch (selected_param) {
        case Shape:
            screen_print(4, 13, shape_name[param_value[selected_param]]);
            break;
        default:
            screen_print(4, 13, value_name[param_value[selected_param]]);
            break;
        }

        for (int i = 0; i < PARAM_COUNT; i++) {
            const int x0 = 19 + (i % 4) * 27;
            const int y0 = 21 + (i < 4 ? 1 : 2) * 20;
            const int y1 = y0 - param_value[i];

            for (int w = 0; w < 8; w++) {
                screen_draw_line (x0 + w, y0, x0 + w, y1, true);
            }

            if (selected_param == i) {
                const int left  = x0 - 2;
                const int right = x0 + 9;
                const int bot   = y0 + 2;
                const int top   = y0 - 17;
                screen_draw_line (left, bot, right, bot, true);
                screen_draw_line (left, top, right, top, true);
                screen_draw_line (left, bot, left, top, true);
                screen_draw_line (right, bot, right, top, true);
            }
        }


        leds_update();
        screen_update();

        sleep_ms(100);
    }
}
