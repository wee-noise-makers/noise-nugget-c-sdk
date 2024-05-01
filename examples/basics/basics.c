#include <stdio.h>
#include <stdlib.h>

#include "noise_nugget.h"
#include "pgb1.h"

int main(void) {
    stdio_init_all();
    printf("PGB-1 basic example\n");

    keyboard_init();
    leds_init();
    screen_init();
    if (!audio_init(44100)) {
        printf("PGB-1 audio init failed!");
    }

    while (1) {
        keyboard_scan();

        screen_clear();
        screen_draw_line(0, 0, 127, 63, true);
        screen_draw_line(127, 0, 0, 63, true);
        screen_print(10, 10, "Hello PGB-1!");
        screen_update();

        leds_clear();
        if (pressed (K_UP)) {
            printf("UP pressed\n");
            leds_set(0, 255, 0, 0);
            leds_set(3, 255, 0, 0);
            leds_set(4, 255, 0, 0);
            leds_set(5, 255, 0, 0);
            leds_set(6, 255, 0, 0);
            leds_set(7, 255, 0, 0);
            leds_set(23, 255, 0, 0);
        } else if (falling (K_DOWN)) {
            leds_set(1, 0, 255, 0);
            printf("DOWN falling\n");
        } else if (raising (K_LEFT)) {
            leds_set(2, 0, 0, 255);
            printf("LEFT raising\n");
        }


        leds_update();

        sleep_ms(100);
    }
}
