/*
 * File:   ir_transmit.c
 */


#include "xc.h"
#include "ir_receive.h"
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


// 4500us / 200us = 22.5 elems
const uint16_t elem_count_4500us_min = 18;
const uint16_t elem_count_4500us_max = 40;

// 560us / 200us = 2.8 elems
const uint16_t elem_count_560us_min = 1;
const uint16_t elem_count_560us_max = 5;

// 1690us / 200us = 8.45 elems
const uint16_t elem_count_1690us_min = 6;
const uint16_t elem_count_1690us_max = 14;

// internal only; not in header
void count_consec_elems(
                const uint8_t arr[], uint32_t arr_len, uint8_t elem, uint32_t start_idx,
                uint32_t* output_consec_count, uint32_t* output_end_idx
    ) {
    uint32_t consec_count = 0;
    uint32_t arr_idx;
    for (arr_idx = start_idx; arr_idx < arr_len; arr_idx++) {
        if (arr[arr_idx] == elem) {
            consec_count++;
        }
        else {
            break;
        }
    }
    *output_consec_count = consec_count;
    *output_end_idx = arr_idx;
}

uint32_t parse_carrier_log_to_code(const uint8_t carrier_detect_log[], uint32_t carrier_detect_log_len) {
    // returns 0 if no code detected, hopefully

    uint32_t carrier_detect_log_idx = 0;
    
    uint32_t consec_count = 0;
    uint32_t end_idx;

    // find the start bit [high for 4500us, low for 4500us] (end-of-HIGH section)
    for (carrier_detect_log_idx = 0; carrier_detect_log_idx < carrier_detect_log_len; carrier_detect_log_idx++) {
        count_consec_elems(
            carrier_detect_log, carrier_detect_log_len, 1, carrier_detect_log_idx,
            &consec_count, &end_idx);

        if (consec_count >= elem_count_4500us_min) {
            // found the start bit ON part
            break;
        }
    }

    // find the start bit [high for 4500us, low for 4500us] (end-of-LOW section)
    for (carrier_detect_log_idx = end_idx; carrier_detect_log_idx < carrier_detect_log_len; carrier_detect_log_idx++) {
        count_consec_elems(
            carrier_detect_log, carrier_detect_log_len, 0, carrier_detect_log_idx,
            &consec_count, &end_idx);

        if (consec_count >= elem_count_4500us_min) {
            // found the start bit ON part
            break;
        }
    }

    uint32_t output_code = 0;

    for (uint8_t bit_place = 31; bit_place >= 0; bit_place--) {
        // find the regular high bit
        for (carrier_detect_log_idx = end_idx; carrier_detect_log_idx < carrier_detect_log_len; carrier_detect_log_idx++) {
            count_consec_elems(
                carrier_detect_log, carrier_detect_log_len, 1, carrier_detect_log_idx,
                &consec_count, &end_idx);

            if ((consec_count >= elem_count_560us_min) && (consec_count <= elem_count_560us_max)) {
                // found the start bit ON part
                break;
            }
        }

        // find the low bit - either 560us (Bit 0) or 1690us (Bit 1)
        for (carrier_detect_log_idx = end_idx; carrier_detect_log_idx < carrier_detect_log_len; carrier_detect_log_idx++) {
            count_consec_elems(
                carrier_detect_log, carrier_detect_log_len, 0, carrier_detect_log_idx,
                &consec_count, &end_idx);

            // now know the length of the OFF section - find out if it's a Bit 0 or Bit 1
            if ((consec_count >= elem_count_560us_min) && (consec_count <= elem_count_560us_max)) {
                // no change to output_code (it's a 0)
                break;
            }
            else if ((consec_count >= elem_count_1690us_min) && (consec_count <= elem_count_1690us_max)) {
                // set the bit_place to a 1
                // bit_place 31 is the temporally-first bit received, then bit 30, ...
                output_code |= (1UL << bit_place);
                break;
            }
        }
    }
}

void debug_print_carrier_log(const uint8_t carrier_detect_log[], uint32_t carrier_detect_log_len) {
    Disp2String("Carrier detect log:        ");
    for (uint32_t carrier_detect_log_idx = 0; carrier_detect_log_idx < carrier_detect_log_len; carrier_detect_log_idx++) {
        if (carrier_detect_log[carrier_detect_log_idx] == 1) {
            Disp2String("1");
        }
        else {
            Disp2String("_");
        }
    }
    Disp2String("\n");
    
    Disp2String("Carrier detect log counts: ");
    uint32_t carrier_detect_log_idx = 0;
    while (carrier_detect_log_idx < carrier_detect_log_len) {
        uint32_t consec_count = 0;
        uint32_t end_idx = 0;
        
        count_consec_elems(
            carrier_detect_log, carrier_detect_log_len,
            carrier_detect_log[carrier_detect_log_idx], // elem (0 or 1) - use the current element at that place
            carrier_detect_log_idx,
            &consec_count, &end_idx);

        char msg[10];
        sprintf(msg, "%d", consec_count);
        Disp2String(msg);
        if (carrier_detect_log[carrier_detect_log_idx] == 1) {
            Disp2String("X");
        }
        else {
            Disp2String("_");
        }

        carrier_detect_log_idx = end_idx;
    }
    Disp2String("\n");
}
