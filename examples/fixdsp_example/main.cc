#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "noise_nugget.h"
#include "midi_utils.h"
#include "nugget_midi_synth.h"

#include <iostream>

#include "fixdsp.h"
#include "waveform_oscillator.h"
#include "phase_distortion_oscillator.h"
#include "phase.h"
#include "resources.h"
#include "filter_svf.h"
#include "envelope-ar.h"
#include "drum-waveform_kick.h"

#if 1
#define printf(fmt, ...) (0)
#endif

enum VoiceEngine {
  PD_SQ_Sin_Half,
  PD_SQ_Sin_Full,
  PD_Trig_Sin_Half,
  PD_Trig_Sin_Full,
  PD_Sin_SQ_Full,
  PD_Lookup_Trig_Wrap,
  LAST_VOICE_ENGINE
};

typedef struct VoiceParams {
    int16_t cutoff;
    int16_t amount;
    int16_t shape_attack;
    int16_t shape_release;
    int16_t attack;
    int16_t release;
} VoiceParams;

class DemoVoice {
public:
    DemoVoice() {
        amp_env_.setHold(true);
        amp_env_.setAttackTimeRange(fixdsp::envelope::S_5_SECONDS);
        amp_env_.setReleaseTimeRange(fixdsp::envelope::S_10_SECONDS);

        shape_env_.setHold(true);
    }

    uint8_t playing(void) {
        return playing_key_;
    }

    void on(uint8_t key, int16_t velocity) {
        phase_gen_.setKey(key);
        shape_env_.on(velocity);
        amp_env_.on(velocity);
        playing_key_ = key;
    }

    void off(void) {
        playing_key_ = 0;
        shape_env_.off();
        amp_env_.off();
    }

    void renderPhaseDistoHalf(fixdsp::MonoBuffer &buffer,
                              const fixdsp::WaveformData &wave,
                              fixdsp::PhaseBuffer &phase_buf,
                              const fixdsp::MonoBuffer &shape_buf)
    {
        fixdsp::MonoBuffer attenuation;

        fixdsp::phase::distortion::resonantSecondHalf(phase_buf, shape_buf, attenuation);
        fixdsp::Interpolate824(wave.data(), phase_buf, buffer);
        fixdsp::modulate(buffer, attenuation);

    }

    void renderPhaseLookup(fixdsp::MonoBuffer &buffer,
                           const fixdsp::WaveformData &wave,
                           const fixdsp::WaveformData &lookup,
                           fixdsp::PhaseBuffer &phase_buf,
                           const fixdsp::MonoBuffer &shape_buf)
    {
        fixdsp::phase::distortion::lookup(phase_buf, lookup, shape_buf);
        fixdsp::Interpolate824(wave.data(), phase_buf, buffer);
    }

    void renderPhaseDistoFull(fixdsp::MonoBuffer &buffer,
                              const fixdsp::WaveformData &wave,
                              fixdsp::PhaseBuffer &phase_buf,
                              const fixdsp::MonoBuffer &shape_buf)
    {
        fixdsp::MonoBuffer attenuation;

        fixdsp::phase::distortion::resonantFull(phase_buf, shape_buf, attenuation);
        fixdsp::Interpolate824(wave.data(), phase_buf, buffer);
        fixdsp::modulate(buffer, attenuation);
    }

    void render(fixdsp::MonoBuffer &buffer, VoiceParams params, VoiceEngine engine, bool env_hold) {
        fixdsp::PhaseBuffer phase_buf;
        fixdsp::MonoBuffer env_buf;

        shape_env_.setHold(env_hold);
        shape_env_.setAttack(params.shape_attack);
        shape_env_.setRelease(params.shape_release);
        amp_env_.setAttack(params.attack);
        amp_env_.setRelease(params.release);

        phase_gen_.render(phase_buf);
        shape_env_.render(env_buf);

        fixdsp::modulate(env_buf, params.amount);

        switch (engine)
        {
        case PD_SQ_Sin_Full:
            renderPhaseDistoFull(buffer, fixdsp::wav_combined_square_sin, phase_buf, env_buf);
            break;

        case PD_SQ_Sin_Half:
            renderPhaseDistoHalf(buffer, fixdsp::wav_combined_square_full_sin, phase_buf, env_buf);
            break;

        case PD_Trig_Sin_Full:
            renderPhaseDistoFull(buffer, fixdsp::wav_combined_trig_sin, phase_buf, env_buf);
            break;


        case PD_Trig_Sin_Half:
            renderPhaseDistoHalf(buffer, fixdsp::wav_combined_trig_full_sin, phase_buf, env_buf);
            break;

        case PD_Sin_SQ_Full:
            renderPhaseDistoFull(buffer, fixdsp::wav_combined_sin_square, phase_buf, env_buf);
            break;

        case PD_Lookup_Trig_Wrap:
            renderPhaseLookup(buffer, fixdsp::wav_triangle, fixdsp::wav_sine2_warp3, phase_buf, env_buf);
            break;

        default:
            break;
        }

        amp_env_.render(env_buf);
        fixdsp::modulate(buffer, env_buf);
    }

private:
    fixdsp::phase::ConstantPitch phase_gen_;

