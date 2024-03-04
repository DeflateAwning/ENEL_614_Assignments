/*
 * File:   timer.c

 *
 * Created on February 12, 2024, 6:05 PM
 */


#include "xc.h"
#include "timer.h"
#include "clock.h"

// globals

// set to 0 when delay is triggered, set to 1 when delay is over
volatile uint8_t is_tmr2_isr_serviced_yet = 0;

void delay_us(uint16_t delay_time_us) {
    // NOTE: delay_us requires that the clock be set to 8 MHz

    is_tmr2_isr_serviced_yet = 0;

    // Configure clock: must be 8 MHz for us delays
    // uint16_t orig_active_clk_freq_khz = active_clk_freq_khz;
    // set_clock_freq(8000); // 8 MHz

    // region T2CON Configuration
    // start 16-bit Timer2
    T2CONbits.TON = 1;
    
    // continue module operation in idle mode
    T2CONbits.TSIDL = 0;
    
    // Timer2 and Timer3 each act separately as 16-bit timers
    T2CONbits.T32 = 0;
    
    // Timer2 clock source: internal (Fosc/2)
    T2CONbits.TCS = 0;
    
    // Set Timer2 prescaler
    // 0b11 = 1:256, 0b10 = 1:64, 0b01 = 1:8, 0b00 = 1:1
    T2CONbits.TCKPS = 0b00; // 1:1
    // endregion

    // region Timer2 interrupt configuration
    // IPC (interupt priority control register): 1 (lowest) to 7 (highest priority)
    IPC1bits.T2IP = 7; // Timer2 interrupt priority level: 7 (highest)

    // enable timer
    IEC0bits.T2IE = 1;
    
    // "In the event of interrupt, TxIF bit is set. Clear it at setup"
    IFS0bits.T2IF = 0;
    // endregion
    
    // Compute PR2
    // PR2 is number of '2/fclk' cycles to wait before triggering an interrupt
    PR2 = delay_time_us * 4;
    // PR2 = delay_time_us * (active_clk_freq_khz / 2000); <- right math, but can't do the division on processor
    // 50 us at 8 MHz = 400 cycles

    // Idle until waiting for timer 2 interrupt to be serviced
    while (is_tmr2_isr_serviced_yet == 0) {
        Idle();
    }

    // if (orig_active_clk_freq_khz != active_clk_freq_khz) {
    //     set_clock_freq(orig_active_clk_freq_khz);
    // }
}

void delay_ms(uint16_t delay_time_ms) {
    is_tmr2_isr_serviced_yet = 0;

    // store orig clock, prevent overflow
    uint16_t orig_active_clk_freq_khz = active_clk_freq_khz;
    if ((active_clk_freq_khz >= 500) && (delay_time_ms > 200)) {
        set_clock_freq(32);
    }
    else if (active_clk_freq_khz == 8000) {
        set_clock_freq(500);
    }

    // region T2CON Configuration
    // start 16-bit Timer2
    T2CONbits.TON = 1;
    
    // continue module operation in idle mode
    T2CONbits.TSIDL = 0;
    
    // Timer2 and Timer3 each act separately as 16-bit timers
    T2CONbits.T32 = 0;
    
    // Timer2 clock source: internal (Fosc/2)
    T2CONbits.TCS = 0;
    
    // Set Timer2 prescaler
    // 0b11 = 1:256, 0b10 = 1:64, 0b01 = 1:8, 0b00 = 1:1
    T2CONbits.TCKPS = 0b00; // 1:1
    // endregion

    // region Timer2 interrupt configuration
    // IPC (interupt priority control register): 1 (lowest) to 7 (highest priority)
    IPC1bits.T2IP = 7; // Timer2 interrupt priority level: 7 (highest)

    // enable timer
    IEC0bits.T2IE = 1;
    
    // "In the event of interrupt, TxIF bit is set. Clear it at setup"
    IFS0bits.T2IF = 0;
    // endregion
    
    // Compute PR2
    // PR2 is number of '2/fclk' cycles to wait before triggering an interrupt
    // PR2 = delay_time_ms * (active_clk_freq_khz / 2);
    PR2 = delay_time_ms * (active_clk_freq_khz >> 1);

    // Idle until waiting for timer 2 interrupt to be serviced
    while (is_tmr2_isr_serviced_yet == 0) {
        Idle();
    }

    if (orig_active_clk_freq_khz != active_clk_freq_khz) {
        set_clock_freq(orig_active_clk_freq_khz);
    }
}

void delay_sec(uint16_t delay_time_sec) {
    is_tmr2_isr_serviced_yet = 0;
    uint16_t orig_active_clk_freq_khz = active_clk_freq_khz;

    set_clock_freq(32);

    // region T2CON Configuration
    // start 16-bit Timer2
    T2CONbits.TON = 1;
    
    // continue module operation in idle mode
    T2CONbits.TSIDL = 0;
    
    // Timer2 and Timer3 each act separately as 16-bit timers
    T2CONbits.T32 = 0;
    
    // Timer2 clock source: internal (Fosc/2)
    T2CONbits.TCS = 0;
    
    // Set Timer2 prescaler
    // 0b11 = 1:256, 0b10 = 1:64, 0b01 = 1:8, 0b00 = 1:1
    T2CONbits.TCKPS = 0b00; // 1:1
    // endregion

    // region Timer2 interrupt configuration
    // IPC (interupt priority control register): 1 (lowest) to 7 (highest priority)
    IPC1bits.T2IP = 7; // Timer2 interrupt priority level: 7 (highest)

    // enable timer
    IEC0bits.T2IE = 1;
    
    // "In the event of interrupt, TxIF bit is set. Clear it at setup"
    IFS0bits.T2IF = 0;
    // endregion
    
    // Compute PR2
    // PR2 is number of '2/fclk' cycles to wait before triggering an interrupt
    // PR2 = delay_time_ms * (active_clk_freq_khz / 2);
    PR2 = delay_time_sec * 1000 * (active_clk_freq_khz >> 1);

    // Idle until waiting for timer 2 interrupt to be serviced
    while (is_tmr2_isr_serviced_yet == 0) {
        Idle();
    }

    if (orig_active_clk_freq_khz != active_clk_freq_khz) {
        set_clock_freq(orig_active_clk_freq_khz);
    }
}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void) {
    // Timer2 ISR: triggers once when TMR2 = PR2, which is when the delay is over

    // Clear timer 2 interrupt flag
    IFS0bits.T2IF = 0;

    // Stop timer 2
    T2CONbits.TON = 0;

    is_tmr2_isr_serviced_yet = 1;
}
