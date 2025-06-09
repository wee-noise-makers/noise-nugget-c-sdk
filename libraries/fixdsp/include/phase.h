#pragma once
#include "fixdsp.h"
#include "resources.h"

namespace fixdsp {
    namespace phase {

        inline uint32_t ComputePhaseIncrement(int16_t midi_pitch)
          __attribute__((always_inline));

        namespace distortion {

            inline void lookup(PhaseBuffer &buffer, const WaveformData &lookup, const MonoBuffer &amount) {
                auto phase_p = buffer.getWritePointer(0);
                auto amount_p = amount.getReadPointer(0);
                auto len = buffer.getBufferLength();

                for (int i = 0; i < len; i++) {
                    const auto phase_in = phase_p[i];

                    const uint64_t lookup_64 = Interpolate824(lookup.data(), phase_in);
                    const uint64_t new_phase_64 =
                      (static_cast<uint64_t>(lookup_64 + 32768) << 16) - 1;
                    const uint32_t new_phase_32 = static_cast<uint32_t>(new_phase_64);

                    phase_p[i] = Mix (phase_in, new_phase_32, amount_p[i]);
                }
            }

            inline void resonantFull(PhaseBuffer &buffer, const MonoBuffer &amount, MonoBuffer &attenuation) {
                auto phase_p = buffer.getWritePointer(0);
                auto amount_p = amount.getReadPointer(0);
                auto att_p = attenuation.getWritePointer(0);
                auto len = buffer.getBufferLength();

                const uint32_t MAX_U32 = std::numeric_limits<uint32_t>::max();
                const uint64_t MAX_U64 = std::numeric_limits<uint64_t>::max();
                const uint32_t MAX_S32 = std::numeric_limits<MonoSample>::max();

                for (int i = 0; i < len; i++) {
                    if (phase_p[i] < (MAX_U32 / 2)) {
                        att_p[i] = MAX_S32; // No attenuation
                    } else {
                        const uint32_t phase_in = phase_p[i];

                        // For the second half of the phase we increase the phase rate
                        // to produce a higher frequency waveform. Up to several octaves
                        // higher than the base pitch.
                        const uint64_t multiplier =
                          MAX_U64 + (static_cast<uint64_t>(amount_p[i]) << 22);
                        const uint64_t phase_u64 =
                           (static_cast<uint64_t>(phase_in - (MAX_U32 / 2)) * multiplier) >> 32;
                        const uint32_t new_phase =
                           static_cast<uint32_t>(phase_u64 % MAX_U32);

                        phase_p[i]= new_phase;

                        // This high frequency part of the waveform is then linearly
                        // attenuated to merge without discontinuities.
                        const uint32_t phase_invert = MAX_U32 - phase_in;
                        const uint32_t scaled = phase_invert >> 16;
                        const int16_t attenuation = static_cast<int16_t>(scaled - 1);

                        att_p[i] = attenuation;
                    }

                }
            }

            inline void resonantSecondHalf(PhaseBuffer &buffer, const MonoBuffer &amount, MonoBuffer &attenuation) {
                auto phase_p = buffer.getWritePointer(0);
                auto amount_p = amount.getReadPointer(0);
                auto att_p = attenuation.getWritePointer(0);
                auto len = buffer.getBufferLength();

                const uint32_t MAX_U32 = std::numeric_limits<uint32_t>::max();
                const uint64_t MAX_U64 = std::numeric_limits<uint64_t>::max();
                const uint32_t MAX_S32 = std::numeric_limits<MonoSample>::max();

                for (int i = 0; i < len; i++) {
                    if (phase_p[i] < (MAX_U32 / 2)) {
                        att_p[i] = MAX_S32; // No attenuation
                    } else {
                        const uint32_t phase_in = phase_p[i];

                        // For the second half of the phase we increase the phase rate
                        // to produce a higher frequency waveform. Up to several octaves
                        // higher than the base pitch.
                        const uint64_t multiplier =
                          MAX_U64 + (static_cast<uint64_t>(amount_p[i]) << 22);
                        const uint64_t phase_u64 =
                           (static_cast<uint64_t>(phase_in - (MAX_U32 / 2)) * multiplier) >> 32;
                        const uint32_t new_phase =
                           static_cast<uint32_t>((MAX_U32 / 2) + (phase_u64 ) % (MAX_U32 / 2));

                        phase_p[i]= new_phase;

                        // This high frequency part of the waveform is then linearly
                        // attenuated to merge without discontinuities.
                        const uint32_t phase_invert = MAX_U32 - phase_in;
                        const uint32_t scaled = phase_invert >> 16;
                        const int16_t attenuation = static_cast<int16_t>(scaled - 1);

                        att_p[i] = attenuation;
                    }

                }
            }
        }

        class ConstantPitch {
        public:

            void setKey(uint8_t key) {
                setPitch(keyToPitch(key));
            }

            void setPitch(int16_t pitch) {
                phase_increment_ = ComputePhaseIncrement(pitch);
            }

            inline uint32_t phaseRender() {
                phase_ += phase_increment_;
                return phase_;
            }

            inline void render(PhaseBuffer &output) {
                for (auto &phase_out : output.getBufferContainer()[0]) {
                    phase_ += phase_increment_;
                    phase_out = phase_;
                }
            }

            uint32_t phase_ = 0;
            uint32_t phase_increment_ = 0;
        };

        class PitchDecay {
        public:

            void setPunch(int16_t punch) {
                if (punch < 0) punch = 0;
                punch_ = punch;
            }
            void setPunchDecay(int16_t punch_decay) {
                if (punch_decay < 0) punch_decay = 0;
                punch_decay_ = punch_decay;
            }

