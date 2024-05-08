/*
 * Copyright (c) 2024 Fabien Chouteau @ Wee Noise Makers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file midi_utils.h
 * @brief MIDI messages decoder.
 */

#pragma once
#include <stdbool.h>
#include <unistd.h>

typedef enum midi_cmd {
    Note_Off             = 0x8,
    Note_On              = 0x9, 
    Aftertouch           = 0xA, 
    Continous_Controller = 0xB, 
    Patch_Change         = 0xC, 
    Channel_Pressure     = 0xD, 
    Pitch_Bend           = 0xE, 
    Sys                  = 0xF, 
} midi_cmd;

typedef enum midi_sys_cmd {
    Exclusive      = 0x0,
    Song_Position  = 0x2,
    Song_Select    = 0x3,
    Bus_Select     = 0x5,
    Tune_Request   = 0x6,
    End_Exclusive  = 0x7,
    Timming_Tick   = 0x8,
    Start_Song     = 0xA,
    Continue_Song  = 0xB,
    Stop_Song      = 0xC,
    Active_Sensing = 0xE,
    Reset          = 0xF
} midi_sys_cmd;

typedef struct midi_decoder {
    bool expect_data;
    int count;
    uint32_t msg;
    int index;
} midi_decoder;

/*! \brief Init/reset a MIDI decoder
 *
 * \param dec MIDI decoder instance
 */
void midi_decoder_init(midi_decoder *dec);

/*! \brief Process and incoming byte of MIDI input
 *
 * \param dec MIDI decoder instance
 * \param byte incoming byte of MIDI input
 * \return decoded midi message or 0 for no message
 */
uint32_t midi_decoder_push(midi_decoder *dec, uint8_t byte);
