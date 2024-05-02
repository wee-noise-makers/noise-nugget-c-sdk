// Copyright 2012 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION ObF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.

//#include <stm32f10x_conf.h>

#include <algorithm>

#include "dsp.h"
#include "envelope.h"
#include "macro_oscillator.h"
#include "signature_waveshaper.h"
#include "vco_jitter_source.h"

// #include "plugin_interface.h"

#include "pico/multicore.h"
#include "braids_main.h"

// #define PROFILE_RENDER 1

using namespace braids;
using namespace std;
using namespace stmlib;

#define MAX_MIDI_VAL (15)
#define MAX_PARAM (32767)

#define POLY_OSCs (NBR_OF_OSCs - 0) // Number of oscillators for the poly chan

const size_t kBlockSize = MAX_RENDER_BUFFER_SIZE;

MacroOscillator osc[NBR_OF_OSCs];
Envelope envelope[NBR_OF_OSCs];
SignatureWaveshaper ws[NBR_OF_OSCs];
int32_t midi_pitch[NBR_OF_OSCs] = {48 << 7};

uint16_t volume[NBR_OF_CHANs] = {MAX_PARAM};
int32_t cc_params[NBR_OF_CHANs][2] = {{MAX_PARAM / 2}};

const MacroOscillatorShape shape_from_param[MAX_MIDI_VAL + 1] =
{
    // MACRO_OSC_SHAPE_CSAW,
    // MACRO_OSC_SHAPE_MORPH,// +
    // MACRO_OSC_SHAPE_SAW_SQUARE,
    // MACRO_OSC_SHAPE_SINE_TRIANGLE, // +
    // MACRO_OSC_SHAPE_BUZZ, // +

    // MACRO_OSC_SHAPE_SQUARE_SUB,// +
    // MACRO_OSC_SHAPE_SAW_SUB, // +
    // MACRO_OSC_SHAPE_SQUARE_SYNC,
    // MACRO_OSC_SHAPE_SAW_SYNC,
    MACRO_OSC_SHAPE_SAW_SWARM, // ++
    MACRO_OSC_SHAPE_SAW_COMB, // ++
    MACRO_OSC_SHAPE_TRIPLE_SAW, // ++
    MACRO_OSC_SHAPE_TRIPLE_SQUARE,// ++
    MACRO_OSC_SHAPE_TRIPLE_TRIANGLE,// ++
    MACRO_OSC_SHAPE_TRIPLE_SINE,// ++
    // MACRO_OSC_SHAPE_TRIPLE_RING_MOD,
    // MACRO_OSC_SHAPE_TOY,

    // MACRO_OSC_SHAPE_DIGITAL_FILTER_LP,
    // MACRO_OSC_SHAPE_DIGITAL_FILTER_PK,
    MACRO_OSC_SHAPE_DIGITAL_FILTER_BP, // +
    // MACRO_OSC_SHAPE_DIGITAL_FILTER_HP,
    MACRO_OSC_SHAPE_VOSIM, // +
    // MACRO_OSC_SHAPE_VOWEL,
    // MACRO_OSC_SHAPE_VOWEL_FOF,

    // MACRO_OSC_SHAPE_HARMONICS,

    // MACRO_OSC_SHAPE_FM,// +
    MACRO_OSC_SHAPE_FEEDBACK_FM, // ++
    // MACRO_OSC_SHAPE_CHAOTIC_FEEDBACK_FM,

    MACRO_OSC_SHAPE_PLUCKED, // ++
    MACRO_OSC_SHAPE_BOWED,   // ++
    MACRO_OSC_SHAPE_BLOWN,   // ++
    // MACRO_OSC_SHAPE_FLUTED,


    // MACRO_OSC_SHAPE_WAVETABLES, // +
    // MACRO_OSC_SHAPE_WAVE_MAP,
    // MACRO_OSC_SHAPE_WAVE_LINE,
    MACRO_OSC_SHAPE_WAVE_PARAPHONIC, // ++

    // MACRO_OSC_SHAPE_FILTERED_NOISE,
    MACRO_OSC_SHAPE_TWIN_PEAKS_NOISE, // ++
    // MACRO_OSC_SHAPE_CLOCKED_NOISE,
    // MACRO_OSC_SHAPE_GRANULAR_CLOUD,
    // MACRO_OSC_SHAPE_PARTICLE_NOISE,

    // MACRO_OSC_SHAPE_DIGITAL_MODULATION

    // MACRO_OSC_SHAPE_STRUCK_BELL,
    // MACRO_OSC_SHAPE_STRUCK_DRUM,
    MACRO_OSC_SHAPE_KICK, // +
    // MACRO_OSC_SHAPE_CYMBAL,
    MACRO_OSC_SHAPE_SNARE, // +
};

int16_t audio_samples[NBR_OF_OSCs][kBlockSize];
uint8_t sync_samples[NBR_OF_OSCs][kBlockSize];

