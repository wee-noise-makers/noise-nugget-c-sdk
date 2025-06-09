/*
 * Copyright (c) 2024 Fabien Chouteau @ Wee Noise Makers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "duplex_i2s.pio.h"
#include "noise_nugget.h"
#include "aic3105_reg_def.h"
#define I2S_PIO pio1
#define I2S_SM 0
#define I2S_PIN_FUNC GPIO_FUNC_PIO1

#define I2S_IN_PIN 0
#define I2S_OUT_PIN 1
#define I2S_LRCLK_PIN 2
#define I2S_BCLK_PIN 3

#define AIC3105_ADDR 0x18
#define TCA6408_ADDR 0x20

#define I2C_PORT i2c1
#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7

#define IO_EXP_SPK_Enable_L_Mask  0b00000001
#define IO_EXP_SPK_Enable_R_Mask  0b00000010
#define IO_EXP_SPK_Gain_0_Mask    0b00000100
#define IO_EXP_SPK_Gain_1_Mask    0b00001000
#define IO_EXP_DAC_Not_Reset_Mask 0b00010000
#define IO_EXP_Jack_Detect_Mask   0b00100000

#define IO_EXP_INPUT_REG 0
#define IO_EXP_OUTPUT_REG 1
#define IO_EXP_CONFIG_REG 3

#define IO_EXP_CONFIG_REG_INIT IO_EXP_Jack_Detect_Mask
#define IO_EXP_OUTPUT_REG_INIT 0b00000000

static uint8_t g_io_exp_output_reg_state = IO_EXP_OUTPUT_REG_INIT;

static bool g_io_exp_init = false;
static bool g_i2c_init = false;

int i2s_out_dma_chan = -1; // init with invalid DMA channel id
int i2s_in_dma_chan = -1; // init with invalid DMA channel id

#define DUMMY_AUDIO_BUFFER_SIZE 256
const uint32_t zeroes_audio_buffer[DUMMY_AUDIO_BUFFER_SIZE] = {0x0};
uint32_t dev_null_audio_buffer[DUMMY_AUDIO_BUFFER_SIZE] = {0x0};

audio_cb_t user_audio_input_callback = NULL;
audio_cb_t user_audio_output_callback = NULL;

void dma_out_handler() {
    uint32_t *buffer = NULL;
    uint32_t point_count = 0;

    // Clear the interrupt request.
    dma_hw->ints0 = 1u << i2s_out_dma_chan;

    // Call user callback, if any
    if (user_audio_output_callback != NULL) {
        user_audio_output_callback (&buffer, &point_count);
    }

    // No user callback or user returned NULL
    if (buffer == NULL) {
        buffer = (void*)zeroes_audio_buffer;
        point_count = DUMMY_AUDIO_BUFFER_SIZE;
    }

    dma_channel_transfer_from_buffer_now(i2s_out_dma_chan,
                                         buffer,
                                         point_count);
}

void dma_in_handler() {
    uint32_t *buffer = NULL;
    uint32_t point_count = 0;

    // Clear the interrupt request.
    dma_hw->ints1 = 1u << i2s_in_dma_chan;

    // Call user callback, if any
    if (user_audio_input_callback != NULL) {
        user_audio_input_callback (&buffer, &point_count);
    }

    // No user callback or user returned NULL
    if (buffer == NULL) {
        buffer = (void*)dev_null_audio_buffer;
        point_count = DUMMY_AUDIO_BUFFER_SIZE;
    }

    dma_channel_transfer_to_buffer_now(i2s_in_dma_chan,
                                       buffer,
                                       point_count);
}

void setup_audio_pin(int pin, bool out) {
    gpio_init(pin);
    gpio_set_dir(pin, out ? GPIO_OUT : GPIO_IN);
    gpio_pull_up(pin);
    gpio_pull_down(pin);
    gpio_set_function(pin, I2S_PIN_FUNC);
}

bool init_i2s(int sample_rate) {

    // PIO I2S Pins

    setup_audio_pin(I2S_IN_PIN, false);
    setup_audio_pin(I2S_OUT_PIN, true);
    setup_audio_pin(I2S_LRCLK_PIN, true);
    setup_audio_pin(I2S_BCLK_PIN, true);

    const int sample_bits = 16;
    const int channels = 2;
    const int cycles_per_sample = 4;

    // PIO I2S Program

    uint offset = pio_add_program(I2S_PIO, &audio_i2s_program);
    pio_sm_config c = audio_i2s_program_get_default_config(offset);
    sm_config_set_out_pins(&c, I2S_OUT_PIN, 1);
    sm_config_set_in_pins(&c, I2S_IN_PIN);
    sm_config_set_sideset_pins(&c, I2S_LRCLK_PIN);
    sm_config_set_out_shift(&c, false, true, sample_bits * channels);
    sm_config_set_in_shift(&c, false, true, sample_bits * channels);

    /* ALREADY IN audio_i2s default config: */
    /* sm_config_set_sideset(&c, 2, false, false); */
    /* sm_config_set_wrap */

    pio_sm_claim(I2S_PIO, I2S_SM);
    pio_sm_set_config(I2S_PIO, I2S_SM, &c);
    pio_sm_init (I2S_PIO, I2S_SM, offset, &c);

    pio_sm_set_consecutive_pindirs(I2S_PIO, I2S_SM, I2S_IN_PIN, 1, false);
    pio_sm_set_consecutive_pindirs(I2S_PIO, I2S_SM, I2S_OUT_PIN, 1, true);
    pio_sm_set_consecutive_pindirs(I2S_PIO, I2S_SM, I2S_LRCLK_PIN, 1, true);
    pio_sm_set_consecutive_pindirs(I2S_PIO, I2S_SM, I2S_BCLK_PIN, 1, true);

    pio_sm_exec(I2S_PIO, I2S_SM, pio_encode_jmp(offset + audio_i2s_offset_entry_point));

    const int pio_frequency = sample_rate * sample_bits * channels * cycles_per_sample;
    sm_config_set_clkdiv(&c, (float)clock_get_hz(clk_sys) / (float)pio_frequency);
    pio_sm_set_config(I2S_PIO, I2S_SM, &c);
    pio_sm_set_enabled(I2S_PIO, I2S_SM, true);

    i2s_out_dma_chan = dma_claim_unused_channel(true);
    dma_channel_config chan = dma_channel_get_default_config(i2s_out_dma_chan);
    channel_config_set_transfer_data_size(&chan, DMA_SIZE_32);
    channel_config_set_dreq(&chan, pio_get_dreq(I2S_PIO, I2S_SM, true));
    channel_config_set_read_increment(&chan, true);
    channel_config_set_write_increment(&chan, false);
    dma_channel_set_config(i2s_out_dma_chan, &chan, false);
    dma_channel_set_write_addr(i2s_out_dma_chan, &I2S_PIO->txf[I2S_SM],
                               false);

    dma_channel_set_irq0_enabled(i2s_out_dma_chan, true);

    irq_set_exclusive_handler(DMA_IRQ_0, dma_out_handler);
    dma_out_handler();
    dma_irqn_set_channel_enabled (DMA_IRQ_0, i2s_out_dma_chan, true);
    irq_set_enabled(DMA_IRQ_0, true);



    i2s_in_dma_chan = dma_claim_unused_channel(true);
    chan = dma_channel_get_default_config(i2s_in_dma_chan);
    channel_config_set_transfer_data_size(&chan, DMA_SIZE_32);
    channel_config_set_dreq(&chan, pio_get_dreq(I2S_PIO, I2S_SM, false));
    channel_config_set_read_increment(&chan, false);
    channel_config_set_write_increment(&chan, true);
    dma_channel_set_config(i2s_in_dma_chan, &chan, false);
    dma_channel_set_read_addr(i2s_in_dma_chan, &I2S_PIO->rxf[I2S_SM],
                              false);

    dma_channel_set_irq1_enabled(i2s_in_dma_chan, true);

    irq_set_exclusive_handler(DMA_IRQ_1, dma_in_handler);
    dma_in_handler();
    dma_irqn_set_channel_enabled (DMA_IRQ_1, i2s_in_dma_chan, true);
    irq_set_enabled(DMA_IRQ_1, true);

    return true;
}

