#include "waveform_oscillator.h"
#include "phase.h"

namespace fixdsp {
    namespace oscillator {

        void WaveformOscillator::setPitch(int16_t pitch) {
            this->phase.setPitch(pitch);
        }

        void WaveformOscillator::setKey(uint8_t key) {
            this->phase.setKey(key);
        }

        void WaveformOscillator::setGlide(int16_t glide) {
            this->phase.setGlide(glide);
        }

        void WaveformOscillator::setMaxGlideDuration(float seconds) {
            this->phase.setMaxGlideDuration(seconds);
        }

        void WaveformOscillator::setWaveformData(const WaveformData &waveform_data) {
            this->waveform_data = &waveform_data;
        }

        void WaveformOscillator::render(MonoBuffer &buffer) {
            auto wave_data = this->waveform_data;
            PhaseBuffer phase_buf;

            this->phase.render(phase_buf);
            Interpolate824(wave_data->data(), phase_buf, buffer);
        }
    }
}
