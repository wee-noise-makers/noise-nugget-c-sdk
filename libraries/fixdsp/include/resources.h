
#include <cstdint>
#include "fixdsp.h"



namespace fixdsp {

typedef uint8_t ResourceId;

extern const uint16_t* lookup_table_table[];

extern const int16_t* lookup_table_signed_table[];

extern const uint32_t* lookup_table_hr_table[];

extern const int16_t* waveform_table[];

extern const int16_t* waveshaper_table[];

extern const uint16_t lut_resonator_coefficient[];
extern const uint16_t lut_resonator_scale[];
extern const uint16_t lut_svf_cutoff[];
extern const uint16_t lut_svf_damp[];
extern const uint16_t lut_svf_scale[];
extern const uint16_t lut_granular_envelope[];
extern const uint16_t lut_granular_envelope_rate[];
extern const uint16_t lut_bowing_envelope[];
extern const uint16_t lut_bowing_friction[];
extern const uint16_t lut_blowing_envelope[];
extern const uint16_t lut_flute_body_filter[];
extern const uint16_t lut_fm_frequency_quantizer[];
extern const uint16_t lut_vco_detune[];
extern const uint16_t lut_bell[];
extern const uint16_t lut_env_linear[];
extern const uint16_t lut_env_expo[];
extern const uint16_t lut_env_log[];
extern const WaveformData lut_blowing_jet;
extern const WaveformData lut_tanh;
extern const uint32_t lut_oscillator_increments[];
extern const uint32_t lut_lfo_increments[];
extern const uint32_t lut_env_increments_10seconds[];
extern const uint32_t lut_env_increments_5seconds[];
extern const uint32_t lut_env_increments_2seconds[];
extern const uint32_t lut_env_increments_1seconds[];
extern const uint32_t lut_env_increments_half_second[];
extern const uint32_t lut_env_increments_quarter_second[];
extern const int16_t wav_formant_sine[];
extern const int16_t wav_formant_square[];
extern const WaveformData wav_sine;
extern const WaveformData wav_cos;
extern const WaveformData wav_sine_warp1;
extern const WaveformData wav_sine_warp2;
extern const WaveformData wav_sine_warp3;
extern const WaveformData wav_sine2_warp1;
extern const WaveformData wav_sine2_warp2;
extern const WaveformData wav_sine2_warp3;
extern const WaveformData wav_sawtooth;
extern const WaveformData wav_screech;
extern const WaveformData wav_triangle;
extern const WaveformData wav_chip_triangle;
extern const WaveformData wav_chip_pulse_50;
extern const WaveformData wav_chip_pulse_25;
extern const WaveformData wav_combined_sin_saw;
extern const WaveformData wav_combined_saw_sin;
extern const WaveformData wav_combined_saw_full_sin;
extern const WaveformData wav_combined_trig_sin;
extern const WaveformData wav_combined_trig_full_sin;
extern const WaveformData wav_combined_sin_square;
extern const WaveformData wav_combined_square_sin;
extern const WaveformData wav_combined_square_full_sin;
extern const WaveformData wav_sine_lfo;
extern const WaveformData wav_triangle_lfo;
extern const WaveformData wav_ramp_up_lfo;
extern const WaveformData wav_ramp_down_lfo;
extern const WaveformData wav_exp_up_lfo;
extern const WaveformData wav_exp_down_lfo;
extern const WaveformData wav_bandlimited_comb_0;
extern const WaveformData wav_bandlimited_comb_1;
extern const WaveformData wav_bandlimited_comb_2;
extern const WaveformData wav_bandlimited_comb_3;
extern const WaveformData wav_bandlimited_comb_4;
extern const WaveformData wav_bandlimited_comb_5;
extern const WaveformData wav_bandlimited_comb_6;
extern const WaveformData wav_bandlimited_comb_7;
extern const WaveformData wav_bandlimited_comb_8;
extern const WaveformData wav_bandlimited_comb_9;
extern const WaveformData wav_bandlimited_comb_10;
extern const WaveformData ws_moderate_overdrive;
extern const WaveformData ws_violent_overdrive;
extern const WaveformData ws_extreme_overdrive;
extern const WaveformData ws_step_overdrive;
extern const WaveformData ws_violent_step_overdrive;
extern const WaveformData ws_extreme_step_overdrive;
extern const WaveformData ws_sine_fold;
extern const WaveformData ws_tri_fold;
#define LUT_RESONATOR_COEFFICIENT 0
#define LUT_RESONATOR_COEFFICIENT_SIZE 129
#define LUT_RESONATOR_SCALE 1
#define LUT_RESONATOR_SCALE_SIZE 129
#define LUT_SVF_CUTOFF 2
#define LUT_SVF_CUTOFF_SIZE 257
#define LUT_SVF_DAMP 3
#define LUT_SVF_DAMP_SIZE 257
#define LUT_SVF_SCALE 4
#define LUT_SVF_SCALE_SIZE 257
#define LUT_GRANULAR_ENVELOPE 5
#define LUT_GRANULAR_ENVELOPE_SIZE 513
#define LUT_GRANULAR_ENVELOPE_RATE 6
#define LUT_GRANULAR_ENVELOPE_RATE_SIZE 257
#define LUT_BOWING_ENVELOPE 7
#define LUT_BOWING_ENVELOPE_SIZE 92
#define LUT_BOWING_FRICTION 8
#define LUT_BOWING_FRICTION_SIZE 257
#define LUT_BLOWING_ENVELOPE 9
#define LUT_BLOWING_ENVELOPE_SIZE 62
#define LUT_FLUTE_BODY_FILTER 10
#define LUT_FLUTE_BODY_FILTER_SIZE 128
#define LUT_FM_FREQUENCY_QUANTIZER 11
#define LUT_FM_FREQUENCY_QUANTIZER_SIZE 129
#define LUT_VCO_DETUNE 12
#define LUT_VCO_DETUNE_SIZE 257
#define LUT_BELL 13
#define LUT_BELL_SIZE 257
#define LUT_ENV_LINEAR 14
#define LUT_ENV_LINEAR_SIZE 257
#define LUT_ENV_EXPO 15
#define LUT_ENV_EXPO_SIZE 257
#define LUT_ENV_LOG 16
#define LUT_ENV_LOG_SIZE 257
#define LUT_BLOWING_JET 0
#define LUT_BLOWING_JET_SIZE 257
#define LUT_TANH 1
#define LUT_TANH_SIZE 257
#define LUT_OSCILLATOR_INCREMENTS 0
#define LUT_OSCILLATOR_INCREMENTS_SIZE 97
#define LUT_LFO_INCREMENTS 1
#define LUT_LFO_INCREMENTS_SIZE 257
#define LUT_ENV_INCREMENTS_10SECONDS 2
#define LUT_ENV_INCREMENTS_10SECONDS_SIZE 128
#define LUT_ENV_INCREMENTS_5SECONDS 3
#define LUT_ENV_INCREMENTS_5SECONDS_SIZE 128
#define LUT_ENV_INCREMENTS_2SECONDS 4
#define LUT_ENV_INCREMENTS_2SECONDS_SIZE 128
#define LUT_ENV_INCREMENTS_1SECONDS 5
#define LUT_ENV_INCREMENTS_1SECONDS_SIZE 128
#define LUT_ENV_INCREMENTS_HALF_SECOND 6
#define LUT_ENV_INCREMENTS_HALF_SECOND_SIZE 128
#define LUT_ENV_INCREMENTS_QUARTER_SECOND 7
#define LUT_ENV_INCREMENTS_QUARTER_SECOND_SIZE 128
#define WAV_FORMANT_SINE 0
#define WAV_FORMANT_SINE_SIZE 256
#define WAV_FORMANT_SQUARE 1
#define WAV_FORMANT_SQUARE_SIZE 256
#define WAV_SINE 2
#define WAV_SINE_SIZE 257
#define WAV_COS 3
#define WAV_COS_SIZE 257
#define WAV_SINE_WARP1 4
#define WAV_SINE_WARP1_SIZE 257
#define WAV_SINE_WARP2 5
#define WAV_SINE_WARP2_SIZE 257
#define WAV_SINE_WARP3 6
#define WAV_SINE_WARP3_SIZE 257
#define WAV_SINE2_WARP1 7
#define WAV_SINE2_WARP1_SIZE 257
#define WAV_SINE2_WARP2 8
#define WAV_SINE2_WARP2_SIZE 257
#define WAV_SINE2_WARP3 9
#define WAV_SINE2_WARP3_SIZE 257
#define WAV_SAWTOOTH 10
#define WAV_SAWTOOTH_SIZE 257
#define WAV_SCREECH 11
#define WAV_SCREECH_SIZE 257
#define WAV_TRIANGLE 12
#define WAV_TRIANGLE_SIZE 257
#define WAV_CHIP_TRIANGLE 13
#define WAV_CHIP_TRIANGLE_SIZE 257
#define WAV_CHIP_PULSE_50 14
#define WAV_CHIP_PULSE_50_SIZE 257
#define WAV_CHIP_PULSE_25 15
#define WAV_CHIP_PULSE_25_SIZE 257
#define WAV_COMBINED_SIN_SAW 16
#define WAV_COMBINED_SIN_SAW_SIZE 257
#define WAV_COMBINED_SAW_SIN 17
#define WAV_COMBINED_SAW_SIN_SIZE 257
#define WAV_COMBINED_SAW_FULL_SIN 18
#define WAV_COMBINED_SAW_FULL_SIN_SIZE 257
#define WAV_COMBINED_TRIG_SIN 19
#define WAV_COMBINED_TRIG_SIN_SIZE 257
#define WAV_COMBINED_TRIG_FULL_SIN 20
#define WAV_COMBINED_TRIG_FULL_SIN_SIZE 257
#define WAV_COMBINED_SIN_SQUARE 21
#define WAV_COMBINED_SIN_SQUARE_SIZE 257
#define WAV_COMBINED_SQUARE_SIN 22
#define WAV_COMBINED_SQUARE_SIN_SIZE 257
#define WAV_COMBINED_SQUARE_FULL_SIN 23
#define WAV_COMBINED_SQUARE_FULL_SIN_SIZE 257
#define WAV_SINE_LFO 24
#define WAV_SINE_LFO_SIZE 257
#define WAV_TRIANGLE_LFO 25
#define WAV_TRIANGLE_LFO_SIZE 257
#define WAV_RAMP_UP_LFO 26
#define WAV_RAMP_UP_LFO_SIZE 257
#define WAV_RAMP_DOWN_LFO 27
#define WAV_RAMP_DOWN_LFO_SIZE 257
#define WAV_EXP_UP_LFO 28
#define WAV_EXP_UP_LFO_SIZE 257
#define WAV_EXP_DOWN_LFO 29
#define WAV_EXP_DOWN_LFO_SIZE 257
#define WAV_BANDLIMITED_COMB_0 30
#define WAV_BANDLIMITED_COMB_0_SIZE 257
#define WAV_BANDLIMITED_COMB_1 31
#define WAV_BANDLIMITED_COMB_1_SIZE 257
#define WAV_BANDLIMITED_COMB_2 32
#define WAV_BANDLIMITED_COMB_2_SIZE 257
#define WAV_BANDLIMITED_COMB_3 33
#define WAV_BANDLIMITED_COMB_3_SIZE 257
#define WAV_BANDLIMITED_COMB_4 34
#define WAV_BANDLIMITED_COMB_4_SIZE 257
#define WAV_BANDLIMITED_COMB_5 35
#define WAV_BANDLIMITED_COMB_5_SIZE 257
#define WAV_BANDLIMITED_COMB_6 36
#define WAV_BANDLIMITED_COMB_6_SIZE 257
#define WAV_BANDLIMITED_COMB_7 37
#define WAV_BANDLIMITED_COMB_7_SIZE 257
#define WAV_BANDLIMITED_COMB_8 38
#define WAV_BANDLIMITED_COMB_8_SIZE 257
#define WAV_BANDLIMITED_COMB_9 39
#define WAV_BANDLIMITED_COMB_9_SIZE 257
#define WAV_BANDLIMITED_COMB_10 40
#define WAV_BANDLIMITED_COMB_10_SIZE 257
#define WAV_BANDLIMITED_COMB_11 41
#define WAV_BANDLIMITED_COMB_11_SIZE 257
#define WAV_BANDLIMITED_COMB_12 42
#define WAV_BANDLIMITED_COMB_12_SIZE 257
#define WAV_BANDLIMITED_COMB_13 43
#define WAV_BANDLIMITED_COMB_13_SIZE 257
#define WAV_BANDLIMITED_COMB_14 44
#define WAV_BANDLIMITED_COMB_14_SIZE 257
#define WS_MODERATE_OVERDRIVE 0
#define WS_MODERATE_OVERDRIVE_SIZE 257
#define WS_OVERDRIVE 1
#define WS_OVERDRIVE_SIZE 257
#define WS_VIOLENT_OVERDRIVE 2
#define WS_VIOLENT_OVERDRIVE_SIZE 257
#define WS_EXTREME_OVERDRIVE 3
#define WS_EXTREME_OVERDRIVE_SIZE 257
#define WS_STEP_OVERDRIVE 4
#define WS_STEP_OVERDRIVE_SIZE 257
#define WS_VIOLENT_STEP_OVERDRIVE 5
#define WS_VIOLENT_STEP_OVERDRIVE_SIZE 257
#define WS_EXTREME_STEP_OVERDRIVE 6
#define WS_EXTREME_STEP_OVERDRIVE_SIZE 257
#define WS_SINE_FOLD 7
#define WS_SINE_FOLD_SIZE 257
#define WS_TRI_FOLD 8
#define WS_TRI_FOLD_SIZE 257

}  // namespace fixdsp
