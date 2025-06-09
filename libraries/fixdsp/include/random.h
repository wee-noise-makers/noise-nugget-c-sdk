#pragma once

#include <cstdint>
#include "fixdsp.h"

namespace fixdsp {
    class Random {
    public:
        inline uint32_t state() { return rng_state_; }

        inline void Seed(uint32_t seed) {
          rng_state_ = seed;
        }
      
        inline uint32_t GetWord() {
          rng_state_ = rng_state_ * 1664525L + 1013904223L;
          return state();
        }
        
        inline int16_t GetSample() {
          return static_cast<int16_t>(GetWord() >> 16);
        }
      
        inline float GetFloat() {
          return static_cast<float>(GetWord()) / 4294967296.0f;
        }
      
        inline void render(MonoBuffer &buffer) {
            for (auto& sample : buffer.getBufferContainer()[0]){
                sample = GetSample();
            }
        }
    private:
        uint32_t rng_state_;
    };
}