bool nn_i2c_init(void){
    if (g_i2c_init) {
        return true;
    }

    gpio_init(I2C_SCL_PIN);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SCL_PIN);

    gpio_init(I2C_SDA_PIN);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);

    i2c_init(I2C_PORT, 100 * 1000);

    g_i2c_init = true;
    return true;
}

bool tca6408_write_register(uint8_t reg, uint8_t value){
    uint8_t buf[2] = {reg, value};

    const int result = i2c_write_blocking(I2C_PORT, TCA6408_ADDR, buf, 2, false);
    return result == 2;
}

bool io_exp_init(void){
    bool success = true;

    if (g_io_exp_init) {
        return true;
    }

    nn_i2c_init();

    success &= tca6408_write_register(IO_EXP_OUTPUT_REG, g_io_exp_output_reg_state);
    success &= tca6408_write_register(IO_EXP_CONFIG_REG, IO_EXP_CONFIG_REG_INIT);

    g_io_exp_init = success;

    return success;
}

bool io_exp_set_out(uint8_t new_state) {

    io_exp_init();

    if (tca6408_write_register(IO_EXP_OUTPUT_REG, new_state)) {
        g_io_exp_output_reg_state = new_state;
        return true;
    } else {
        return false;
    }
}

bool io_exp_enable_speakers(bool left, bool right) {
    uint8_t new_state = g_io_exp_output_reg_state;

    if (left) {
        new_state |= IO_EXP_SPK_Enable_L_Mask;
    } else {
        new_state &= ~IO_EXP_SPK_Enable_L_Mask;
    }

    if (right) {
        new_state |= IO_EXP_SPK_Enable_R_Mask;
    } else {
        new_state &= ~IO_EXP_SPK_Enable_R_Mask;
    }

    return io_exp_set_out(new_state);
}

