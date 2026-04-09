#pragma once

#include <stdint.h>

#ifdef __cplusplus

#define kSensorsLaneCount 15

struct SensorsStatus{
    bool paperDetected;
    bool holeDetected[kSensorsLaneCount];
};

uint16_t getRawStatus(void);
SensorsStatus getStatus(void);
#endif

#ifdef __cplusplus
extern "C" {
#endif

void sensors_init(void);

#ifdef __cplusplus
}
#endif
