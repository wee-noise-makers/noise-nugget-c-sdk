#pragma once

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void braids_main(void);

#define BITS_PER_SAMPLE        12
#define SAMPLE_BITS_TO_DISCARD (16- BITS_PER_SAMPLE)
#define MAX_MIDI_VAL (127)

#define RAM_BASE ((uint32_t)0x20000000)

#define AUDIO_BUFFER_CNT 5
#define AUDIO_BUFFER_LEN 64
extern uint32_t audio_buffer_tmp[AUDIO_BUFFER_CNT][AUDIO_BUFFER_LEN];

#ifdef __cplusplus

enum class synth_param {
    Shape = 0,
    Timbre,
    AD_Timbre,
    Color,
    AD_Color,
    Attack,
    Decay,
    Volume,
    Count
};

#define PARAM_COUNT static_cast<int>(synth_param::Count)
static const char *param_name[PARAM_COUNT] =
{
    "Shape",
    "Timbre",
    "Timbre Env",
    "Color",
    "Color Env",
    "Attack",
    "Decay",
    "Volume"
};

static const char *param_name_short[PARAM_COUNT] =
{
    "Sh",
    "Ti",
    "TE",
    "Co",
    "CE",
    "Ak",
    "Dy",
    "Vo"
};

#define SHAPE_COUNT 47

static const uint8_t param_max_value[PARAM_COUNT] =
{
    SHAPE_COUNT - 1,
    127,
    15,
    127,
    15,
    15,
    15,
    127
};

static const char *shape_name[SHAPE_COUNT] =
{
    "CSAW",
    "MORPH",
    "SAW SQUARE",
    "SINE TRIAN",
    "BUZZ",
    "SQUARE SUB",
    "SAW SUB",
    "SQUARESYNC",
    "SAW SYNC",
    "3xSAW",
    "3xSQUARE",
    "3xTRIANGLE",
    "3xSINE",
    "3xRING",
    "SAW SWARM",
    "SAW COMB",
    "TOY",
    "DIGI LP",
    "DIGI PK",
    "DIGI BP",
    "DIGI HP",
    "VOSIM",
    "VOWEL",
    "VOWEL FOF",
    "HARMONICS",
    "FM",
    "FB FM",
    "!FB FM",
    "PLUCKED",
    "BOWED",
    "BLOWN",
    "FLUTED",
    "BELL",
    "DRUM",
    "KICK",
    "CYMBAL",
    "SNARE",
    "WAVETABLES",
    "WAVE MAP",
    "WAVE LINE",
    "WAVE PARA",
    "NOISE",
    "TWIN PEAKS",
    "CLOCKED",
    "GRAN CLOUD",
    "PART NOISE",
    "DIGI MOD"
};

#undef EXTERNC

#endif
