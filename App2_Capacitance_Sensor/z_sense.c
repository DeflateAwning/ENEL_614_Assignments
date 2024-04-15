/*
 * File:   z_sense.c
 */


#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "z_sense.h"
#include "uart.h"
#include "adc.h"

// Output is on Pin 16/AN11/RB13

// Arg current_value_exponent:
//     -1 -> 5.5x10^-1 uA = 0.55 uA
//      0 -> 5.5x10^0  uA = 5.5 uA
//      1 -> 5.5x10^1  uA = 55  uA
void init_ctmu(int8_t current_value_exponent) {
    CTMUCONbits.CTMUEN = 1; // enable
    // TODO: maybe more in here
    
    // these should all be zero by default, but good to set like this anyway
    CTMUCONbits.TGEN = 0;
    CTMUCONbits.EDGEN = 0;
    CTMUCONbits.IDISSEN = 0;
    CTMUCONbits.CTTRIG = 0;
    
    
    // CTMUICONbits.ITRIM = 0b00000; // 5 bits of offset (no offset)
    CTMUICONbits.ITRIM = 0b01111; // 5 bits of offset (max pos offset)
    
    if (current_value_exponent == -1) {
        // 0.55 uA
        CTMUICONbits.IRNG = 0b01;
    }
    else if (current_value_exponent == 0) {
        // 5.5 uA
        CTMUICONbits.IRNG = 0b10;
    }
    else if (current_value_exponent == 1) {
        // 55 uA
        CTMUICONbits.IRNG = 0b11;
    }
    
    CTMUCONbits.EDG1STAT = 1;
    CTMUCONbits.EDG2STAT = 0;
    
    AD1CON1bits.SAMP = 1; // Start sampling
}

void r_sense_and_log(int8_t current_value_exponent, uint32_t resistance_ohms, uint16_t sample_count) {
    init_ctmu(current_value_exponent);
    
    double current_source_value_uA = 0; //  = pow(5.5, current_value_exponent);
    if (current_value_exponent == -1) {
        current_source_value_uA = 0.55;
    }
    else if (current_value_exponent == 0) {
        current_source_value_uA = 5.5;
    }
    else if (current_value_exponent == 1) {
        current_source_value_uA = 55.0;
    }
    
    for (uint16_t i = 0; i < sample_count; i++) {
        char msg[255];
        
        // read the ADC value
        // Disp2String("Start read_adc_value");
        const uint16_t adc_value = read_adc_value();
        // Disp2String("Done read_adc_value");
        const double adc_v = adc_value * 3.3 / 1023.0;
        const double current_read_value_uA = (adc_v / ((double)(resistance_ohms))) * 1000000.0;
        
        sprintf(msg, "current_source_value_uA=%f, R=%ld, adc_value=%d, adc_volts=%3f, current_read_value_uA=%3f\n",
                current_source_value_uA, resistance_ohms, adc_value, adc_v, current_read_value_uA);
        
        Disp2String(msg);
    }
}

