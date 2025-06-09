#pragma once

#include "fixdsp.h"
#include "resources.h"
#include "phase.h"

namespace fixdsp {

    namespace oscillator {
        class PhaseDistortionLookupOscillator{
            public:

            PhaseDistortionLookupOscillator() { }
           ~PhaseDistortionLookupOscillator() { }

           void setPitch(int16_t pitch);

           void setKey(uint8_t key);

           void setGlide(int16_t glide);
           void setMaxGlideDuration(float seconds);

           void setWaveformData(const WaveformData &waveform_data);
           void setLookupData(const WaveformData &loopup_data);

           void render(MonoBuffer &buffer, const MonoBuffer &amount);

           private:

           const WaveformData * waveform_data_ = &wav_sine;
           const WaveformData * lookup_data_ = &wav_sine2_warp2;
           phase::PitchGlide phase_gen_;
        };

        class PhaseDistortionResonantOscillator{
            public:

            PhaseDistortionResonantOscillator() { }
           ~PhaseDistortionResonantOscillator() { }

           void setPitch(int16_t pitch);
           void setKey(uint8_t key);

           void setGlide(int16_t glide);
           void setMaxGlideDuration(float seconds);

           void setWaveformData(const WaveformData &waveform_data);

           // Repeat the full waveform
           void renderResonance(MonoBuffer &buffer, const MonoBuffer &amount);

           // Repeat the second half of the waveform
           void renderResonanceHalf(MonoBuffer &buffer, const MonoBuffer &amount);

           private:

           const WaveformData * waveform_data_ = &wav_sine;
           phase::PitchGlide phase_gen_;
        };
    }
}
