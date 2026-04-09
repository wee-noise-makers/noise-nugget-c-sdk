#pragma once

#include <string>
#include "braids/braids_main.h"
#include "midi.hpp"

class SynthControls {
    public:

    SynthControls(uint8_t chan)
      : chan_(chan)
    {
        sync();
    }

    void sync(void) {
        /* Set synth parameters */
        for (int i = 0; i < PARAM_COUNT; i++) {
            sendCC(static_cast<synth_param>(i), param_value_[i]);
        }
    }

    void valueNext(synth_param param) {
        const int id = static_cast<int>(param);

        if (param_value_[id] < param_max_value[id]) {
            param_value_[id]++;
            sendCC(param, param_value_[id], true);
        }
    }

    void valuePrev(synth_param param) {
        const int id = static_cast<int>(param);

        if (param_value_[id] > 0) {
            param_value_[id]--;
            sendCC(param, param_value_[id], true);
        }
    }

    std::string paramImg(synth_param param) {
        const int id = static_cast<int>(param);
        return param_name[id];
    }

    std::string valueImg(synth_param param) {
        const int id = static_cast<int>(param);
        if (param == synth_param::Shape) {
            return shape_name[param_value_[id]];
        } else {
            return std::to_string(param_value_[id]);
        }
    }

    void sendCC(synth_param param, uint8_t value, bool preview = false) {
        sendSynth(makeCC(chan_, static_cast<uint8_t>(param), value));

        if (preview) {
            sendSynth(makeNoteOn(chan_, 48, 127));
            sendSynth(makeNoteOff(chan_, 48, 127));
        }
    }

    private:
    uint8_t chan_;
    uint8_t param_value_[PARAM_COUNT] = {0, 127 / 2, 0, 127 / 2, 0, 1, 6, 127};
};
