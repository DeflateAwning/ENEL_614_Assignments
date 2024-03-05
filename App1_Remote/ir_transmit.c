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

void ir_tx_single_start() {
    // make carrier wave for on ON portion    
    for (uint8_t carrier_cycle_num = 0;
            carrier_cycle_num < CARRIER_CYCLE_COUNT_4500US;
            carrier_cycle_num++ ) {
        // do one carrier cycle
        SET_IR_STATE(1);
        delay32_cycles(46); // previously CLOCK_CYCLES_IN_38KHZ_HALF_PERIOD, now determined by experiment
        SET_IR_STATE(0);
        delay32_cycles(46);
    }
    
    // make silence for the OFF portion
    // no need to generate carrier here
    SET_IR_STATE(0);
//    delay32_cycles(off_clk_cycle_count);
    delay32_us(4500);
}

void ir_tx_single_bit_0() {
    // make carrier wave for on ON portion    
    for (uint8_t carrier_cycle_num = 0;
            carrier_cycle_num < CARRIER_CYCLE_COUNT_560US;
            carrier_cycle_num++ ) {
        // do one carrier cycle
        SET_IR_STATE(1);
        delay32_cycles(46); // previously CLOCK_CYCLES_IN_38KHZ_HALF_PERIOD, now determined by experiment
        SET_IR_STATE(0);
        delay32_cycles(46);
    }
    
    // make silence for the OFF portion
    // no need to generate carrier here
    SET_IR_STATE(0);
    delay32_us(560);
}

void ir_tx_single_bit_1() {
    // make carrier wave for on ON portion    
    for (uint8_t carrier_cycle_num = 0;
            carrier_cycle_num < CARRIER_CYCLE_COUNT_560US;
            carrier_cycle_num++ ) {
        // do one carrier cycle
        SET_IR_STATE(1);
        delay32_cycles(46); // previously CLOCK_CYCLES_IN_38KHZ_HALF_PERIOD, now determined by experiment
        SET_IR_STATE(0);
        delay32_cycles(46);
    }
    
    // make silence for the OFF portion
    // no need to generate carrier here
    SET_IR_STATE(0);
    delay32_us(1690);
}

void ir_tx_32_bit_code(uint32_t code) {
    ir_tx_single_start();
    
//    Disp2String("Start of code: ");
    
    for (int8_t bit_place = 31; bit_place >= 0; bit_place--) {
        if ((code & (1UL << bit_place)) > 0) {
//            Disp2String("1");
            ir_tx_single_bit_1();
        }
        else {
//            Disp2String("0");
            ir_tx_single_bit_0();
        }
    }
    
//    Disp2String(" END\n");
    
    // force LED to low
    ir_tx_reset();
}
