#include <string>
#include <bitset>

#include "gui.hpp"
#include "pgb1.h"
#include "encoder.hpp"
#include "sequencer.hpp"
#include "views.hpp"

encoder::Encoder encA(pio0, 0, {26, 27});
encoder::Encoder encB(pio0, 1, {23, 24});
bool prev_a_pressed = false;
bool prev_b_pressed = false;

#define ENC_A_SW_PIN 25
#define ENC_B_SW_PIN 22

MainView mainView(getSequencer());

extern "C" void gui_init(void)
{
    screen_init();
    encA.init();
    encB.init();

    gpio_init(ENC_A_SW_PIN);
    gpio_set_dir(ENC_A_SW_PIN, GPIO_IN);
    gpio_pull_up(ENC_A_SW_PIN);

    gpio_init(ENC_B_SW_PIN);
    gpio_set_dir(ENC_B_SW_PIN, GPIO_IN);
    gpio_pull_up(ENC_B_SW_PIN);

    mainView.setBounds(screenBounds);
}

extern "C" void gui_update(void)
{
    const auto captA = encA.capture();
    const auto captB = encB.capture();
    const auto a_pressed = !gpio_get(ENC_A_SW_PIN);
    const auto b_pressed = !gpio_get(ENC_B_SW_PIN);

    screen_clear();
    //drawRect(screenBounds);

    if (captA.delta() > 0) {
        for (int i = 0; i < captA.delta(); i++) {
            mainView.event({GUIinput::Enc_A_Right});
        }
    } else if (captA.delta() < 0) {
        for (int i = 0; i < -captA.delta(); i++) {
            mainView.event({GUIinput::Enc_A_Left});
        }
    }
    if (captB.delta() > 0) {
        for (int i = 0; i < captB.delta(); i++) {
            mainView.event({GUIinput::Enc_B_Right});
        }
    } else if (captB.delta() < 0) {
        for (int i = 0; i < -captB.delta(); i++) {
            mainView.event({GUIinput::Enc_B_Left});
        }
    }
    if (a_pressed && !prev_a_pressed) {
        mainView.event({GUIinput::Button_A});
    }
    if (b_pressed && !prev_b_pressed) {
        mainView.event({GUIinput::Button_B});
    }

    // screen_print(0,42, std::to_string(captA.delta()).c_str());
    // screen_print(0, 50, std::to_string(captB.delta()).c_str());
    // screen_print(0, 20, std::to_string(gpio_get(ENC_A_SW_PIN)).c_str());
    // screen_print(0, 30, std::to_string(gpio_get(ENC_B_SW_PIN)).c_str());

    // const auto status = getlastStatus();
    // if (status.paperDetected) {
    //     screen_print(0, 40, "Got a paper!");
    // }

    // for (int i = 0; i < kSensorsLaneCount; i++) {
    //     if (status.holeDetected[i]) {
    //         screen_print(i * 8, 50, "*");
    //     } else {
    //         screen_print(i * 8, 50, ".");
    //     }
    // }

    mainView.draw();

    screen_update();

    prev_a_pressed = a_pressed;
    prev_b_pressed = b_pressed;
}

