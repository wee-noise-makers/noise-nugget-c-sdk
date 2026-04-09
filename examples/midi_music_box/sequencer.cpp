#include "sequencer.hpp"

PaperSequencer gSeq;

SensorsStatus getlastStatus(void) {
    return gSeq.lastStatus_;
}

PaperSequencer& getSequencer(void) {
    return gSeq;
}

extern "C" void seq_init(void)
{
    sensors_init();
}

extern "C" void seq_update(void)
{
    gSeq.update();
}
