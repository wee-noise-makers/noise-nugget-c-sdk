#pragma once

#include <iostream>
#include "fixdsp.h"
#include "envelope-ar.h"
#include "phase.h"

namespace fixdsp {
    namespace drum {

        class WaveformKick {
        public:

            WaveformKick() {
                env_.setAttackTimeRange (envelope::S_QUARTER_SECOND);
                env_.setAttack(0);
                env_.setReleaseTimeRange (envelope::S_2_SECONDS);
            }
            void setDecay(int16_t decay) {
                if (decay < 0) decay = 0;
                decay_ = decay;
            }
            void setPunch(int16_t punch) {
                phase_decay_.setPunch(punch);
            }
            void setPunchDecay(int16_t punch_decay) {
                phase_decay_.setPunchDecay(punch_decay);
            }
            void setDrive(int16_t drive) {
                if (drive < 0) drive = 0;
                drive_ = drive;
            }
            void on(uint8_t key, int16_t velocity) {
                env_.on(velocity);
                phase_decay_.on(key);
            }

            void setWaveformData(const WaveformData &waveform_data) {
                waveform_data_ = &waveform_data;
            }

            void render(MonoBuffer &buffer) {
                PhaseBuffer phase_buf;
                MonoBuffer env_buffer;

                phase_decay_.render(phase_buf);
                env_.render(env_buffer);

                Interpolate824(waveform_data_->data(), phase_buf, buffer);
                modulate(buffer, env_buffer);
            }

        private:
            fixdsp::envelope::AR env_;

            const WaveformData * waveform_data_ = &wav_sine;

            phase::PitchDecay phase_decay_;

            int16_t decay_;
            int16_t drive_;

        };
    }
}