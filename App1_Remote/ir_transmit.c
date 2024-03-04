/*
 * File:   ir_transmit.c
 */


#include "xc.h"
#include "ir_transmit.h"

// Assume 8 MHz clock
#define CLOCK_CYCLES_IN_38KHZ_HALF_PERIOD (105) // WA: ((1/(38 kHz)) at 8MHz) * 0.5

// Assume 38 kHz carrier
#define CARRIER_CYCLE_COUNT_4500US (171) // WA: 4500 us at 38 kHz
#define CARRIER_CYCLE_COUNT_560US (21) // WA: 560 us at 38 kHz
#define CARRIER_CYCLE_COUNT_1690US (64) // WA: 1690 us at 38 kHz

#define CLK_CYCLE_COUNT_4500US (36000) // WA: 4500 us at 8 MHz
#define CLK_CYCLE_COUNT_560US (4480) // WA: 560 us at 8 MHz
#define CLK_CYCLE_COUNT_1690US (13520) // WA: 1690 us at 8 MHz


typedef enum {
    IR_BIT_0 = 0,
    IR_BIT_1 = 1,
    IR_BIT_START = 2
} IR_BIT_t;

// define a function alias, because the old name is stupid and ambiguous
#define delay_clock_cycles(x) (__delay32((x)))

void ir_tx_init();
void ir_tx_reset();
void ir_tx_single_bit(IR_BIT_t bit);

void set_ir_led_state(uint8_t en) {
    // ON: en=1; OFF: en=0;
    LATBbits.LATB8 = en;
}

void ir_tx_init() {
    TRISBbits.TRISB9 = 0; // set IR LED state as output
    set_ir_led_state(0); // turn off to begin
}

void ir_tx_reset() {
    // set the IR pin to low
    set_ir_led_state(0); // turn off to begin
}

void ir_tx_single_bit(IR_BIT_t bit) {
    // transmit a single bit
    const uint8_t on_carrier_cycle_count = (bit == IR_BIT_START) ? CARRIER_CYCLE_COUNT_4500US : CARRIER_CYCLE_COUNT_560US;
    uint8_t off_clk_cycle_count = 3; // arbitrary case for invalid "bit"
    switch (bit) {
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
        // do one carrier cycle of 
        set_ir_led_state(1);
        delay_clock_cycles(CLOCK_CYCLES_IN_38KHZ_HALF_PERIOD);
        set_ir_led_state(0);
        delay_clock_cycles(CLOCK_CYCLES_IN_38KHZ_HALF_PERIOD);
    }
    
    // make silence for the OFF portion
    // no need to generate carrier here
    set_ir_led_state(0);
    delay_clock_cycles(off_clk_cycle_count);
}

void ir_tx_32_bit_code(uint32_t code) {
    ir_tx_single_bit(IR_BIT_START);
    
    for (int8_t bit_place = 31; bit_place >= 0; bit_place--) {
        const uint32_t target_bit_mask = (1 << bit_place);
        const uint8_t target_bit = (code & bit_place) > 0; // 0 or 1
        
        // do the transmission (either 0 or 1)
        ir_tx_single_bit((IR_BIT_t) target_bit);
    }
    
    // force LED to low
    ir_tx_reset();
}
