#pragma once

#include "fixdsp.h"

namespace fixdsp {
    namespace filter {

        enum SvfMode {
          SVF_MODE_LP,
          SVF_MODE_BP,
          SVF_MODE_HP
        };

        class SVF
        {
        public:
            SVF() {};
            ~SVF() {};

            void setCutoff(int16_t frequency);
            void setResonance(int16_t resonance);
            void setMode(SvfMode mode);
            void process(MonoBuffer &buffer);

        private:
            bool dirty_ = true;
  
            int16_t frequency_ = 33 << 7;
            int16_t resonance_ = 16384;
            
            int32_t f_;
            int32_t damp_;
          
            int32_t lp_ = 0;
            int32_t bp_ = 0;
            
            SvfMode mode_ = SVF_MODE_LP;
        };        
    }
}