#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "ws2812.pio.h"

#include "noise_nugget.h"

#define WS2812_PIN_BASE 24
#define NUM_PIXELS 24
#define IS_RGBW true

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void pattern_random(uint len) {
    for (int i = 0; i < len; ++i) {
        put_pixel(rand());
    }
}

int main(void) {
    stdio_init_all();
    printf("PGB-1 basic example\n");

    keyboard_init();

    // todo get free sm
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN_BASE, 800000, IS_RGBW);
    
    while (1) {
        pattern_random(NUM_PIXELS);
        keyboard_scan();

        if (pressed (K_UP)) {
            printf("UP pressed\n");
        } else if (falling (K_DOWN)) {
            printf("DOWN falling\n");
        } else if (raising (K_LEFT)) {
            printf("LEFT raising\n");
        }

        sleep_ms(100);
    }
}