bool set_speaker_gain(bool G0, bool G1) {
    uint8_t new_state = g_io_exp_output_reg_state;

    if (G0) {
        new_state |= IO_EXP_SPK_Gain_0_Mask;
    } else {
        new_state &= ~IO_EXP_SPK_Gain_0_Mask;
    }

    if (G1) {
        new_state |= IO_EXP_SPK_Gain_1_Mask;
    } else {
        new_state &= ~IO_EXP_SPK_Gain_1_Mask;
    }

    return io_exp_set_out(new_state);
}

bool enable_codec(void) {
    const uint8_t new_state =
        g_io_exp_output_reg_state | IO_EXP_DAC_Not_Reset_Mask;

    return io_exp_set_out(new_state);
}

static uint8_t g_aic3105_reg_local_copy[] =
{0b00000000, // 0
 0b00000000, // 1
 0b00000000, // 2
 0b00010000, // 3
 0b00000010, // 4
 0b00000000, // 5
 0b00000000, // 6
 0b00000000, // 7
 0b00000000, // 8
 0b00000000, // 9
 0b00000000, // 10
 0b00000001, // 11
 0b00000000, // 12
 0b00000000, // 13
 0b00000000, // 14
 0b10000000, // 15
 0b10000000, // 16
 0b11111111, // 17
 0b11111111, // 18
 0b01111000, // 19
 0b01111000, // 20
 0b01111000, // 21
 0b01111000, // 22
 0b01111000, // 23
 0b01111000, // 24
 0b00000000, // 25
 0b00000000, // 26
 0b11111110, // 27
 0b00000000, // 28
 0b00000000, // 29
 0b11111110, // 30
 0b00000000, // 31
 0b00000000, // 32
 0b00000000, // 33
 0b00000000, // 34
 0b00000000, // 35
 0b00000000, // 36
 0b00000000, // 37
 0b00000000, // 38
 0b00000000, // 39
 0b00000000, // 40
 0b00000000, // 41
 0b00000000, // 42
 0b10000000, // 43
 0b10000000, // 44
 0b00000000, // 45
 0b00000000, // 46
 0b00000000, // 47
 0b00000000, // 48
 0b00000000, // 49
 0b00000000, // 50
 0b00000110, // 51
 0b00000000, // 52
 0b00000000, // 53
 0b00000000, // 54
 0b00000000, // 55
 0b00000000, // 56
 0b00000000, // 57
 0b00000110, // 58
 0b00000000, // 59
 0b00000000, // 60
 0b00000000, // 61
 0b00000000, // 62
 0b00000000, // 63
 0b00000000, // 64
 0b00000010, // 65
 0b00000000, // 66
 0b00000000, // 67
 0b00000000, // 68
 0b00000000, // 69
 0b00000000, // 70
 0b00000000, // 71
 0b00000010, // 72
 0b00000000, // 73
 0b00000000, // 74
 0b00000000, // 75
 0b00000000, // 76
 0b00000000, // 77
 0b00000000, // 78
 0b00000000, // 79
 0b00000000, // 80
 0b00000000, // 81
 0b00000000, // 82
 0b00000000, // 83
 0b00000000, // 84
 0b00000000, // 85
 0b00000010, // 86
 0b00000000, // 87
 0b00000000, // 88
 0b00000000, // 89
 0b00000000, // 90
 0b00000000, // 91
 0b00000000, // 92
 0b00000010, // 93
 0b00000000, // 94
 0b00000000, // 95
 0b00000000, // 96
 0b00000000, // 97
 0b00000000, // 98
 0b00000000, // 99
 0b00000000, // 100
 0b00000000, // 101
 0b00000010, // 102
 0b00000000, // 103
 0b00000000, // 104
 0b00000000, // 105
 0b00000000, // 106
 0b00000000, // 107
 0b00000000, // 108
 0b00000000}; // 109

