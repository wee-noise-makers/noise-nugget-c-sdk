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

#define I2S_PIO pio1
#define I2S_SM 0
#define I2S_PIN_FUNC GPIO_FUNC_PIO1

#define I2S_IN_PIN 0
#define I2S_OUT_PIN 1
#define I2S_LRCLK_PIN 2
#define I2S_BCLK_PIN 3
#define I2S_MCLK_PIN 4

#define WM8960_ADDR 0x1A

#define I2C_PORT i2c1
#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7

int i2s_out_dma_chan = -1; // init with invalid DMA channel id
int i2s_in_dma_chan = -1; // init with invalid DMA channel id

#define TEST_AUDIO_BUFFER_SIZE 256
uint32_t test_audio_buffer_in[TEST_AUDIO_BUFFER_SIZE] = {0x42};
uint32_t test_audio_buffer_out[TEST_AUDIO_BUFFER_SIZE] = {0x42};

void dma_out_handler() {
    // Clear the interrupt request.
    dma_hw->ints0 = 1u << i2s_out_dma_chan;

    dma_channel_transfer_from_buffer_now(i2s_out_dma_chan,
                                         test_audio_buffer_out,
                                         TEST_AUDIO_BUFFER_SIZE);
}

void dma_in_handler() {
    // Clear the interrupt request.
    dma_hw->ints1 = 1u << i2s_in_dma_chan;

    dma_channel_transfer_to_buffer_now(i2s_in_dma_chan,
                                       test_audio_buffer_in,
                                       TEST_AUDIO_BUFFER_SIZE);
}

void setup_audio_pin(int pin, bool out) {
    gpio_init(pin);
    gpio_set_dir(pin, out ? GPIO_OUT : GPIO_IN);
    gpio_pull_up(pin);
    gpio_pull_down(pin);
    gpio_set_function(pin, I2S_PIN_FUNC);
}

bool init_i2s(int sample_rate) {


    for (int i = 0; i < TEST_AUDIO_BUFFER_SIZE; i++) {
        test_audio_buffer_out[i] = (i % 10) == 0 ? 0x80008000 : 0x7fff7fff;
    }

    // Square wave with PWM for the MCLK signal

    const int MCLK_requested_frequency = 256 * sample_rate * 2;
    gpio_set_function(I2S_MCLK_PIN, GPIO_FUNC_PWM);
    const uint pwm_slice_num = pwm_gpio_to_slice_num(I2S_MCLK_PIN);
    const uint pwm_channel = pwm_gpio_to_channel(I2S_MCLK_PIN);

    pwm_set_clkdiv
        (pwm_slice_num,
         (float)clock_get_hz(clk_sys) / (float)MCLK_requested_frequency);

    pwm_set_wrap(pwm_slice_num, 1);
    pwm_set_both_levels(pwm_slice_num, 1, 1);
    pwm_set_enabled(pwm_slice_num, true);

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

bool init_i2c(void){
    gpio_init(I2C_SCL_PIN);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SCL_PIN);

    gpio_init(I2C_SDA_PIN);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);

    i2c_init(I2C_PORT, 100 * 1000);

    return true;
}

bool wm8060_write_register(uint8_t reg, uint16_t value){
    uint8_t buf[2] = {(reg << 1) | ((value & 0x100) >> 8), value & 0xff};

    const int result = i2c_write_blocking(I2C_PORT, WM8960_ADDR, buf, 2, false);
    return result == 2;
}

bool init_wm8960(void){
    bool success = true;

    success &= wm8060_write_register(0xf, 0); // Reset
    success &= wm8060_write_register(0x7, 0x2); // 16-bit I2S format
    success &= wm8060_write_register(0x4, 0x0); // SYSCLK = MCLK 11.2 mhz

    success &= wm8060_write_register(0xa, 0x1ff); // Left DAC volume
    success &= wm8060_write_register(0xb, 0x1ff); // Right DAC volume

    success &= wm8060_write_register(0x2, 0x079); // Left headphone volume
    success &= wm8060_write_register(0x3, 0x179); // Right headphone volume

    success &= wm8060_write_register(0x19, 0xc0); // 2 x 50k divider enabled and enable vref

    sleep_ms(100);

    success &= wm8060_write_register(0x1a, 0x180); // DAC L + R

    success &= wm8060_write_register(0x22, 0x100); // Left DAC to Left Mixer
    success &= wm8060_write_register(0x25, 0x100); // Right DAC to Right Mixer

    success &= wm8060_write_register(0x2f, 0xc); // L + R Mixer output

    success &= wm8060_write_register(0x1a, 0x1e0); // DACL, DACR, LOUT1, ROUT1

    // Unmute DAC
    success &= wm8060_write_register(0x5, 0);

    return success;
}

bool audio_init(int sample_rate) {
    bool success = true;

    success &= init_i2s(sample_rate);
    success &= init_i2c();
    success &= init_wm8960();

    return success;
}
