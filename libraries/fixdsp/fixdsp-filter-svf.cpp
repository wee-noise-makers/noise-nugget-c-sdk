#include "filter_svf.h"
#include "resources.h"

#define CLIP(x) if (x < -32767) x = -32767; if (x > 32767) x = 32767

namespace fixdsp {
    namespace filter {

        void SVF::setCutoff(int16_t frequency) {
          if (frequency < 0) {
            frequency = 0;
          }
          dirty_ = dirty_ || (frequency_ != frequency);
          frequency_ = frequency;
        }

        void SVF::setResonance(int16_t resonance) {
          if (resonance < 0) {
            resonance = 0;
          }
          resonance_ = resonance;
          dirty_ = true;
        }

        void SVF::setMode(SvfMode mode) {
          mode_ = mode;
        }

        void SVF::process(MonoBuffer &buffer) {

            if (dirty_) {
              f_ = Interpolate824(lut_svf_cutoff, frequency_ << 17);
              damp_ = Interpolate824(lut_svf_damp, resonance_ << 17);
              dirty_ = false;
            }
            int32_t f = f_;
            int32_t damp = damp_;

            for (auto &sample : buffer.getBufferContainer()[0]) {
                int32_t notch = sample - (bp_ * damp >> 15);
                lp_ += f * bp_ >> 15;
                CLIP(lp_);
                int32_t hp = notch - lp_;
                bp_ += f * hp >> 15;
                CLIP(bp_);
                sample = mode_ == SVF_MODE_BP ? bp_ : (mode_ == SVF_MODE_HP ? hp : lp_);
            }
        }
    }
}