#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"
#include "hardware/adc.h"

#define AUDIO_PIN 2


#include "./media/sample_22kHz_16bits_mono.h"

int pwm_dma_chan;

void dma_irh() {
    dma_hw->ch[pwm_dma_chan].al3_read_addr_trig = (long unsigned int)audio_buffer;
    dma_hw->ints0 = (1u << pwm_dma_chan);
}

int main(void) {
    stdio_init_all();

    set_sys_clock_khz(176000, true); 

    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    int audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_config config = pwm_get_default_config();

    if( SAMPLE_BIT_DEPTH == 8) {
        pwm_config_set_clkdiv(&config, 31.1f);  // clk_divider = (176mHz / 22kHz) / 256 = 31.1
        pwm_config_set_wrap(&config, 256);      // Wrap value should 2^sample_bit_depth ~ 8 bits
    } else if (SAMPLE_BIT_DEPTH == 13) {
        // Any sample above 7981 will be clipped
        pwm_config_set_clkdiv(&config, 1.0f);   // clk_divider = (176mHz / 22kHz) / 8192 = 0.97
        pwm_config_set_wrap(&config, 7981);     // Wrap value should 2^sample_bit_depth ~ 13 bits        
    } else {
        return -1;
    }

    pwm_init(audio_pin_slice, &config, true);

    pwm_set_gpio_level(AUDIO_PIN, 0);

    // Setup DMA channel to drive the PWM
    pwm_dma_chan = dma_claim_unused_channel(true);

    dma_channel_config pwm_dma_chan_config = dma_channel_get_default_config(pwm_dma_chan);
    // Transfer 16 bits at once, increment read address to go through sample
    // buffer, always write to the same address (PWM slice CC register).
    channel_config_set_transfer_data_size(&pwm_dma_chan_config, DMA_SIZE_16);
    channel_config_set_read_increment(&pwm_dma_chan_config, true);
    channel_config_set_write_increment(&pwm_dma_chan_config, false);
    // Transfer on PWM cycle end
    channel_config_set_dreq(&pwm_dma_chan_config, DREQ_PWM_WRAP0 + audio_pin_slice);

    // Setup the channel and set it going
    dma_channel_configure(
        pwm_dma_chan,
        &pwm_dma_chan_config,
        &pwm_hw->slice[audio_pin_slice].cc, // Write to PWM counter compare
        audio_buffer, // Read values from audio buffer
        AUDIO_SAMPLES,
        true // Start immediately.
    );

    // Setup interrupt handler to fire when PWM DMA channel has gone through the
    // whole audio buffer
    dma_channel_set_irq0_enabled(pwm_dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irh);
    irq_set_enabled(DMA_IRQ_0, true);

    while(true) {
        tight_loop_contents();
    }
}