    fixdsp::envelope::AR amp_env_;
    fixdsp::envelope::AR shape_env_;

    uint8_t playing_key_ = 0;
};

#define POLY_COUNT 8

volatile bool poly_on = false;
volatile bool env_hold = false;
volatile VoiceEngine engine = PD_SQ_Sin_Half;

DemoVoice voices[POLY_COUNT];
uint next_voice = 0;
VoiceParams v_params = {.amount = MAX_PARAM / 2,
                        .shape_release = MAX_PARAM / 3,
                        .attack = MAX_PARAM / 20,
                        .release = MAX_PARAM / 3};

uint8_t params[4] = {0, 0, 0, 0};

void render_audio(uint32_t *buffer, int len) {

    fixdsp::MonoBuffer mono_buffer[POLY_COUNT];
    fixdsp::MonoBuffer output;

    for (uint i = 0; i < POLY_COUNT; i++) {
        voices[i].render(mono_buffer[i], v_params, engine, env_hold);
    }

    if (poly_on) {
        fixdsp::addScale(output, mono_buffer[0],
                                 mono_buffer[1],
                                 mono_buffer[2],
                                 mono_buffer[3],
                                 mono_buffer[4],
                                 mono_buffer[5],
                                 mono_buffer[6],
                                 mono_buffer[7]);
    } else {
        fixdsp::addScale(output, mono_buffer[0]);
    }

    const auto mono_p = output.getReadPointer(0);
    for (int i = 0; i < len; i++) {
        int16_t* stereo_point = (int16_t*)&(buffer[i]);
        stereo_point[0] = mono_p[i];
        stereo_point[1] = mono_p[i];
    }

    return;
}

void note_on(uint8_t chan, uint8_t key, uint8_t velocity) {
    if (poly_on) {
        for (uint i = 0; i < POLY_COUNT && voices[next_voice].playing() != 0; i++) {
            next_voice = ++next_voice % POLY_COUNT;
        }

        printf("Note on! voice: %d\n", next_voice);
        voices[next_voice].on(key, static_cast<int16_t>(velocity) << 8);
        next_voice = ++next_voice % POLY_COUNT;
    } else {
        voices[0].on(key, static_cast<int16_t>(velocity) << 8);

        for (uint i = 1; i < POLY_COUNT; i++) {
            voices[i].off();
        }
    }

    return;
}

void note_off(uint8_t chan, uint8_t key, uint8_t velocity) {

    if (poly_on) {
        for (uint i = 0; i < POLY_COUNT; i++) {
            if (voices[i].playing() == key) {
                printf("Note off! voice: %d\n", i);
                voices[i].off();
                return;
            }
        }
    } else {
        if (voices[0].playing() == key) {
            voices[0].off();
        }
    }
}

void control_change(uint8_t chan, uint8_t controller, uint8_t value) {
    printf("Got CC %d -> %d\n", controller, value);

    const int16_t param = static_cast<int16_t>(value) << 8;
    switch (controller)
    {
    case 1:
        printf("Amount %d -> %d\n", param);
        v_params.amount = param;
        break;

    case 2:
        printf("Shape Release %d -> %d\n", param);
        v_params.shape_release = param;
        v_params.shape_attack = param / 2;
        break;

    case 3:
        printf("Attacj %d -> %d\n", param);
        v_params.attack = param;
        break;

    case 4:
        printf("Release %d -> %d\n", param);
        v_params.release = param;
        break;

    default:
        break;
    }
    return;
}

