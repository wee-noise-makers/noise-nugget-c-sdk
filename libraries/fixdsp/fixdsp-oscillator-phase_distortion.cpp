#include "phase_distortion_oscillator.h"
#include "phase.h"

namespace fixdsp {
    namespace oscillator {

        void PhaseDistortionLookupOscillator::setPitch(int16_t pitch) {
            phase_gen_.setPitch(pitch);
        }

        void PhaseDistortionLookupOscillator::setKey(uint8_t key) {
            phase_gen_.setKey(key);
        }

        void PhaseDistortionLookupOscillator::setGlide(int16_t glide) {
            phase_gen_.setGlide(glide);
        }

        void PhaseDistortionLookupOscillator::setMaxGlideDuration(float seconds) {
            phase_gen_.setMaxGlideDuration(seconds);
        }

        void PhaseDistortionLookupOscillator::setWaveformData(const WaveformData &waveform_data) {
            waveform_data_ = &waveform_data;
        }

        void PhaseDistortionLookupOscillator::setLookupData(const WaveformData &lookup_data) {
            lookup_data_ = &lookup_data;
        }

        void PhaseDistortionLookupOscillator::render(MonoBuffer &buffer, const MonoBuffer &amount) {
            PhaseBuffer phase_buf;

            phase_gen_.render(phase_buf);
            phase::distortion::lookup(phase_buf, *lookup_data_, amount);
            Interpolate824(waveform_data_->data(), phase_buf, buffer);
        }

        void PhaseDistortionResonantOscillator::setPitch(int16_t pitch) {
            phase_gen_.setPitch(pitch);
        }

        void PhaseDistortionResonantOscillator::setKey(uint8_t key) {
            phase_gen_.setKey(key);
        }

        void PhaseDistortionResonantOscillator::setGlide(int16_t glide) {
            phase_gen_.setGlide(glide);
        }

        void PhaseDistortionResonantOscillator::setMaxGlideDuration(float seconds) {
            phase_gen_.setMaxGlideDuration(seconds);
        }

        void PhaseDistortionResonantOscillator::setWaveformData(const WaveformData &waveform_data) {
            waveform_data_ = &waveform_data;
        }

        void PhaseDistortionResonantOscillator::renderResonance(MonoBuffer &buffer, const MonoBuffer &amount) {
            PhaseBuffer phase_buf;
            MonoBuffer attenuation;

            phase_gen_.render(phase_buf);
            phase::distortion::resonantFull(phase_buf, amount, attenuation);
            Interpolate824(waveform_data_->data(), phase_buf, buffer);
            modulate(buffer, attenuation);
        }

        void PhaseDistortionResonantOscillator::renderResonanceHalf(MonoBuffer &buffer, const MonoBuffer &amount) {
            PhaseBuffer phase_buf;
            MonoBuffer attenuation;

            phase_gen_.render(phase_buf);
            phase::distortion::resonantSecondHalf(phase_buf, amount, attenuation);
            Interpolate824(waveform_data_->data(), phase_buf, buffer);
            modulate(buffer, attenuation);
        }
    }
}
