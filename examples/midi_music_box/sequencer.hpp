#pragma once

#include "sensors.hpp"
#include "midi.hpp"

#ifdef __cplusplus

#include <array>

class SeqLane {
    public:
    SeqLane()
    {
    }

    bool isOn() {
        return lastHoldDetected_;
    }

    void setKey(int key) {
        noteOff();
        key_ = key;
    }

    int getKey(void) {
        return key_;
    }

    void nextKey(void) {
        if (key_ < 127) {
            noteOff();
            key_++;
        }
    }

    void prevKey(void) {
        if (key_ > 0) {
            noteOff();
            key_--;
        }
    }

    void setChan(int chan) {
        noteOff();
        chan_ = chan;
    }
    uint8_t getChan(void) {
        return chan_;
    }
    void nextChan(void) {
        if (chan_ < 15) {
            noteOff();
            chan_++;
        }
    }
    void prevChan(void) {
        if (chan_ > 0) {
            noteOff();
            chan_--;
        }
    }

    void setVelocity(int velocity) {
        velocity_ = velocity;
    }
    uint8_t getVelocity(void) {
        return velocity_;
    }
    void nextVelocity(void) {
        if (velocity_ < 127) {
            velocity_++;
        }
    }
    void prevVelocity(void) {
        if (velocity_ > 0) {
            velocity_--;
        }
    }

    void noteOn(void) {
        sendSynth(makeNoteOn(chan_, key_, velocity_));
        noteIsOn_ = true;
    }
    void noteOff(){
        if (noteIsOn_) {
           sendSynth(makeNoteOff(chan_, key_, velocity_));
            noteIsOn_ = false;
        }
    }
    void update(bool holeDetected) {

        if (holeDetected && !lastHoldDetected_) {
            // Start note
            noteOn();
        } else if (!holeDetected && lastHoldDetected_) {
            // End note
            noteOff();
        }

        lastHoldDetected_ = holeDetected;
    }

    private:
    bool lastHoldDetected_ = false;
    bool noteIsOn_ = false;
    uint8_t key_ = 70;
    uint8_t velocity_ = 127;
    uint8_t chan_ = 0;
};

class PaperSequencer {
    public:

    PaperSequencer() {
        lanes[14].setKey(36 + 12);
        lanes[13].setKey(38 + 12);
        lanes[12].setKey(40 + 12);
        lanes[11].setKey(41 + 12);
        lanes[10].setKey(43 + 12);
        lanes[9].setKey (45 + 12);
        lanes[8].setKey (47 + 12);
        lanes[7].setKey (48 + 12);
        lanes[6].setKey (50 + 12);
        lanes[5].setKey (52 + 12);
        lanes[4].setKey (53 + 12);
        lanes[3].setKey (55 + 12);
        lanes[2].setKey (57 + 12);
        lanes[1].setKey (59 + 12);
        lanes[0].setKey (60 + 12);
    }

    void update(void) {
        lastStatus_ = getStatus();

        if (lastStatus_.paperDetected) {
            for (int i = 0; i < lanes.size(); i++) {
                lanes[i].update(lastStatus_.holeDetected[i]);
            }
        } else {
            for (auto& l : lanes) {
                l.update(false);
            }
        }
    }

    std::array<SeqLane, kSensorsLaneCount> lanes;
    SensorsStatus lastStatus_;
};

SensorsStatus getlastStatus(void);
PaperSequencer& getSequencer(void);

extern "C" {

#endif

void seq_init(void);
void seq_update(void);

#ifdef __cplusplus
}
#endif