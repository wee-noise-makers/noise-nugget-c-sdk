#pragma once

#include <cstdint>
#include "fixdsp.h"
#include "resources.h"

namespace fixdsp {
    namespace envelope {
        enum SegmentTime {
            S_10_SECONDS = 0,
            S_5_SECONDS = 1,
            S_2_SECONDS = 2,
            S_1_SECONDS = 3,
            S_HALF_SECOND = 4,
            S_QUARTER_SECOND = 5,
        };

        enum SegmentCurve {
            CURVE_EXPO = 0,
            CURVE_LINEAR = 1,
            CURVE_LOG = 2,
        };

        enum EnvelopeSegment {
            ENV_SEGMENT_ATTACK = 0,
            ENV_SEGMENT_HOLD = 1,
            ENV_SEGMENT_RELEASE = 2,
            ENV_SEGMENT_DEAD = 3,
            ENV_NUM_SEGMENTS,
        };

        class AR {
        public:
            AR() {
                value_ = 0;
                hold_ = false;
                attack_time_ = S_2_SECONDS;
                attack_curve_ = lut_env_log;
                release_curve_ = lut_env_log;
                release_time_ = S_5_SECONDS;

                target_[ENV_SEGMENT_ATTACK] = std::numeric_limits<int16_t>::max();
                target_[ENV_SEGMENT_RELEASE] = 0;
                target_[ENV_SEGMENT_DEAD] = 0;
                increment_[ENV_SEGMENT_DEAD] = 0;
                increment_[ENV_SEGMENT_HOLD] = 0;

                updateIncrements();
            }

            ~AR() { }

            inline void setHold(bool hold) {
                hold_ = hold;
            }

            inline void setAttackTimeRange(SegmentTime time) {
                attack_time_ = time;
                updateIncrements();
            }

            inline void setAttackCurve(SegmentCurve curve){
                switch(curve) {
                  case CURVE_EXPO:
                    attack_curve_ = lut_env_log;
                    break;
                  case CURVE_LINEAR:
                    attack_curve_ = lut_env_linear;
                    break;
                  default:
                    attack_curve_ = lut_env_expo;
                }
            }

            inline void setReleaseTimeRange(SegmentTime time) {
                release_time_ = time;
                updateIncrements();
            }

            inline void setReleaseCurve(SegmentCurve curve){
                switch(curve) {
                  case CURVE_EXPO:
                    release_curve_ = lut_env_log;
                    break;
                  case CURVE_LINEAR:
                    release_curve_ = lut_env_linear;
                    break;
                  default:
                    release_curve_ = lut_env_expo;
                }
            }

            inline void setAttack(int16_t attack) {
                attack_ = attack;
                updateIncrements();
            }

            inline void setRelease(int16_t release) {
                release_ = release;
                updateIncrements();
            }

            inline void on(int16_t velocity) {
                if (velocity < 0) velocity = 0;
                target_[ENV_SEGMENT_ATTACK] = velocity;
                Trigger(ENV_SEGMENT_ATTACK);
            }

            inline void off() {
                if (segment_ != ENV_SEGMENT_RELEASE && segment_ != ENV_SEGMENT_DEAD) {
                    Trigger(ENV_SEGMENT_RELEASE);
                }
            }

            inline EnvelopeSegment segment() const {
                return static_cast<EnvelopeSegment>(segment_);
            }

            inline uint16_t render() {
                uint32_t increment = increment_[segment_];
                phase_ += increment;
                //std::cout << "phase: " << phase_ << " incr: " << increment << std::endl;
                if (phase_ < increment) {
                    // uint32_t overflow => end of the segment
                    value_ = b_;

                    // go to next segment
                    Trigger(static_cast<EnvelopeSegment>(segment_ + 1));
                }

                if (increment_[segment_]) {
                    value_ = Mix(a_, b_, Interpolate824(curve_, phase_));
                }

                return value_;
            }

            inline void render(MonoBuffer &buffer) {
                for (auto& sample : buffer.getBufferContainer()[0]) {
                    sample = render();
                }
            }

            inline int16_t value() const { return value_; }

        private:

            inline void Trigger(EnvelopeSegment segment) {

              if (segment == ENV_SEGMENT_DEAD) {
                value_ = 0;
              } else if (segment == ENV_SEGMENT_HOLD && !hold_) {
                  segment = ENV_SEGMENT_RELEASE;
              }

              a_ = value_;
              b_ = target_[segment];
              segment_ = segment;
              phase_ = 0;

              switch (segment)
              {
              case ENV_SEGMENT_ATTACK:
                  curve_ = attack_curve_;
                  break;

              default:
                  curve_ = release_curve_;
                  break;
              }
            }

            inline void updateIncrements(void) {
                increment_[ENV_SEGMENT_ATTACK] = getIncrement(attack_time_, attack_);
                increment_[ENV_SEGMENT_RELEASE] = getIncrement(release_time_, release_);
            }

            inline uint32_t getIncrement(SegmentTime time, int16_t a) {
                if (a < 0) a = 0;

                a = a >> 8;

                switch(time) {
                  case S_10_SECONDS:
                    return lut_env_increments_10seconds[a];
                    break;
                  case S_5_SECONDS:
                    return lut_env_increments_5seconds[a];
                    break;
                  case S_2_SECONDS:
                    return lut_env_increments_2seconds[a];
                    break;
                  case S_1_SECONDS:
                    return lut_env_increments_1seconds[a];
                    break;
                  case S_HALF_SECOND:
                    return lut_env_increments_half_second[a];
                    break;
                  default:
                    return lut_env_increments_quarter_second[a];
                }
            }

            // Phase increments for each segment.
            uint32_t increment_[ENV_NUM_SEGMENTS];

            // Value that needs to be reached at the end of each segment.
            int16_t target_[ENV_NUM_SEGMENTS];

            // Current segment.
            size_t segment_ = ENV_SEGMENT_DEAD;

            // Start and end value of the current segment.
            uint16_t a_;
            uint16_t b_;
            int16_t value_;
            uint32_t phase_;

            bool hold_ = false;
            int16_t attack_ = 1000;
            int16_t release_ = 16000;
            SegmentTime attack_time_ = S_5_SECONDS;
            SegmentTime release_time_ = S_10_SECONDS;

            const uint16_t * attack_curve_ = lut_env_log;
            const uint16_t * release_curve_ = lut_env_expo;
            const uint16_t * curve_ = lut_env_log;
        };

    }
}