// bool trigger_detected_flag;
volatile bool trigger_flag[NBR_OF_OSCs];

// extern "C" {

// void HardFault_Handler(void) { while (1); }
// void MemManage_Handler(void) { while (1); }
// void BusFault_Handler(void) { while (1); }
// void UsageFault_Handler(void) { while (1); }
// void NMI_Handler(void) { }
// void SVC_Handler(void) { }
// void DebugMon_Handler(void) { }
// void PendSV_Handler(void) { }

// }

// //  Hack to remove a bunch (~500 bytes) of C++ at exit data in .bss
// extern "C"{
//     int __aeabi_atexit(void *object, void (*destructor)(void *), void *dso_handle) {
//         return 0;
//     }

//     void __cxa_atexit() {
//     }

//     void __register_exitproc() {
//     }
// }


void Init() {

  for (int i = 0; i < NBR_OF_OSCs; i++){
      osc[i].Init();
      envelope[i].Init();
      ws[i].Init(42000 * (i + 1));
      fill(&audio_samples[i][0], &audio_samples[i][kBlockSize], 0);
      fill(&sync_samples[i][0], &sync_samples[i][kBlockSize], 0);
  }

  for (int i = 0; i < NBR_OF_CHANs; i++){
      settings[i].Init();
  }
}

const uint16_t bit_reduction_masks[] = {
    0xc000,
    0xe000,
    0xf000,
    0xf800,
    0xff00,
    0xfff0,
    0xffff };

const uint16_t decimation_factors[] = { 24, 12, 6, 4, 3, 2, 1 };

void RenderBlock(int osc_id, int chan_id) {
  static int16_t previous_pitch[NBR_OF_OSCs] = {0};
  static uint16_t gain_lp[NBR_OF_OSCs];

#ifdef PROFILE_RENDER
  debug_pin.High();
#endif
  envelope[osc_id].Update(
      settings[chan_id].GetValue(SETTING_AD_ATTACK) * 8,
      settings[chan_id].GetValue(SETTING_AD_DECAY) * 8);
  uint32_t ad_value = envelope[osc_id].Render();

  osc[osc_id].set_shape(settings[chan_id].shape());

  // Set timbre and color: CV + internal modulation.
  uint16_t parameters[2];
  for (uint16_t i = 0; i < 2; ++i) {
    int32_t value = cc_params[chan_id][i];
    Setting ad_mod_setting = i == 0 ? SETTING_AD_TIMBRE : SETTING_AD_COLOR;
    value += ad_value * settings[chan_id].GetValue(ad_mod_setting) >> 5;
    CONSTRAIN(value, 0, 32767);
    parameters[i] = value;
  }
  osc[osc_id].set_parameters(parameters[0], parameters[1]);

  int32_t pitch = midi_pitch[osc_id];
  previous_pitch[osc_id] = pitch;

  //pitch += jitter_source.Render(settings[chan_id].vco_drift());
  //pitch += internal_adc.value() >> 8;
  pitch += ad_value * settings[chan_id].GetValue(SETTING_AD_FM) >> 7;

  //  Adjust pitch for Pico Keys
  pitch -= 75;

  if (pitch > 16383) {
    pitch = 16383;
  } else if (pitch < 0) {
    pitch = 0;
  }

  if (settings[chan_id].vco_flatten()) {
    pitch = Interpolate88(lut_vco_detune, pitch << 2);
  }
  osc[osc_id].set_pitch(pitch + settings[chan_id].pitch_transposition());

  if (trigger_flag[osc_id]) {
    osc[osc_id].Strike();
    envelope[osc_id].Trigger(ENV_SEGMENT_ATTACK);
    trigger_flag[osc_id] = false;
  }

  uint8_t* sync_buffer = sync_samples[osc_id];
  int16_t* render_buffer = audio_samples[osc_id];

  if (settings[chan_id].GetValue(SETTING_AD_VCA) != 0
    || settings[chan_id].GetValue(SETTING_AD_TIMBRE) != 0
    || settings[chan_id].GetValue(SETTING_AD_COLOR) != 0
    || settings[chan_id].GetValue(SETTING_AD_FM) != 0) {
    memset(sync_buffer, 0, kBlockSize);
  }

  osc[osc_id].Render(sync_buffer, render_buffer, kBlockSize);

  // Copy to DAC buffer with sample rate and bit reduction applied.
  int16_t sample = 0;
  size_t decimation_factor = decimation_factors[settings[chan_id].data().sample_rate];
  uint16_t bit_mask = bit_reduction_masks[settings[chan_id].data().resolution];
  int32_t gain = settings[chan_id].GetValue(SETTING_AD_VCA) ? ad_value : 65535;
  uint16_t signature = settings[chan_id].signature() * settings[chan_id].signature() * 4095;
  for (size_t i = 0; i < kBlockSize; ++i) {
    if ((i % decimation_factor) == 0) {
      sample = render_buffer[i] & bit_mask;
    }
    sample = sample * gain_lp[osc_id] >> 16;
    gain_lp[osc_id] += (gain - gain_lp[osc_id]) >> 4;
    int16_t warped = ws[osc_id].Transform(sample);
    render_buffer[i] = Mix(sample, warped, signature);
  }
#ifdef PROFILE_RENDER
  debug_pin.Low();
#endif
}

