/*
 * File:   ir_transmit.c
 */


#include "xc.h"
#include "ir_transmit.h"
#include "uart.h"
#include "delay.h"

// Assume 8 MHz clock
#define CLOCK_CYCLES_IN_38KHZ_HALF_PERIOD (105) // WA: ((1/(38 kHz)) at 8MHz) * 0.5

// Assume 38 kHz carrier
#define CARRIER_CYCLE_COUNT_4500US (171) // WA: 4500 us at 38 kHz
#define CARRIER_CYCLE_COUNT_560US (21) // WA: 560 us at 38 kHz
#define CARRIER_CYCLE_COUNT_1690US (64) // WA: 1690 us at 38 kHz

#define CLK_CYCLE_COUNT_4500US (36000) // WA: 4500 us at 8 MHz
#define CLK_CYCLE_COUNT_560US (4480) // WA: 560 us at 8 MHz
#define CLK_CYCLE_COUNT_1690US (13520) // WA: 1690 us at 8 MHz

#define SET_IR_STATE(x) (LATBbits.LATB9 = (x))

void ir_set_led_state(uint8_t en) {
    // ON: en=1; OFF: en=0;
    LATBbits.LATB9 = en;
}

void ir_tx_init() {
    TRISBbits.TRISB9 = 0; // set IR LED state as output
    SET_IR_STATE(0); // turn off to begin
}

void ir_tx_reset() {
    // set the IR pin to low
    SET_IR_STATE(0); // turn off to begin
}

void ir_tx_single_bit(IR_BIT_t ir_bit) {
    // transmit a single bit
    const uint8_t on_carrier_cycle_count = ((ir_bit == IR_BIT_START) ? CARRIER_CYCLE_COUNT_4500US : CARRIER_CYCLE_COUNT_560US);
    uint32_t off_clk_cycle_count = 3; // arbitrary case for invalid "bit"
    switch (ir_bit) {
        case IR_BIT_0:
            off_clk_cycle_count = CLK_CYCLE_COUNT_560US;
            break;
        case IR_BIT_1:
            off_clk_cycle_count = CLK_CYCLE_COUNT_1690US;
            break;
        case IR_BIT_START:
            off_clk_cycle_count = CLK_CYCLE_COUNT_4500US;
            break;
    }
    
    // make carrier wave for on ON portion    
    for (uint8_t carrier_cycle_num = 0;
            carrier_cycle_num < on_carrier_cycle_count;
            carrier_cycle_num++ ) {
        // do one carrier cycle
        SET_IR_STATE(1);
        delay32_cycles(46); // previously CLOCK_CYCLES_IN_38KHZ_HALF_PERIOD
        SET_IR_STATE(0);
        delay32_cycles(46);
    }
    
    // make silence for the OFF portion
    // no need to generate carrier here
    SET_IR_STATE(0);
    delay32_cycles(off_clk_cycle_count);
}

void ir_tx_32_bit_code(uint32_t code) {
    ir_tx_single_bit(IR_BIT_START);
    
    for (int8_t bit_place = 31; bit_place >= 0; bit_place--) {
        const uint8_t target_bit = (code & (1 << bit_place)) > 0; // 0 or 1
        
        // do the transmission (either 0 or 1)
        ir_tx_single_bit((IR_BIT_t) target_bit);
    }
    
    // force LED to low
    ir_tx_reset();
}
