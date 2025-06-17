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

void midi_in_cb(uint32_t msg) {
    /* Just send the MIDI message to core1... */
    send_MIDI(msg);
}

synth_param selected_param = Timbre;
uint8_t param_value[PARAM_COUNT] = {6, 0, 6, 0, 0, 0, 6, 10};

void incr_param(synth_param id) {
    if (param_value[id] < MAX_MIDI_VAL) {
        param_value[id] += 1;
        send_CC(0, id, param_value[id]);
    }
}

void decr_param(synth_param id) {
    if (param_value[id] > 0) {
        param_value[id] -= 1;
        send_CC(0, id, param_value[id]);
    }
}

#define DEFAULT_BASE_NOTE 48 // C3
int base_note = DEFAULT_BASE_NOTE;

int main(void) {
    stdio_init_all();

    multicore_fifo_drain();
    sleep_ms(200);
    multicore_reset_core1();
    sleep_ms(200);
    multicore_launch_core1(braids_main);

    keyboard_init();
    leds_init();
    screen_init();
    if (!nn_audio_init(44100, audio_out_cb, NULL)) {
        printf("PGB-1 audio init failed");
    }
    if (!nn_set_hp_volume(1.0, 1.0)) {
        printf("PGB-1 HP volume failed");
    }
    // if (!nn_enable_speakers(true, false, 3)) {
    //     printf("PGB-1 speaker init failed");
    // }
    // if (!nn_set_line_out_volume(1.0, 1.0, 0.0, 0.0)) {
    //     printf("PGB-1 line out volume failed");
    // }

    /* Set synth parameters */
    for (int i = 0; i < PARAM_COUNT; i++) {
        send_CC(0, i, param_value[i]);
    }

    /* Send buffers to core1 */
    for (int i = 0; i < AUDIO_BUFFER_CNT; i++) {
        send_buffer_id(i);
    }

    midi_init(midi_in_cb);

    leds_clear();
    leds_set_color(0, Spring_Green); // Shape
    leds_set_color(1, Spring_Green); // Shape

    leds_set_color(4, Magenta);  // Timbre
    leds_set_color(14, Magenta); // Timbre

    leds_set_color(13, Yellow); // Color
    leds_set_color(23, Yellow); // Color

    leds_set_color(5, Azure); // Octave down
    leds_set_color(12, Cyan); // Octave UP
    leds_set_color(8, Cyan);  // Octave reset

    // Keyboard
    leds_set_color(6, White);
    leds_set_color(7, White);
    leds_set_color(9, White);
    leds_set_color(10, White);
    leds_set_color(11, White);

    leds_set_color(15, White);
    leds_set_color(16, White);
    leds_set_color(17, White);
    leds_set_color(18, White);
    leds_set_color(19, White);
    leds_set_color(20, White);
    leds_set_color(21, White);
    leds_set_color(22, White);

    leds_update();
    for (uint32_t frame = 0; ; frame++) {
        keyboard_scan();

        // D-PAD to select current param
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

        // A and B to change selected param value
        if (falling(K_A)) {
            incr_param(selected_param);
        } else if (falling(K_B)) {
            decr_param(selected_param);
        }

        // Shortcuts for Shape
        if (falling(K_SONG)) {
            incr_param(Shape);
        } else if (falling(K_MENU)) {
            decr_param(Shape);
        }

        if (frame % 100 == 0) {
            // Shortcuts for Timbre
            if (pressed(K_TRACK)) {
                incr_param(Timbre);
            } else if (pressed(K_STEP)) {
                decr_param(Timbre);
            }

            // Shortcuts for Color
            if (pressed(K_PLAY)) {
                incr_param(Color);
             } else if (pressed(K_REC)) {
                 decr_param(Color);
             }
        }


        // Octave Up/Down
        if (falling(K_1)) {
            if (base_note > 12) {
                base_note -= 12;
            }
        } else if (falling(K_8)) {
            if (base_note < 108) {
                base_note += 12;
            }
        } else if (falling(K_4)) {
            base_note = DEFAULT_BASE_NOTE;
        }

        // Keyboard
        if (falling(K_9)) {
            send_note_on (0, base_note + 0, 127);
        }
        if (falling(K_2)) {
            send_note_on (0, base_note + 1, 127);
        }
        if (falling(K_10)) {
            send_note_on (0, base_note + 2, 127);
        }
        if (falling(K_3)) {
            send_note_on (0, base_note + 3, 127);
        }
        if (falling(K_11)) {
            send_note_on (0, base_note + 4, 127);
        }
        if (falling(K_12)) {
            send_note_on (0, base_note + 5, 127);
        }
        if (falling(K_5)) {
            send_note_on (0, base_note + 6, 127);
        }
        if (falling(K_13)) {
            send_note_on (0, base_note + 7, 127);
        }
        if (falling(K_6)) {
            send_note_on (0, base_note + 8, 127);
        }
        if (falling(K_14)) {
            send_note_on (0, base_note + 9, 127);
        }
        if (falling(K_7)) {
            send_note_on (0, base_note + 10, 127);
        }
        if (falling(K_15)) {
            send_note_on (0, base_note + 11, 127);
        }
        if (falling(K_16)) {
            send_note_on (0, base_note + 12, 127);
        }

        /* Update the screen every 300 frames */
        if (frame % 300 == 0) {
            screen_clear();
            screen_print(0, 57, value_name[base_note / 12]);

            switch (selected_param) {
            case Shape:
                screen_print(0, 0, shape_name[param_value[selected_param]]);
                break;
            default:
                screen_print(0, 0, param_name[selected_param]);
                screen_print(70, 0, value_name[param_value[selected_param]]);
                break;
            }

            for (int i = 0; i < PARAM_COUNT; i++) {
                const int x0 = 19 + (i % 4) * 27;
                const int y0 = -2 + (i < 4 ? 1 : 2) * 27;
                const int y1 = y0 - param_value[i];

                for (int w = 0; w < 8; w++) {
                    screen_draw_line (x0 + w, y0, x0 + w, y1, true);
                }

                screen_print(x0 - 1, y0 + 4, param_name_short[i]);

                if (selected_param == i) {
                    const int left  = x0 - 2;
                    const int right = x0 + 9;
                    const int bot   = y0 + 1;
                    const int top   = y0 - 16;
                    screen_draw_line (left, bot, right, bot, true);
                    screen_draw_line (left, top, right, top, true);
                    screen_draw_line (left, bot, left, top, true);
                    screen_draw_line (right, bot, right, top, true);
                }
            }

            screen_update();
        }

        sleep_ms(1);
    }
}
