#include <string>
#include "sensors.hpp"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define ENA_BASE_PIN 18
#define ENA_PIN_COUNT 4

#define SENSE_BASE_PIN 14
#define SENSE_PIN_COUNT 4

extern "C" void sensors_init(void)
{
    for (int i = 0; i < SENSE_PIN_COUNT; i++) {
        const auto pin = SENSE_BASE_PIN + i;
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_disable_pulls(pin);
    }
    for (int i = 0; i < ENA_PIN_COUNT; i++) {
        const auto pin = ENA_BASE_PIN + i;
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_disable_pulls(pin);
        gpio_put(pin, false);
    }
}

uint16_t getRawStatus(void) {
    uint16_t ret = 0;
    uint16_t mask = 1;

    for (int i = 0; i < ENA_PIN_COUNT; i++) {

        const auto ena_pin = ENA_BASE_PIN + i;
        gpio_put(ena_pin, true);
        sleep_ms(5);
        for (int j = 0; j < SENSE_PIN_COUNT; j++) {
            const auto sense_pin = SENSE_BASE_PIN + j;
            if (gpio_get(sense_pin)) {
                ret |= mask;
            }
            mask <<= 1;
        }

        gpio_put(ena_pin, false);
    }

    return ret;
}

SensorsStatus getStatus(void) {
    SensorsStatus ret;

    const auto raw = getRawStatus();

    ret.paperDetected = ! (raw & 0b0000000000001000);

    ret.holeDetected[0]  = raw & 0b0000000000000100;
    ret.holeDetected[1]  = raw & 0b0000000000000010;
    ret.holeDetected[2]  = raw & 0b0000000000000001;
    ret.holeDetected[3]  = raw & 0b0000000010000000;
    ret.holeDetected[4]  = raw & 0b0000000001000000;
    ret.holeDetected[5]  = raw & 0b0000000000100000;
    ret.holeDetected[6]  = raw & 0b0000000000010000;
    ret.holeDetected[7]  = raw & 0b0000100000000000;
    ret.holeDetected[8]  = raw & 0b0000010000000000;
    ret.holeDetected[9]  = raw & 0b0000001000000000;
    ret.holeDetected[10] = raw & 0b0000000100000000;
    ret.holeDetected[11] = raw & 0b1000000000000000;
    ret.holeDetected[12] = raw & 0b0100000000000000;
    ret.holeDetected[13] = raw & 0b0010000000000000;
    ret.holeDetected[14] = raw & 0b0001000000000000;
    return ret;
}