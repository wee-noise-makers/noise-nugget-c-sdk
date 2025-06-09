#pragma once

#include "fixdsp.h"
#include "phase.h"
#include "resources.h"

namespace fixdsp {

    namespace oscillator {
        class WaveformOscillator{
            public:

            WaveformOscillator() { }
           ~WaveformOscillator() { }

           void setPitch(int16_t pitch);
           void setKey(uint8_t key);

           void setGlide(int16_t glide);
           void setMaxGlideDuration(float seconds);

           void setWaveformData(const WaveformData &waveform_data);

           void render(MonoBuffer &buffer);

           private:

           const WaveformData * waveform_data = &wav_sine;
           fixdsp::phase::PitchGlide phase;
        };
    }
}
