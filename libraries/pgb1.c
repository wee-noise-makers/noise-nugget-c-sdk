/*
 * Copyright (c) 2024 Fabien Chouteau @ Wee Noise Makers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "ws2812.pio.h"
#include "pgb1.h"
#include "midi_utils.h"

#define LED_PIO_SM 0
#define LED_PIO pio0
#define IS_RGBW false
#define WS2812_PIN 5

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

int leds_dma_chan = -1; // init with invalid DMA channel id
uint32_t leds_framebuffer[PGB1_LEDS_COUNT] = {0};

void leds_init(void) {
    uint offset = pio_add_program(LED_PIO, &ws2812_program);
    ws2812_program_init(LED_PIO, LED_PIO_SM, offset, WS2812_PIN, 800000, IS_RGBW);

    leds_dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(leds_dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_dreq(&c, pio_get_dreq(LED_PIO, LED_PIO_SM, true));
    dma_channel_set_config(leds_dma_chan, &c, false);
    dma_channel_set_write_addr(leds_dma_chan, &LED_PIO->txf[LED_PIO_SM],
                               false);

}

bool leds_update(void) {
    if (leds_dma_chan < 0 || dma_channel_is_busy(leds_dma_chan)) {
        // Previous DMA transfer still in progress
        return false;
    }

    dma_channel_transfer_from_buffer_now(leds_dma_chan,
                                         leds_framebuffer,
                                         PGB1_LEDS_COUNT);
    return true;

}

void leds_clear(void) {
    memset(leds_framebuffer, 0, sizeof(leds_framebuffer));
}

void leds_set_rgb(int id, uint8_t r, uint8_t g, uint8_t b) {
    if (id >= 0 && id < PGB1_LEDS_COUNT) {
        leds_framebuffer[id] = ((uint32_t) (r) << 16) |
            ((uint32_t) (g) << 24) |
            ((uint32_t) (b) << 8);
    }
}

void leds_set_color(int id, LedColor rgb) {
    leds_set_rgb(id, rgb.r, rgb.g, rgb.b);
}

// register definitions
#define SET_CONTRAST        0x81
#define SET_ENTIRE_ON       0xA4
#define SET_NORM_INV        0xA6
#define SET_DISP            0xAE
#define SET_MEM_ADDR        0x20
#define SET_COL_ADDR        0x21
#define SET_PAGE_ADDR       0x22
#define SET_DISP_START_LINE 0x40
#define SET_SEG_REMAP       0xA0
#define SET_MUX_RATIO       0xA8
#define SET_IREF_SELECT     0xAD
#define SET_COM_OUT_DIR     0xC0
#define SET_DISP_OFFSET     0xD3
#define SET_COM_PIN_CFG     0xDA
#define SET_DISP_CLK_DIV    0xD5
#define SET_PRECHARGE       0xD9
#define SET_VCOM_DESEL      0xDB
#define SET_CHARGE_PUMP     0x8D

#define WIDTH 128
#define HEIGHT 64

#define SCREEN_SPI spi1
#define N_RESET_PIN 13
#define DC_PIN      12
#define SCK_PIN     10
#define MOSI_PIN    11
#define SCREEN_FRAMEBUFFER_SIZE (WIDTH * HEIGHT) / 8

int screen_dma_chan = -1; // init with invalid DMA channel id
uint8_t screen_framebuffer[SCREEN_FRAMEBUFFER_SIZE] = {0};

uint8_t init_cmds[] =
{SET_DISP, // off
 SET_MEM_ADDR, // address setting
 0x00, // Horizontal Addressing Mode
 //  resolution and layout
 SET_DISP_START_LINE,
 SET_SEG_REMAP | 0x01, // column addr 127 mapped to SEG0
 SET_MUX_RATIO,
 HEIGHT- 1,
 SET_COM_OUT_DIR | 0x08, // scan from COM[N] to COM0
 SET_DISP_OFFSET,
 0x00,
 SET_COM_PIN_CFG,
 0x12,
 //  timing and driving scheme
 SET_DISP_CLK_DIV,
 0x80,
 SET_PRECHARGE,
 0xF1,
 SET_VCOM_DESEL,
 0x30, //  0.83*Vcc
 //  n.b. specs for ssd1306 64x32 oled screens imply this should be 0x40
 //  display
 SET_CONTRAST,
 0xFF, // maximum
 SET_ENTIRE_ON, // output follows RAM contents
 SET_NORM_INV, // not inverted
 SET_IREF_SELECT,
 0x30, // enable internal IREF during display on
 //  charge pump
 SET_CHARGE_PUMP,
 0x14,
 SET_DISP | 0x01};  // display on

void screen_write_cmd(uint8_t cmd) {
    gpio_put(DC_PIN, false);
    spi_write_blocking(SCREEN_SPI, &cmd, 1);
}

void screen_init(void) {
    gpio_init(N_RESET_PIN);
    gpio_set_dir(N_RESET_PIN, GPIO_OUT);
    gpio_pull_up(N_RESET_PIN);
    gpio_put(N_RESET_PIN, false);

    gpio_init(MOSI_PIN);
    gpio_set_dir(MOSI_PIN, GPIO_OUT);
    gpio_pull_up(MOSI_PIN);
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);

    gpio_init(SCK_PIN);
    gpio_set_dir(SCK_PIN, GPIO_OUT);
    gpio_pull_up(SCK_PIN);
    gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);

    gpio_init(DC_PIN);
    gpio_set_dir(DC_PIN, GPIO_OUT);
    gpio_pull_up(DC_PIN);
    gpio_put(DC_PIN, false);

    spi_init(SCREEN_SPI, 1000000);

    gpio_put(N_RESET_PIN, true);
    sleep_ms(1);
    gpio_put(N_RESET_PIN, false);
    sleep_ms(10);
    gpio_put(N_RESET_PIN, true);

    screen_write_cmd(SET_DISP | 0x01);

    for (int i = 0; i < sizeof(init_cmds) / sizeof(*init_cmds); i++){
        screen_write_cmd(init_cmds[i]);
    }

    screen_dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(screen_dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(SCREEN_SPI, true));
    dma_channel_set_config(screen_dma_chan, &c, false);
    dma_channel_set_write_addr(screen_dma_chan, &spi_get_hw(SCREEN_SPI)->dr,
                               false);
    screen_clear();
}

void screen_clear(void) {
    memset(screen_framebuffer, 0, sizeof(screen_framebuffer));
}

bool screen_update(void) {
    if (screen_dma_chan < 0 || dma_channel_is_busy(screen_dma_chan)) {
        // Previous DMA transfer still in progress
        return false;
    }

    gpio_put(DC_PIN, true);
    dma_channel_transfer_from_buffer_now(screen_dma_chan,
                                         screen_framebuffer,
                                         SCREEN_FRAMEBUFFER_SIZE);
    return true;
}

void screen_set_pixel (int x, int y, bool set) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        int index = x + (y / 8) * WIDTH;
        uint8_t *byte = &screen_framebuffer[index];
        if (set) {
            *byte |= 1 << (y % 8);
        } else {
            *byte &= ~(1 << (y % 8));
        }
    }
}

void screen_draw_line(int x0, int y0, int x1, int y1, bool set) {
    int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;){  /* loop */
        screen_set_pixel (x0, y0, set);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}