            void on(uint8_t key) {
                on(keyToPitch(key));
            }

            void on(int16_t pitch) {
                target_phase_increment_ = ComputePhaseIncrement(pitch);

                const uint32_t offset_phase_incr =
                  ComputePhaseIncrement(pitch + 12 * 128 * 5);

                phase_increment_ = Mix(target_phase_increment_, offset_phase_incr, punch_);

                const uint32_t param =
                  static_cast<uint32_t>(MAX_PARAM) - static_cast<uint32_t>(punch_decay_);

                phase_incr_delta_ =
                  (phase_increment_ - target_phase_increment_) /
                   (FIXDSP_SAMPLE_RATE / (1 + param / 256));

                if (phase_incr_delta_ == 0) {
                    phase_incr_delta_ = 1;
                }
            }

            inline uint32_t phaseRender() {

                if (phase_increment_ > target_phase_increment_) {
                    phase_increment_ -= phase_incr_delta_;
                }

                phase_ += phase_increment_;
                return phase_;
            }

            inline void render(PhaseBuffer &output) {
                for (auto &phase_out : output.getBufferContainer()[0]) {
                    if (phase_increment_ > target_phase_increment_) {
                        phase_increment_ -= phase_incr_delta_;
                    }

                    phase_ += phase_increment_;
                    phase_out = phase_;
                }
            }

            uint32_t phase_ = 0;
            uint32_t target_phase_increment_ = 0;
            uint32_t phase_increment_ = 0;
            uint32_t phase_incr_delta_ = 0;

            int16_t decay_;
            int16_t punch_;
            int16_t punch_decay_;
        };

        class PitchGlide {
        public:

            void setGlide(int16_t glide) {
                if (glide < 0) glide = 0;
                glide_ = glide;
            }

            void setMaxGlideDuration(float seconds) {
                const float max_allowed =
                  static_cast<float>(std::numeric_limits<uint32_t>::max() / FIXDSP_SAMPLE_RATE);

                if (seconds < 0) seconds = 0;
                if (seconds > 100.0) seconds = max_allowed;

                max_glide_ =
                  static_cast<uint32_t>(static_cast<float>(FIXDSP_SAMPLE_RATE) * seconds);
            }

            void setKey(uint8_t key) {
                setPitch(keyToPitch(key));
            }

            void setPitch(int16_t pitch) {

                target_phase_increment_ = ComputePhaseIncrement(pitch);

                org_phase_increment_ = phase_increment_;

                if (glide_ == 0 || phase_increment_ == 0) {
                    phase_increment_ = target_phase_increment_;
                    phase_incr_delta_ = 0;
                } else {
                    const int64_t param = static_cast<int64_t>(MAX_PARAM - glide_);
                    const int64_t incr_64 = static_cast<int64_t>(phase_increment_);
                    const int64_t target_64 = static_cast<int64_t>(target_phase_increment_);
                    const int64_t diff_64 = target_64 - incr_64;
                    const int64_t diff_mod_64 = diff_64 / Mix(1, max_glide_, glide_);

                    phase_incr_delta_ = static_cast<uint32_t>(diff_mod_64);
                    if (phase_incr_delta_ == 0) {
                        phase_incr_delta_ = 1;
                    }
                }
            }

            inline void render(PhaseBuffer &output) {
                if (phase_increment_ == target_phase_increment_) {
                    for (auto &phase_out : output.getBufferContainer()[0]) {
                        phase_ += phase_increment_;
                        phase_out = phase_;
                    }

                } else if (org_phase_increment_ < target_phase_increment_) {
                    for (auto &phase_out : output.getBufferContainer()[0]) {
                        if (phase_increment_ < target_phase_increment_) {
                            phase_increment_ += phase_incr_delta_;
                        }

                        phase_ += phase_increment_;
                        phase_out = phase_;
                    }
                } else {
                    for (auto &phase_out : output.getBufferContainer()[0]) {
                        if (phase_increment_ > target_phase_increment_) {
                            phase_increment_ += phase_incr_delta_;
                        }

                        phase_ += phase_increment_;
                        phase_out = phase_;
                    }
                }
            }

            uint32_t phase_ = 0;
            uint32_t target_phase_increment_ = 0;
            uint32_t phase_increment_ = 0;
            uint32_t org_phase_increment_ = 0;
            int64_t phase_incr_delta_ = 0;

            int16_t glide_ = 0;
            uint32_t max_glide_ = FIXDSP_SAMPLE_RATE; // 1 second
        };

        inline uint32_t ComputePhaseIncrement(int16_t midi_pitch) {
            static const uint16_t kHighestNote = 140 * 128;
            static const uint16_t kPitchTableStart = 128 * 128;
            static const uint16_t kOctave = 12 * 128;

            if (midi_pitch >= kHighestNote) {
                midi_pitch = kHighestNote - 1;
            }

            int32_t ref_pitch = midi_pitch;
            ref_pitch -= kPitchTableStart;

            size_t num_shifts = 0;
            while (ref_pitch < 0) {
                ref_pitch += kOctave;
                ++num_shifts;
            }

            uint32_t a = lut_oscillator_increments[ref_pitch >> 4];
            uint32_t b = lut_oscillator_increments[(ref_pitch >> 4) + 1];
            uint32_t phase_increment = a + \
              (static_cast<int32_t>(b - a) * (ref_pitch & 0xf) >> 4);
            phase_increment >>= num_shifts;
            return phase_increment;
        }
    }
}
