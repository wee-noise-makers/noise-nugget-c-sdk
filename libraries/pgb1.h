/*
 * Copyright (c) 2024 Fabien Chouteau @ Wee Noise Makers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file pgb1.h
 * @brief API for controlling the PGB-1 pocket audio gadget which includes
 * functions for handling keyboard inputs, LED controls, screen management, and
 * MIDI interface.
 */

#pragma once
#include "pico/stdlib.h"

// Keyboard key definitions
#define K_TRACK   0x10000000 ///< Key code for TRACK button.
#define K_STEP    0x08000000 ///< Key code for STEP button.
#define K_PLAY    0x02000000 ///< Key code for PLAY button.
#define K_REC     0x04000000 ///< Key code for RECORD button.
#define K_ALT     0x00040000 ///< Key code for ALT button.
#define K_PATT    0x00001000 ///< Key code for PATTERN button.
#define K_SONG    0x00000040 ///< Key code for SONG button.
#define K_MENU    0x00000020 ///< Key code for MENU button.
#define K_UP      0x00020000 ///< Key code for UP arrow.
#define K_DOWN    0x00800000 ///< Key code for DOWN arrow.
#define K_RIGHT   0x00000800 ///< Key code for RIGHT arrow.
#define K_LEFT    0x20000000 ///< Key code for LEFT arrow.
#define K_A       0x01000000 ///< Key code for A button.
#define K_B       0x00000001 ///< Key code for B button.
#define K_1       0x00400000 ///< Key code for key 1.
#define K_2       0x00010000 ///< Key code for key 2.
#define K_3       0x00000400 ///< Key code for key 3.
#define K_4       0x00000010 ///< Key code for key 4.
#define K_5       0x00000002 ///< Key code for key 5.
#define K_6       0x00000080 ///< Key code for key 6.
#define K_7       0x00002000 ///< Key code for key 7.
#define K_8       0x00080000 ///< Key code for key 8.
#define K_9       0x00200000 ///< Key code for key 9.
#define K_10      0x00008000 ///< Key code for key 10.
#define K_11      0x00000200 ///< Key code for key 11.
#define K_12      0x00000008 ///< Key code for key 12.
#define K_13      0x00000004 ///< Key code for key 13.
#define K_14      0x00000100 ///< Key code for key 14.
#define K_15      0x00004000 ///< Key code for key 15.
#define K_16      0x00100000 ///< Key code for key 16.


/**
 * @brief Initializes the keyboard interface.
 */
void keyboard_init(void);

/**
 * @brief Scans the keyboard state and updates internal key state tracking.
 */
void keyboard_scan(void);

/**
 * @brief Checks if a key is currently pressed.
 * @param key Key code to check.
 * @return true if the key is pressed, false otherwise.
 */
bool pressed(uint32_t key);

/**
 * @brief Checks if a key has transitioned from not-pressed to pressed state.
 * @param key Key code to check.
 * @return true if the key transition is detected, false otherwise.
 */
bool falling(uint32_t key);


/**
 * @brief Checks if a key has transitioned from pressed to not-pressed state.
 * @param key Key code to check.
 * @return true if the key transition is detected, false otherwise.
 */
bool raising(uint32_t key);

#define PGB1_LEDS_COUNT 24 ///< Number of LEDs on the device.

/**
 * @brief Initializes the LED subsystem.
 */
void leds_init(void);

/**
 * @brief Clears all LEDs to off state.
 */
void leds_clear(void);

/**
 * @brief Updates the LED states to the device.
 * @return true if the update was successful, false otherwise.
 */
bool leds_update(void);

/**
 * @struct LedColor
 * @brief Represents an RGB color value.
 */
typedef struct LedColor {
    uint8_t r; ///< Red component.
    uint8_t g; ///< Green component.
    uint8_t b; ///< Blue component.
} LedColor;

#define White        (LedColor){255 / 8, 255 / 8, 255 / 8}
#define Red          (LedColor){255 / 8, 000 / 8, 000 / 8}
#define Rose         (LedColor){255 / 8, 000 / 8, 128 / 8}
#define Magenta      (LedColor){255 / 8, 000 / 8, 255 / 8}
#define Violet       (LedColor){128 / 8, 000 / 8, 255 / 8}
#define Blue         (LedColor){000 / 8, 000 / 8, 255 / 8}
#define Azure        (LedColor){000 / 8, 128 / 8, 255 / 8}
#define Cyan         (LedColor){000 / 8, 255 / 8, 255 / 8}
#define Spring_Green (LedColor){000 / 8, 255 / 8, 128 / 8}
#define Green        (LedColor){000 / 8, 255 / 8, 000 / 8}
#define Chartreuse   (LedColor){128 / 8, 255 / 8, 000 / 8}
#define Yellow       (LedColor){255 / 8, 255 / 8, 000 / 8}
#define Orange       (LedColor){255 / 8, 128 / 8, 000 / 8}

/**
 * @brief Sets an individual LED internal state to a specific RGB color. The new
 * state will be visible after a call to leds_update().
 * @param id The LED index to set, from 0 to PGB1_LEDS_COUNT - 1.
 * @param r Red intensity.
 * @param g Green intensity.
 * @param b Blue intensity.
 */
void leds_set_rgb(int id, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Sets an individual LED internal state to a predefined color. The new
 * state will be visible after a call to leds_update().
 * @param id The LED index to set, from 0 to PGB1_LEDS_COUNT - 1.
 * @param rgb Color to set the LED to.
 */
void leds_set_color(int id, LedColor rgb);

/**
 * @brief Initializes the OLED screen.
 */
void screen_init(void);

/**
 * @brief Clears the OLED screen to a blank state.
 */
void screen_clear(void);

/**
 * @brief Updates the screen with the current graphics buffer.
 * @return true if the update is successful, false otherwise.
 */
bool screen_update(void);

/**
 * @brief Sets or clears a pixel on the screen.
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param set true to set the pixel (turn it on), false to clear it (turn it
 *        off).
 */
void screen_set_pixel(int x, int y, bool set);

/**
 * @brief Draws a line between two points on the screen.
 * @param x0 The x-coordinate of the start point.
 * @param y0 The y-coordinate of the start point.
 * @param x1 The x-coordinate of the end point.
 * @param y1 The y-coordinate of the end point.
 * @param set true to draw the line (set pixels), false to clear the line (clear
 *        pixels).
 */
void screen_draw_line(int x0, int y0, int x1, int y1, bool set);

/**
 * @brief Prints a single character at a specified position on the screen.
 * @param x The x-coordinate for the character.
 * @param y The y-coordinate for the character.
 * @param c The character to print.
 */
void screen_printc(int x, int y, char c);

/**
 * @brief Prints a string at a specified position on the screen.
 * @param x The x-coordinate for the start of the string.
 * @param y The y-coordinate for the start of the string.
 * @param str Pointer to the null-terminated string to be printed.
 */
void screen_print(int x, int y, const char *str);

/**
 * @typedef midi_in_cb_t
 * @brief Type definition for MIDI input callback functions.
 *
 * @param msg The MIDI message received.
 */
typedef void (*midi_in_cb_t)(uint32_t msg);

/**
 * @brief Initializes the MIDI interface with a callback for incoming messages.
 * @param cb Function pointer to the callback that handles incoming MIDI
 *         messages.
 */
void midi_init(midi_in_cb_t cb);