extern "C"{

uint32_t get_from_fifo(void) {
    const uint32_t data = multicore_fifo_pop_blocking();
    return data;
}

void braids_main(void) {
    multicore_fifo_drain();
    Init();
    while (1) {

        const uint32_t data = get_from_fifo();
        const uint8_t  kind = (uint8_t)(data & 0b1111);

        switch (kind) {
        case 1:{ // Out_Buffer
            break;
        }
        case 2: { // In_Buffer

            const uint32_t  buffer_id= (data >> 4) & 0xFF;
            const int       buffer_len = AUDIO_BUFFER_LEN;
            uint32_t *ubuffer          = audio_buffer_tmp[buffer_id];

            for (int x = 0; x < buffer_len;){
                int32_t mix_buffer[kBlockSize] = {0};

                for (int osc_id = 0; osc_id < NBR_OF_OSCs; osc_id++) {
                    int chan_id;

                    if (osc_id < POLY_OSCs) {
                        chan_id = 0;
                    } else {
                        chan_id = 1 + osc_id - POLY_OSCs;
                    }

                    RenderBlock(osc_id, chan_id);

                    int16_t* render_buffer = audio_samples[osc_id];
                    for (int y = 0; y < kBlockSize; y++) {
                        mix_buffer[y] += render_buffer[y] * volume[chan_id] >> 16;
                    }
                }

                for (int y = 0; y < kBlockSize; y++, x++) {
                    if (mix_buffer[y] > 32767) {
                        mix_buffer[y] = 32767;
                    }  else if (mix_buffer[y] < -32768) {
                        mix_buffer[y] = -32768;
                    }
                    uint16_t sample = (uint16_t)(mix_buffer[y] + 0x8000) >> SAMPLE_BITS_TO_DISCARD;
                    ubuffer[x] = (uint32_t)sample << 16 | (uint32_t)sample;
                }
            }

            multicore_fifo_push_blocking((data & (~0b1111)) | 1);
            break;
        }
        case 3: {// MIDI Msg
            const uint32_t midi = (data >> 8)  & 0xFFFFFF;
            const uint8_t  chan = (midi >> 0) & 0xF;
            const uint8_t  kind = (midi >> 4) & 0xF;
            const uint8_t  key  = (midi >> 8)  & 0xFF;
            const uint8_t  val  = (midi >> 16)  & 0xFF;

            switch (kind) {
            case 0b1000:{// Note off
                break;
            }
            case 0b1001:{// Note on

                if (chan < NBR_OF_CHANs) {

                    int osc_id;

                    if (chan == 0) {
                        //  The first channel is polyphonic (round-robin)
                        static uint8_t rr_next_osc = 0;

                        osc_id = rr_next_osc;
                        rr_next_osc = (rr_next_osc + 1) % POLY_OSCs;
                    } else {
                        osc_id = POLY_OSCs + chan - 1;
                    }

                    trigger_flag[osc_id] = true;

                    midi_pitch[osc_id] = (((int32_t)key) << 7) - 24;

                }
                break;

            }
            case 0b1011:{// Control change

                if (chan < NBR_OF_CHANs) {
                    switch (key) {
                    case 0:{
                        if (val <= MAX_MIDI_VAL) {
                            const MacroOscillatorShape shape = shape_from_param[val];
                            settings[chan].SetValue(SETTING_OSCILLATOR_SHAPE, shape);
                        }
                        break;
                    }
                    case 1:{
                        cc_params[chan][0] = (int32_t)val * (MAX_PARAM / MAX_MIDI_VAL);
                        break;
                    }
                    case 2:{
                        settings[chan].SetValue(SETTING_AD_TIMBRE, val);
                        break;
                    }
                    case 3:{
                        cc_params[chan][1] = (int32_t)val * (MAX_PARAM / MAX_MIDI_VAL);
                        break;
                    }
                    case 4:{
                        settings[chan].SetValue(SETTING_AD_COLOR, val);
                        break;
                    }
                    case 5:{
                        settings[chan].SetValue(SETTING_AD_ATTACK, val);
                        break;
                    }
                    case 6:{
                        settings[chan].SetValue(SETTING_AD_DECAY, val);
                        break;
                    }
                    case 7:{
                        // Volume
                        volume[chan] = val * (MAX_PARAM / MAX_MIDI_VAL);
                        break;
                    }
                    default:{
                        break;
                    }
                    }
                }
            }
            default:{
                break;
            }
            }

            break;
        }
        default: {
            continue;
        }
        }

        //ui.DoEvents();
    }
}
}
