/*
 * Copyright (c) 2024 Fabien Chouteau @ Wee Noise Makers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "midi_utils.h"

void midi_decoder_init(midi_decoder *dec) {
    dec->expect_data = false;
    dec->msg = 0;
    dec->count = 0;
    dec->index = 0;
}

uint32_t midi_decoder_push(midi_decoder *dec, uint8_t byte) {
    const bool is_status = (byte & 0b10000000) != 0;
    
    if (dec->expect_data) {
        if (is_status || dec->count == 0) {
            /* There is an issue with the current message, ignore it */
            dec->expect_data = false;

            /* Fallback to status byte processing below */
        } else {
            /* Save incoming data byte*/
            dec->msg = dec->msg | (byte << (8 * dec->index));
            dec->count -= 1;
            dec->index += 1;

            /* Check if we have a complete message */
            if (dec->count == 0) {
                const uint32_t msg = dec->msg;
                midi_decoder_init(dec);

                return msg;
            }

            return 0;
        }
    }

    if (is_status) {
        const uint8_t cmd = (byte >> 4) & 0b1111;
        const uint8_t sub = (byte >> 0) & 0b1111;

        switch (cmd) {

        case Note_Off:
        case Note_On:
        case Aftertouch:
        case Continous_Controller:
                dec->msg = byte;
                dec->count = 3 - 1;
                dec->index = 1;
                dec->expect_data = true;
            break;

        case Patch_Change:
        case Channel_Pressure:
        case Pitch_Bend:
                dec->msg = byte;
                dec->count = 2 - 1;
                dec->index = 1;
                dec->expect_data = true;
            break;

        case Sys:
            switch (sub) {

            case Exclusive:
            case End_Exclusive:
                /* Ignore SysEx start and stop, we do not support those
                   messages */
                break;
                
            case Song_Position:
                dec->msg = byte;
                dec->count = 3 - 1;
                dec->index = 1;
                dec->expect_data = true;
                break;

            case Song_Select:
            case Bus_Select:
                dec->msg = byte;
                dec->count = 2 - 1;
                dec->index = 1;
                dec->expect_data = true;

            case Tune_Request:
            case Timming_Tick:
            case Start_Song:
            case Continue_Song:
            case Stop_Song:
            case Active_Sensing:
            case Reset:
                dec->msg = 0;
                dec->count = 0;
                dec->index = 0;
                dec->expect_data = false;
                return byte;
                break;

            default:
                /* Unknown/unsupported sys message */
                break;
            }
            break;

        default:
            /* Unknown/unsupported message */
            break;
            
        }
    }
}
