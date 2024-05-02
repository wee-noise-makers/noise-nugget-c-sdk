#pragma once

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void braids_main(void);

#define BITS_PER_SAMPLE        10
#define SAMPLE_BITS_TO_DISCARD (16- BITS_PER_SAMPLE)

#define RAM_BASE ((uint32_t)0x20000000)

#define AUDIO_BUFFER_CNT 5
#define AUDIO_BUFFER_LEN 64
extern uint32_t audio_buffer_tmp[AUDIO_BUFFER_CNT][AUDIO_BUFFER_LEN];

#undef EXTERNC
