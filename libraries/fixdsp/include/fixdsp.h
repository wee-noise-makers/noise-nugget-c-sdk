#pragma once

#include <limits>
#include <iostream>

#include "audio_buffer.h"

#ifndef FIXDSP_BUFFER_LEN
#define FIXDSP_BUFFER_LEN 64
#endif

#ifndef FIXDSP_SAMPLE_RATE
#define FIXDSP_SAMPLE_RATE 44100
#endif

#include <array>

#define MAX_PARAM std::numeric_limits<int16_t>::max()

namespace fixdsp {

    using WaveformData = std::array<int16_t, 257>;

    /* using Q16 = fpm::fixed<std::int16_t, std::int32_t, 15>; */
    /* using Q17_15 = fpm::fixed<std::int32_t, std::int64_t, 15>; */
    using MonoSample = int16_t;

    class MonoBuffer : public AudioBuffer<MonoSample, 1, FIXDSP_BUFFER_LEN> {};
    class StereoBuffer : public AudioBuffer<MonoSample, 2, FIXDSP_BUFFER_LEN> {};

    typedef struct {
        MonoSample l;
        MonoSample r;
    } StereoSample;

    static_assert(sizeof(StereoSample) == 4, "must be 4 bytes");

    class InterleavedStereoBuffer : public AudioBuffer<StereoSample, 1, FIXDSP_BUFFER_LEN> {};

    class PhaseBuffer : public AudioBuffer<uint32_t, 1, FIXDSP_BUFFER_LEN> {};

    inline int16_t keyToPitch(uint8_t key);

    inline int16_t Interpolate824(const int16_t* table, uint32_t phase)
      __attribute__((always_inline));

    inline uint16_t Interpolate824(const uint16_t* table, uint32_t phase)
      __attribute__((always_inline));

    inline int16_t Interpolate824(const uint8_t* table, uint32_t phase)
      __attribute__((always_inline));

    inline uint16_t Interpolate88(const uint16_t* table, uint16_t index)
      __attribute__((always_inline));

    inline int16_t Interpolate88(const int16_t* table, uint16_t index)
      __attribute__((always_inline));

    inline int16_t Interpolate1022(const int16_t* table, uint32_t phase)
      __attribute__((always_inline));

    inline int16_t Interpolate115(const int16_t* table, uint32_t phase)
      __attribute__((always_inline));

    inline void Interpolate824(const int16_t* table, const PhaseBuffer &phase, MonoBuffer &output)
      __attribute__((always_inline));

    inline int16_t Crossfade(
        const int16_t* table_a,
        const int16_t* table_b,
        uint32_t phase,
        uint16_t balance)
      __attribute__((always_inline));

    inline int16_t Crossfade(
        const uint8_t* table_a,
        const uint8_t* table_b,
        uint32_t phase,
        uint16_t balance)
      __attribute__((always_inline));

    inline int16_t Crossfade1022(
        const uint8_t* table_a,
        const uint8_t* table_b,
        uint32_t phase,
        uint16_t balance)
      __attribute__((always_inline));

    inline int16_t Crossfade115(
        const uint8_t* table_a,
        const uint8_t* table_b,
        uint16_t phase,
        uint16_t balance)
      __attribute__((always_inline));

    inline int16_t mult(int16_t a, int16_t b) {
        const int16_t res = (a * static_cast<int32_t>(b)) >> 15;
        return res;
    }

    inline void modulate(MonoBuffer &in, const MonoBuffer &mod) {
        auto in_p = in.getWritePointer(0);
        auto mod_p = mod.getReadPointer(0);

        for (int i = 0; i < in.getBufferLength(); i++) {
            in_p[i] = in_p[i] * static_cast<int32_t>(mod_p[i]) >> 15;
         }
    }

    inline void modulate(MonoBuffer &in, uint16_t mod) {
        auto in_p = in.getWritePointer(0);

        for (int i = 0; i < in.getBufferLength(); i++) {
            in_p[i] = in_p[i] * static_cast<int32_t>(mod) >> 15;
         }
    }

    inline int16_t clip(int32_t a) {
      if (a > std::numeric_limits<int16_t>::max()) {
        return std::numeric_limits<int16_t>::max();
      } else if (a < std::numeric_limits<int16_t>::min()) {
        return std::numeric_limits<int16_t>::min();
      } else {
        return static_cast<int16_t>(a);
      }
    }

    inline void addSat(MonoBuffer &a, const MonoBuffer &b) {
        auto a_p = a.getWritePointer(0);
        auto b_p = b.getReadPointer(0);

        for (int i = 0; i < a.getBufferLength(); i++) {
            int32_t sa = static_cast<int32_t>(a_p[i]);
            int32_t sb = static_cast<int32_t>(b_p[i]);
            a_p[i] = clip (sa + sb);
         }
    }

    template<typename... Buffers>
    inline void addSat(MonoBuffer &output, const Buffers&... s) {
      const auto len = output.getBufferLength();

      for (int i = 0; i < len; i++){
          int32_t sum = (s.getReadPointer(0)[i] + ...);
          output.getWritePointer(0)[i] = fixdsp::clip(sum);
      }
    }