extern "C" {

uint8_t read_adc(uint id) {
    adc_select_input(id);
    const float conversion_factor = 3.3f / (1 << 12);
    const uint16_t result = adc_read();
    const uint16_t cc = result >> 5;
    //printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
    //printf("%d, ", cc);
    return cc;
}

#define MIDI_UART uart0
#define MIDI_IRQ UART0_IRQ
#define MIDI_OUT_PIN 12
#define MIDI_IN_PIN 13
#define MIDI_GPIO_FUNC GPIO_FUNC_UART

static midi_decoder demo_midi_decoder;

enum Buttons {
    BTN_VOL_UP, BTN_VOL_DOWN,

    BTN_ENG_UP, BTN_ENG_DOWN,

    BTN_COUNT
};

const uint btn_pin[BTN_COUNT] = {8, 9, 11, 10};
bool btn_last_state[BTN_COUNT] = {true, true, true, true};

float speaker_volume = 0.0;

#define POLY_SWITCH_PIN 4
#define ENV_HOLD_SWITCH_PIN 5

void demo_on_uart_rx() {
    while (uart_is_readable(MIDI_UART)) {
        uint8_t ch = uart_getc(MIDI_UART);
        printf("MIDI CHAR: 0x%x\n", ch);
        uint32_t msg = midi_decoder_push(&demo_midi_decoder, ch);
        if (msg != 0) {
            printf("MIDI MSG: 0x%x\n", msg);
            send_MIDI(msg);
        }
    }
}

void demo_midi_init(void) {

    uart_init(MIDI_UART, 31250);
    gpio_set_function(MIDI_IN_PIN, MIDI_GPIO_FUNC);
    gpio_set_function(MIDI_OUT_PIN, MIDI_GPIO_FUNC);
    uart_set_format(MIDI_UART, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(MIDI_UART, true);

    irq_set_exclusive_handler(MIDI_IRQ, demo_on_uart_rx);
    irq_set_enabled(MIDI_IRQ, true);
    uart_set_irq_enables(MIDI_UART, true, false);

    midi_decoder_init(&demo_midi_decoder);
}

int main(void)
{
    stdio_init_all();
    printf("Noise Nugget 2040 FixDSP example\n");

    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(28);
    adc_gpio_init(29);

    demo_midi_init();

    for (uint i = 0; i < BTN_COUNT; i++) {
        gpio_init(btn_pin[i]);
        gpio_set_dir(btn_pin[i], GPIO_IN);
        gpio_pull_up(btn_pin[i]);
    }

    gpio_init(POLY_SWITCH_PIN);
    gpio_set_dir(POLY_SWITCH_PIN, GPIO_IN);
    gpio_pull_up(POLY_SWITCH_PIN);

    gpio_init(ENV_HOLD_SWITCH_PIN);
    gpio_set_dir(ENV_HOLD_SWITCH_PIN, GPIO_IN);
    gpio_pull_up(ENV_HOLD_SWITCH_PIN);

    start_MIDI_Synth(FIXDSP_SAMPLE_RATE,
                     render_audio, note_on, note_off, control_change);

    if (!set_hp_volume(0.7, 0.7)) {
        printf("HP volume failed");
    }

    enable_speakers(true, true, 0);
    // enable_line_out(true, true);
    set_line_out_volume(0.0, 0.0, 0.0, 0.0);

    //send_note_on(0, 44, 127);
    while (1) {
        for (uint i = 0; i < 4; i++) {
            const uint8_t new_value = read_adc(i);
            if (new_value != params[i]) {
                params[i] = new_value;
                send_CC(0, i + 1, new_value);
            }
        }

        poly_on = gpio_get(POLY_SWITCH_PIN);
        env_hold = gpio_get(ENV_HOLD_SWITCH_PIN);

        for (uint i = 0; i < BTN_COUNT; i++) {
            const bool new_state = gpio_get(btn_pin[i]);

            if (!new_state && btn_last_state[i]) {
                printf("%d pressed\n", i);
                switch (i)
                {
                case BTN_VOL_UP:
                    if (speaker_volume <= 0.9) {
                        speaker_volume += 0.1;
                    }
                    printf("Speaker volume: %f\n", speaker_volume);
                    set_line_out_volume(speaker_volume, 0.0, 0.0, speaker_volume);
                    break;

                case BTN_VOL_DOWN:
                    if (speaker_volume > 0.1) {
                        speaker_volume -= 0.1;
                    }
                    printf("Speaker volume: %f\n", speaker_volume);
                    set_line_out_volume(speaker_volume, 0.0, 0.0, speaker_volume);
                    break;

                case BTN_ENG_UP:
                    if (engine < LAST_VOICE_ENGINE - 1) {
                        engine = (enum VoiceEngine)((int)engine + 1);
                    }
                    printf("Next engine: %d\n", engine);
                    break;

                case BTN_ENG_DOWN:
                    if (engine > 0) {
                        engine = (enum VoiceEngine)((int)engine - 1);
                    }
                    printf("Prev engine: %d\n", engine);
                    break;

                default:
                    break;
                }
            }
            btn_last_state[i] = new_state;
        }

        sleep_ms(10);
    }

}

}