typedef struct clock_cfg {
    uint8_t j;
    uint16_t d;
    uint8_t r;
    uint8_t p;
} clock_cfg;

bool aic3105_write_reg (uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};

    const int result = i2c_write_blocking(I2C_PORT, AIC3105_ADDR, buf, 2, false);

    if (result == 2) {
        g_aic3105_reg_local_copy[reg] = value;
        return true;
    } else {
        return false;
    }
}

bool aic3105_write_bit(uint8_t reg, uint8_t pos, uint8_t value) {
    uint8_t current = g_aic3105_reg_local_copy[reg];
    const uint8_t mask = 1 << pos;

    if (value) {
        current |= mask;
    } else {
        current &= ~mask;
    }

    return aic3105_write_reg(reg, current);
}

bool aic3105_write_multi(uint8_t reg, uint8_t msb, uint8_t lsb, uint8_t value) {
    uint8_t current = g_aic3105_reg_local_copy[reg];

    const uint8_t lsb_mask = ~((1 << lsb) - 1);
    const uint8_t msb_mask = (1 << (msb + 1)) - 1;
    const uint8_t clear_mask = lsb_mask ^ msb_mask;

    current &= clear_mask;
    current |= (value << lsb);

    return aic3105_write_reg(reg, current);
}

uint8_t volume_convert(float volume, uint8_t min, uint8_t max, uint8_t mute_value) {
    if (volume == 0.0) {
        return mute_value;
    } else if (volume > 1.0) {
        volume = 1.0;
    } else if (volume < 0.0) {
        volume = 0.0;
    }

    if (max > min) {
        const float amplitude = (float) (max - min);
        const uint8_t val = (uint8_t) (volume * amplitude);
        return min + val;
    } else {
        const float amplitude = (float) (min - max);
        const uint8_t val = (uint8_t) ((1.0 - volume) * amplitude);
        return max + val;
    }
}

#define PGA_VOLUME(volume) volume_convert(volume, 0, 0b1111111, 0)
#define OUTPUT_STAGE_VOLUME(volume) volume_convert(volume, 117, 0, 118)

typedef enum out_mixer_source {
   LINE2_L, PGA_L, DAC_L1, LINE2_R, PGA_R, DAC_R1
} out_mixer_source;

typedef enum out_mixer_sink {
   HP_L_OUT, HP_L_COM, HP_R_OUT, HP_R_COM, LINE_OUT_L, LINE_OUT_R
} out_mixer_sink;

uint8_t sink_base_register(out_mixer_sink sink) {
    switch (sink)
    {
    case HP_L_OUT:
        return 45;
        break;
    case HP_L_COM:
        return 52;
        break;
    case HP_R_OUT:
        return 59;
        break;
    case HP_R_COM:
        return 66;
        break;
    case LINE_OUT_L:
        return 80;
        break;
    case LINE_OUT_R:
        return 87;
        break;
    default:
        return 0;
        break;
    }
}

