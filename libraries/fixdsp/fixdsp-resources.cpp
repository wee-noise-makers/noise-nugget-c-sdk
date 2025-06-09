#include "fixdsp.h"

#ifndef FIXDSP_SAMPLE_RATE
#error "FIXDSP_SAMPLE_RATE not defined"
#endif

#if FIXDSP_SAMPLE_RATE == 8000
#include "fixdsp-resources-8000.cpp"
#elif FIXDSP_SAMPLE_RATE == 11025
#include "fixdsp-resources-11025.cpp"
#elif FIXDSP_SAMPLE_RATE == 16000
#include "fixdsp-resources-16000.cpp"
#elif FIXDSP_SAMPLE_RATE == 22050
#include "fixdsp-resources-22050.cpp"
#elif FIXDSP_SAMPLE_RATE == 32000
#include "fixdsp-resources-32000.cpp"
#elif FIXDSP_SAMPLE_RATE == 44100
#include "fixdsp-resources-44100.cpp"
#elif FIXDSP_SAMPLE_RATE == 48000
#include "fixdsp-resources-48000.cpp"
#elif FIXDSP_SAMPLE_RATE == 96000
#include "fixdsp-resources-96000.cpp"
#else
#error "Invalid value for FIXDSP_SAMPLE_RATE "
#endif
