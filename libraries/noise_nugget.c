#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define KEY_COLUMN_CNT 5
#define KEY_ROW_CNT 6

const int key_column[KEY_COLUMN_CNT] = {18, 19, 26, 23, 29};
const int key_row[KEY_ROW_CNT] = {20, 21, 22, 24, 25, 27};

static uint32_t _keyboard_state = 0;
static uint32_t _keyboard_prev_state = 0;

void keyboard_init(void) {
    for (int i = 0; i < KEY_COLUMN_CNT; i++) {
        const int pin = key_column[i];

        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, false);
    }

    for (int i = 0; i < KEY_ROW_CNT; i++) {
        const int pin = key_row[i];

        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
    }
}

void keyboard_scan(void) {
    uint32_t new_state = 0;
    for (int i = 0; i < KEY_COLUMN_CNT; i++) {
        const int col = key_column[i];

        gpio_put(col, true);

        busy_wait_at_least_cycles(100);

        for (int j = 0; j < KEY_ROW_CNT; j++) {
            const int row = key_row[j];
            new_state <<= 1;
            if (gpio_get (row)) {
                new_state |= 1;
            }
        }

        gpio_put(col, false);
    }

    _keyboard_prev_state = _keyboard_state;
    _keyboard_state = new_state;
}

bool pressed(uint32_t key) {
    return (_keyboard_state & key) != 0;
}

bool falling(uint32_t key){
    uint32_t all_falling_keys = _keyboard_state & (~_keyboard_prev_state);
    return (all_falling_keys & key) != 0;
}

bool raising(uint32_t key){
    uint32_t all_raising_keys = (~_keyboard_state) & _keyboard_prev_state;
    return (all_raising_keys & key) != 0;
}