uint8_t source_register_offset(out_mixer_source source) {
    switch (source)
    {
    case LINE2_L:
        return 0;
        break;
    case PGA_L:
        return 1;
        break;
    case DAC_L1:
        return 2;
        break;
    case LINE2_R:
        return 3;
        break;
    case PGA_R:
        return 4;
        break;
    case DAC_R1:
        return 5;
        break;
    default:
        return 0;
        break;
    }
}

bool power_on(out_mixer_sink sink) {
    const uint8_t reg = sink_base_register(sink) + 6;

    return aic3105_write_bit(reg, 0, 1);
}

bool power_off(out_mixer_sink sink) {
    const uint8_t reg = sink_base_register(sink) + 6;

    return aic3105_write_bit(reg, 0, 0);
}

bool mute(out_mixer_sink sink) {
    const uint8_t reg = sink_base_register(sink) + 6;

    return aic3105_write_bit(reg, 3, 0);
}

bool unmute(out_mixer_sink sink) {
    const uint8_t reg = sink_base_register(sink) + 6;

    return aic3105_write_bit(reg, 3, 1);
}

bool route(out_mixer_source source, out_mixer_sink sink) {
    const uint8_t reg = sink_base_register(sink) + source_register_offset(source);

    return aic3105_write_bit(reg, 7, 1);
}

bool unroute(out_mixer_source source, out_mixer_sink sink) {
    const uint8_t reg = sink_base_register(sink) + source_register_offset(source);

    return aic3105_write_bit(reg, 7, 0);
}

bool set_volume(out_mixer_source source, out_mixer_sink sink, float volume) {
    const uint8_t vol = OUTPUT_STAGE_VOLUME(volume);
    const uint8_t reg = sink_base_register(sink) + source_register_offset(source);

    return aic3105_write_multi(reg, 6, 0, vol);
}

