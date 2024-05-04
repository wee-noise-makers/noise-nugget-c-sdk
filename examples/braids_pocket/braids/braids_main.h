#pragma once

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void braids_main(void);

#define BITS_PER_SAMPLE        12
#define SAMPLE_BITS_TO_DISCARD (16- BITS_PER_SAMPLE)
#define MAX_MIDI_VAL (15)

#define RAM_BASE ((uint32_t)0x20000000)

#define AUDIO_BUFFER_CNT 5
#define AUDIO_BUFFER_LEN 64
extern uint32_t audio_buffer_tmp[AUDIO_BUFFER_CNT][AUDIO_BUFFER_LEN];

typedef enum synth_param {
    Timbre = 0,
    AD_Timbre,
    Color,
    AD_Color,
    Shape,
    Attack,
    Decay,
    Volume,
    PARAM_COUNT
} synth_param;

static const char *param_name[PARAM_COUNT] =
{
    "Timbre",
    "Timbre Env",
    "Color",
    "Color Env",
    "Shape",
    "Attack",
    "Decay",
    "Volume"
};

static const char *param_name_short[PARAM_COUNT] =
{
    "Ti",
    "TE",
    "Co",
    "CE",
    "Sh",
    "Ak",
    "Dy",
    "Vo"
};

static const char *shape_name[MAX_MIDI_VAL + 1] =
{
    "Saw Swarm",
    "Saw Comb",
    "Triple Saw",
    "Triple Square",
    "Triple Triangle",
    "Triple Sine",
    "Filter BP",
    "Vosim",
    "Vowel",
    "Feedback FM",
    "Plucked",
    "Bowed",
    "Blown",
    "Twin Peaks",
    "Kick",
    "Snare",
};

static const char *value_name[MAX_MIDI_VAL + 1] =
{
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15"
};

#undef EXTERNC