const uint8_t font5x7[] =
{187, 214, 205, 231, 125, 253, 255, 255, 255, 141, 59, 130, 11, 38, 136, 241, 255, 251, 123, 140, 17, 70, 12, 64, 116,
 113, 56, 239, 92, 132, 17, 70, 224, 156, 115, 14, 196, 30, 247, 207, 223, 255, 247, 239, 247, 251, 246, 252, 255, 255,
 255, 255, 255, 255, 255, 255, 207, 157, 255, 174, 53, 176, 118, 239, 222, 251, 255, 127, 93, 118, 119, 250, 254, 156, 203,
 121, 255, 237, 156, 115, 206, 122, 239, 220, 190, 214, 19, 231, 156, 115, 110, 59, 231, 156, 123, 189, 223, 250, 251, 247,
 255, 253, 245, 253, 255, 125, 255, 255, 255, 255, 127, 255, 255, 255, 255, 125, 223, 191, 43, 208, 183, 222, 125, 213, 254,
 255, 111, 179, 223, 174, 208, 95, 231, 114, 238, 224, 190, 231, 156, 119, 222, 123, 183, 175, 246, 138, 57, 231, 156, 223,
 206, 185, 90, 111, 223, 215, 253, 29, 101, 76, 113, 7, 153, 111, 219, 84, 70, 24, 74, 136, 206, 57, 230, 64, 223,
 183, 239, 95, 227, 222, 127, 223, 8, 62, 248, 91, 237, 123, 237, 131, 59, 134, 255, 253, 127, 183, 1, 232, 29, 132,
 2, 236, 203, 189, 82, 14, 58, 24, 183, 115, 106, 239, 221, 247, 253, 255, 63, 230, 141, 139, 142, 237, 91, 183, 98,
 206, 185, 236, 183, 115, 110, 182, 235, 247, 83, 251, 131, 213, 171, 223, 87, 237, 252, 255, 102, 123, 63, 240, 220, 118,
 47, 231, 14, 238, 86, 206, 121, 231, 189, 115, 251, 106, 239, 140, 243, 74, 191, 237, 156, 170, 187, 247, 125, 255, 255,
 193, 121, 7, 118, 112, 251, 230, 173, 156, 131, 161, 199, 237, 156, 218, 115, 247, 125, 239, 255, 21, 58, 246, 239, 222,
 123, 255, 220, 221, 238, 238, 58, 183, 221, 205, 123, 255, 253, 149, 115, 206, 122, 239, 220, 182, 214, 59, 231, 188, 181,
 110, 187, 74, 220, 246, 253, 222, 255, 191, 115, 206, 249, 253, 220, 182, 117, 43, 231, 252, 235, 111, 153, 170, 242, 238,
 125, 223, 191, 127, 221, 79, 250, 215, 255, 239, 63, 247, 24, 65, 188, 49, 238, 152, 127, 191, 191, 239, 232, 96, 196,
 192, 7, 23, 179, 3, 206, 69, 159, 92, 220, 113, 59, 183, 65, 188, 241, 131, 31, 98, 12, 113, 71, 23, 115, 139,
 202, 69, 255, 58, 188, 105, 87, 195, 193, 220, 249, 251};