bool init_aic3105(int sample_rate){
    bool success = true;
    clock_cfg cfg;

    switch (sample_rate) {
    case 8000:
        cfg = (clock_cfg){5, 4613, 1, 8};
        break;
    case 16000:
        cfg = (clock_cfg){5, 4613, 1, 4};
        break;
    case 22050:
        cfg = (clock_cfg){7, 5264, 1, 4};
        break;
    case 32000:
        cfg = (clock_cfg){5, 4613, 1, 2};
        break;
    case 44100:
        cfg = (clock_cfg){7, 5264, 1, 2};
        break;
    case 48000:
        cfg = (clock_cfg){8, 1920, 1, 2};
        break;
    default:
        return false;
    }

    nn_i2c_init();
    enable_codec();

    sleep_ms(10);

      //  Select Page 0
      success = aic3105_write_bit(AIC3X_PAGE_SELECT, 0, 0);

      //  Soft reset
      success = success && aic3105_write_bit(AIC3X_RESET, 7, 1);

      //  Let's start with clock configuration.

      //  PLL P = 2
      success = success && aic3105_write_multi(AIC3X_PLL_PROGA_REG, 2, 0, cfg.p);

      //  PLL R = 1
      success = success && aic3105_write_multi(AIC3X_OVRF_STATUS_AND_PLLR_REG, 3, 0, cfg.r);

      //  PLL J = 7
      success = success && aic3105_write_multi(AIC3X_PLL_PROGB_REG, 7, 2, cfg.j);

      //  PLL D = 5264
      const uint16_t PLL_D = cfg.d;
      const uint16_t REG_C = PLL_D >> 6;
      const uint16_t REG_D = PLL_D & 0x3F;
      success = success && aic3105_write_multi(AIC3X_PLL_PROGC_REG, 7, 0, (uint8_t) REG_C);
      success = success && aic3105_write_multi(AIC3X_PLL_PROGD_REG, 7, 2, (uint8_t) REG_D);

      //  Select the PLLCLK_IN source. 0: MCLK, 1: GPIO2, 2: BCLK
      success = success && aic3105_write_multi(AIC3X_CLKGEN_CTRL_REG, 5, 4, 2);

      //  Select the CLKDIV_IN source. 0: MCLK, 1: GPIO2, 2: BCLK
      //
      //  Note: When PLL is used CLKDIV_IN still needs some kind of clock
      //  signal. So if there's no MCLK, BCLK should be used here as well
      success = success && aic3105_write_multi(AIC3X_CLKGEN_CTRL_REG, 6, 7, 0);

      //  Enable PLL
      success = success && aic3105_write_bit(AIC3X_PLL_PROGA_REG, 7, 1);

      //  Set FS(ref) value for AGC time constants to 44.1KHz
      success = success && aic3105_write_bit(AIC3X_CODEC_DATAPATH_REG, 7, 1);

      //  CODEC_CLKIN Source Selection. 0: PLLDIV_OUT. 1: CLKDIV_OUT
      success = success && aic3105_write_bit(AIC3X_CLOCK_REG, 0, 0);

      //  Note: We leave the ADC Sample Rate Select and DAC Sample Rate Select
      //  at the default value: fs(ref) / 1

      //  Audio Serial Data Interface at the default settings: I2S
      //  mode, 16bits words,
      success = success && aic3105_write_multi(AIC3X_ASD_INTF_CTRLB, 7, 6, 0b00);

      //  Output Driver Power-On Delay Control
      success = success && aic3105_write_multi(HPOUT_POP_REDUCTION, 7, 4, 0b1000);

      //  Driver Ramp-Up Step Timing Control
      success = success && aic3105_write_multi(HPOUT_POP_REDUCTION, 3, 2, 0b01);

      //  Power outputs
      success = success && power_on(HP_L_OUT);
      success = success && power_on(HP_R_OUT);

      //  L and R DACs Power On
      success = success && aic3105_write_multi(DAC_PWR, 7, 6, 0b11);

      //  Left DAC plays left input data
      success = success && aic3105_write_multi(AIC3X_CODEC_DATAPATH_REG, 4, 3, 0b01);
      //  Right DAC plays right input data
      success = success && aic3105_write_multi(AIC3X_CODEC_DATAPATH_REG, 2, 1, 0b01);

      //  Unmute L DAC
      success = success && aic3105_write_bit(LDAC_VOL, 7, 0);
      //  Unmute R DAC
      success = success && aic3105_write_bit(RDAC_VOL, 7, 0);

      //  Left-DAC output selects DAC_L1 path.
      success = success && aic3105_write_multi(DAC_LINE_MUX, 7, 6, 0);
      //  Right-DAC output selects DAC_R1 path.
      success = success && aic3105_write_multi(DAC_LINE_MUX, 5, 4, 0);

      //  DAC to HP
      success = success && route(DAC_L1, HP_L_OUT);
      success = success && route(DAC_R1, HP_R_OUT);

      //  DAC to Line-Out
      success = success && route(DAC_L1, LINE_OUT_L);
      success = success && route(DAC_R1, LINE_OUT_R);

      //  Enable Left ADC
      success = success && aic3105_write_bit(LINE1L_2_LADC_CTRL, 2, 1);
      //  Enable Right ADC
      success = success && aic3105_write_bit(LINE1R_2_RADC_CTRL, 2, 1);

      //  Unmute L ADC PGA
      success = success && aic3105_write_bit(LADC_VOL, 7, 0);
      //  Unmute R ADC PGA
      success = success && aic3105_write_bit(RADC_VOL, 7, 0);

      //  Programs high-power outputs for ac-coupled driver configuration
      success = success && aic3105_write_bit(AIC3X_HEADSET_DETECT_CTRL_B, 7, 1);

      //  HPLCOM configured as independent single-ended output
      success = success && aic3105_write_multi(HPLCOM_CFG, 5, 4, 2);

      //  HPRCOM configured as independent single-ended output
      success = success && aic3105_write_multi(HPRCOM_CFG, 5, 3, 1);

      //  Unmute outputs
      success = success && unmute(HP_L_OUT);
      success = success && unmute(HP_R_OUT);

    return success;
}

bool enable_line_out (bool left, bool right) {
    bool success = true;

    if (left) {
        success = success && power_on(LINE_OUT_L);
        success = success && route(DAC_L1, LINE_OUT_L);
        success = success && route(DAC_L1, LINE_OUT_R);
        success = success && unmute(LINE_OUT_L);
    } else {
        success = success && power_off(LINE_OUT_L);
        success = success && unroute(DAC_L1, LINE_OUT_L);
        success = success && unroute(DAC_L1, LINE_OUT_R);
        success = success && mute(LINE_OUT_L);
    }

    if (right) {
        success = success && power_on(LINE_OUT_R);
        success = success && route(DAC_R1, LINE_OUT_L);
        success = success && route(DAC_R1, LINE_OUT_R);
        success = success && unmute(LINE_OUT_R);
    } else {
        success = success && power_off(LINE_OUT_R);
        success = success && unroute(DAC_R1, LINE_OUT_L);
        success = success && unroute(DAC_R1, LINE_OUT_R);
        success = success && mute(LINE_OUT_R);
    }

    return success;
}

