#pragma once

#include "pico/stdlib.h"

#define K_TRACK 0x10000000
#define K_STEP  0x08000000
#define K_PLAY  0x02000000
#define K_REC   0x04000000
#define K_ALT   0x00040000
#define K_PATT  0x00001000
#define K_SONG  0x00000040
#define K_MENU  0x00000020
#define K_UP    0x00020000
#define K_DOWN  0x00800000
#define K_RIGHT 0x00000800
#define K_LEFT  0x20000000
#define K_A     0x01000000
#define K_B     0x00000001
#define K_1     0x00400000
#define K_2     0x00010000
#define K_3     0x00000400
#define K_4     0x00000010
#define K_5     0x00000002
#define K_6     0x00000080
#define K_7     0x00002000
#define K_8     0x00080000
#define K_9     0x00200000
#define K_10    0x00008000
#define K_11    0x00000200
#define K_12    0x00000008
#define K_13    0x00000004
#define K_14    0x00000100
#define K_15    0x00004000
#define K_16    0x00100000

void keyboard_init(void);
void keyboard_scan(void);

bool pressed(uint32_t key);
bool falling(uint32_t key);
bool raising(uint32_t key);

#define NUM_LEDS 24
void leds_init(void);
void leds_clear(void);
bool leds_update(void);
void leds_set(int id, uint8_t r, uint8_t g, uint8_t b);

void screen_init(void);
void screen_clear(void);
bool screen_update(void);

void screen_set_pixel (int x, int y, bool set);
void screen_draw_line(int x0, int y0, int x1, int y1, bool set);
void screen_printc(int x, int y, char c);
void screen_print(int x, int y, const char *str);