uint8_t get_bit(const uint8_t *bitmap, int bit_index) {
    int byte_index = bit_index / 8;
    int bit_position = bit_index % 8;

    return (bitmap[byte_index] >> bit_position) & 0x01;
}

void screen_printc(int x, int y, char c) {
    const int index = c - '!';

    if (index < 0 || index > 93) {
        return;
    }

    for (int dy = 0; dy < 7; dy++) {
        for (int dx = 0; dx < 5; dx++) {
            const int bit_index = index * 5 + dx + dy * 470;
            screen_set_pixel(x + dx, y + dy, get_bit(font5x7, bit_index) == 0);
        }
    }
}

void screen_print(int x, int y, const char *str) {
    int dx = 0;
    for (const char *c = str; *c != 0; c++, dx += 6) {
        screen_printc(x + dx, y, *c);
    }
}

#define MIDI_UART uart1
#define MIDI_IRQ UART1_IRQ
#define MIDI_OUT_PIN 8
#define MIDI_IN_PIN 9
#define MIDI_GPIO_FUNC GPIO_FUNC_UART
static midi_in_cb_t midi_in_user_cb = NULL;

static midi_decoder pgb1_midi_decoder;

void on_uart_rx() {
    while (uart_is_readable(MIDI_UART)) {
        uint8_t ch = uart_getc(MIDI_UART);
        uint32_t msg = midi_decoder_push(&pgb1_midi_decoder, ch);
        if (msg != 0 && midi_in_user_cb != NULL) {
            midi_in_user_cb(msg);
        }
    }
}

void midi_init(midi_in_cb_t cb) {

    uart_init(MIDI_UART, 31250);
    gpio_set_function(MIDI_IN_PIN, MIDI_GPIO_FUNC);
    gpio_set_function(MIDI_OUT_PIN, MIDI_GPIO_FUNC);
    uart_set_format(MIDI_UART, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(MIDI_UART, true);

    irq_set_exclusive_handler(MIDI_IRQ, on_uart_rx);
    irq_set_enabled(MIDI_IRQ, true);
    uart_set_irq_enables(MIDI_UART, true, false);

    midi_decoder_init(&pgb1_midi_decoder);
    midi_in_user_cb = cb;
}