    template<typename... Buffers>
    inline void addScale(MonoBuffer &output, const Buffers&... s) {
        const auto len = output.getBufferLength();
        const int32_t chan_count = sizeof...(s);

        for (int i = 0; i < len; i++){
            int32_t sum = (s.getReadPointer(0)[i] + ...) / chan_count;
            output.getWritePointer(0)[i] = sum;
        }
    }

    inline int16_t Mix(int16_t a, int16_t b, uint16_t balance) {
      return (a * (65535 - balance) + b * balance) >> 16;
    }

    inline uint16_t Mix(uint16_t a, uint16_t b, uint16_t balance) {
      return (a * (65535 - balance) + b * balance) >> 16;
    }

    inline uint32_t Mix(uint32_t a, uint32_t b, int16_t balance) {
        const uint64_t a64 = static_cast<uint64_t>(a);
        const uint64_t b64 = static_cast<uint64_t>(b);
        const uint64_t balance64 = static_cast<uint64_t>(balance);

      return static_cast<uint64_t>((a64 * (MAX_PARAM - balance64) + b64 * balance64) >> 15);
    }

    inline int16_t keyToPitch(uint8_t key) {
        return ((int16_t) key) * 128;
    }

    inline int16_t Interpolate824(const int16_t* table, uint32_t phase) {
      int32_t a = table[phase >> 24];
      int32_t b = table[(phase >> 24) + 1];
      return a + ((b - a) * static_cast<int32_t>((phase >> 8) & 0xffff) >> 16);
    }

    inline uint16_t Interpolate824(const uint16_t* table, uint32_t phase) {
      uint32_t a = table[phase >> 24];
      uint32_t b = table[(phase >> 24) + 1];
      return a + ((b - a) * static_cast<uint32_t>((phase >> 8) & 0xffff) >> 16);
    }

    inline int16_t Interpolate824(const uint8_t* table, uint32_t phase) {
      int32_t a = table[phase >> 24];
      int32_t b = table[(phase >> 24) + 1];
      return (a << 8) + \
          ((b - a) * static_cast<int32_t>(phase & 0xffffff) >> 16) - 32768;
    }

    inline uint16_t Interpolate88(const uint16_t* table, uint16_t index) {
      int32_t a = table[index >> 8];
      int32_t b = table[(index >> 8) + 1];
      return a + ((b - a) * static_cast<int32_t>(index & 0xff) >> 8);
    }

    inline int16_t Interpolate88(const int16_t* table, uint16_t index) {
      int32_t a = table[index >> 8];
      int32_t b = table[(index >> 8) + 1];
      return a + ((b - a) * static_cast<int32_t>(index & 0xff) >> 8);
    }

    inline int16_t Interpolate1022(const int16_t* table, uint32_t phase) {
      int32_t a = table[phase >> 22];
      int32_t b = table[(phase >> 22) + 1];
      return a + ((b - a) * static_cast<int32_t>((phase >> 6) & 0xffff) >> 16);
    }

    inline int16_t Interpolate115(const int16_t* table, uint16_t phase) {
      int32_t a = table[phase >> 5];
      int32_t b = table[(phase >> 5) + 1];
      return a + ((b - a) * static_cast<int32_t>(phase & 0x1f) >> 5);
    }

    inline void Interpolate824(const int16_t* table, const PhaseBuffer &phase, MonoBuffer &output) {
      auto phase_p = phase.getReadPointer(0);
      auto out_p = output.getWritePointer(0);
      auto len = phase.getBufferLength();

      for (int i = 0; i < len; i++) {
        out_p[i] = Interpolate824(table, phase_p[i]);
      }
    }

    inline int16_t Crossfade(
        const int16_t* table_a,
        const int16_t* table_b,
        uint32_t phase,
        uint16_t balance) {
      int32_t a = Interpolate824(table_a, phase);
      int32_t b = Interpolate824(table_b, phase);
      return a + ((b - a) * static_cast<int32_t>(balance) >> 16);
    }

    inline int16_t Crossfade(
        const uint8_t* table_a,
        const uint8_t* table_b,
        uint32_t phase,
        uint16_t balance) {
      int32_t a = Interpolate824(table_a, phase);
      int32_t b = Interpolate824(table_b, phase);
      return a + ((b - a) * static_cast<int32_t>(balance) >> 16);
    }

    inline int16_t Crossfade1022(
        const int16_t* table_a,
        const int16_t* table_b,
        uint32_t phase,
        uint16_t balance) {
      int32_t a = Interpolate1022(table_a, phase);
      int32_t b = Interpolate1022(table_b, phase);
      return a + ((b - a) * static_cast<int32_t>(balance) >> 16);
    }

    inline int16_t Crossfade115(
        const int16_t* table_a,
        const int16_t* table_b,
        uint16_t phase,
        uint16_t balance) {
      int32_t a = Interpolate115(table_a, phase);
      int32_t b = Interpolate115(table_b, phase);
      return a + ((b - a) * static_cast<int32_t>(balance) >> 16);
    }

}