bool enable_speakers(bool left, bool right, uint8_t gain) {
    bool success = false;
    switch (gain)
    {
    case 1:
        success = set_speaker_gain(true, false);
        break;

    case 2:
        success = set_speaker_gain(false, true);
        break;

    case 3:
        success = set_speaker_gain(true, true);
        break;

    default:
        success = set_speaker_gain(false, false);
    }

    success = success && enable_line_out(left, right);
    success = success && io_exp_enable_speakers(left, right);
    return success;
}

bool set_adc_volume (float left, float right) {
    bool success = true;

    if (left == 0) {
        success = success && aic3105_write_reg(LADC_VOL, 0b10000000);
    } else {
        success = success && aic3105_write_reg(LADC_VOL, PGA_VOLUME (left));
    }

    if (right == 0) {
        success = success && aic3105_write_reg(RADC_VOL, 0b10000000);
    } else {
        success = success && aic3105_write_reg(RADC_VOL, PGA_VOLUME (right));
    }
    return success;
}

bool set_hp_volume (float left, float right) {
    return set_volume(DAC_L1, HP_L_OUT, left) && set_volume(DAC_R1, HP_R_OUT, right);
}

bool set_line_out_volume(float L2L, float L2R, float R2L, float R2R) {
    bool success = set_volume(DAC_L1, LINE_OUT_L, L2L);
    success = success && set_volume(DAC_L1, LINE_OUT_R, L2R);

    success = success && set_volume(DAC_R1, LINE_OUT_L, R2L);
    success = success && set_volume(DAC_R1, LINE_OUT_R, R2R);

    return success;
}

uint8_t boost_to_reg (uint8_t b) {
    if (b >= 9) {
        return 0b0000;
    } else if (b == 0) {
        return 0b1111; // mute
    } else {
        return (0b1000 + 1) - b;
    }
}

bool set_line_in_boost (uint8_t line, uint8_t L2L, uint8_t L2R, uint8_t R2L, uint8_t R2R) {
    bool success = true;

    const uint8_t L2LB = boost_to_reg(L2L);
    const uint8_t L2RB = boost_to_reg(L2R);
    const uint8_t R2LB = boost_to_reg(R2L);
    const uint8_t R2RB = boost_to_reg(R2R);

    switch (line)
    {
    case 1:
        success = success && aic3105_write_multi(LINE1L_2_LADC_CTRL, 6, 3, L2LB);
        success = success && aic3105_write_multi(LINE1L_2_RADC_CTRL, 6, 3, L2RB);
        success = success && aic3105_write_multi(LINE1R_2_LADC_CTRL, 6, 3, R2LB);
        success = success && aic3105_write_multi(LINE1R_2_RADC_CTRL, 6, 3, R2RB);
        break;

    case 2:
        success = success && aic3105_write_multi(LINE2L_2_LADC_CTRL, 6, 3, L2LB);
        success = success && aic3105_write_multi(LINE2R_2_RADC_CTRL, 6, 3, R2RB);
        break;

    case 3:
        success = success && aic3105_write_multi(MIC3LR_2_LADC_CTRL, 6, 3, (L2LB << 4) & R2LB);
        success = success && aic3105_write_multi(MIC3LR_2_RADC_CTRL, 6, 3, (L2RB << 4) & R2RB);
        break;

    default:
        success = false;
        break;
    }
    return success;
}

bool enable_mic_bias (void) {
    return aic3105_write_multi(MICBIAS_CTRL, 7, 6, 0b10);
}

bool audio_init(int sample_rate,
                audio_cb_t output_callback,
                audio_cb_t input_callback)
{
    bool success = true;

    user_audio_input_callback = input_callback;
    user_audio_output_callback = output_callback;

    success &= init_i2s(sample_rate);
    success &= nn_i2c_init();
    success &= init_aic3105(sample_rate);

    return success;
}
