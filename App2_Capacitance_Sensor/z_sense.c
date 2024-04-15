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
#include "delay.h"

// Output is on Pin 16/AN11/RB13

#define F_to_pF(cap_F) ((cap_F) * 1000000000000.0)
#define pF_to_F(cap_pF) ((cap_pF) / 1000000000000.0)

#define F_to_nF(cap_F) ((cap_F) * 1000000000.0)
#define nF_to_F(cap_nF) ((cap_nF) / 1000000000.0)

#define F_to_uF(cap_F) ((cap_F) * 1000000.0)
#define uF_to_F(cap_uF) ((cap_uF) / 1000000.0)

const uint32_t FAKE_CAPACITANCE_TO_INDICATE_OVER_RANGE = 0xFFFFFFFF - 6;

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

float convert_ctmu_exp_to_uA(int8_t current_value_exponent) {
    if (current_value_exponent == -1) {
        return 0.55;
    }
    else if (current_value_exponent == 0) {
        return 5.5;
    }
    else if (current_value_exponent == 1) {
        return 55.0;
    }
    Disp2String("ERROR: convert_ctmu_exp_to_A() called with invalid value\n");
    return 0.0;
}

float convert_ctmu_exp_to_A(int8_t current_value_exponent) {
    if (current_value_exponent == -1) {
        return 0.55e-6;
    }
    else if (current_value_exponent == 0) {
        return 5.5e-6;
    }
    else if (current_value_exponent == 1) {
        return 55.0e-6;
    }
    Disp2String("ERROR: convert_ctmu_exp_to_A() called with invalid value\n");
    return 0.0;
}

void disable_ctmu_and_pull_pin_low() {
    // RB13
    
    CTMUCONbits.CTMUEN = 0; // enable
    
    // write the pin low
    TRISBbits.TRISB13 = 0; // Set pin as output (0)
    LATBbits.LATB13 = 0; // set pin state to LOW
}

#if 0
uint32_t c_sense_2_point_delta_pF_v1() {
    const uint16_t low_discharge_time_at_start_ms = 50;
    const uint16_t charge_time_ms = 10;
    const uint8_t ctmu_exp_val = 1; // -1=0.55, 0=5.5uA, 1=55uA
    
    // first, disable the CTMU and pull the pin low to discharge the cap
    const uint16_t extra_adc_val_0 = read_adc_value();
    
    disable_ctmu_and_pull_pin_low();
    delay32_ms(low_discharge_time_at_start_ms);
    init_adc(); // must re-init after disable_ctmu_and_pull_pin_low()
    
    const uint16_t pre_ctmu_adc_val = read_adc_value();
    
    // set the CTMU, and wait a bit for it to charge the cap
    init_ctmu(ctmu_exp_val);
    delay32_ms(10); // try waiting for CTMU to get going
    
    const uint16_t start_adc_val = read_adc_value();
    delay32_ms(charge_time_ms); // delay while capacitor charges from CTMU  
    const uint16_t end_adc_val = read_adc_value();
    
    const uint32_t pre_ctmu_adc_val_mV = (uint32_t) adc_val_to_mV(pre_ctmu_adc_val);
    const uint32_t start_adc_val_mV = (uint32_t) adc_val_to_mV(start_adc_val);
    const uint32_t end_adc_val_mV = (uint32_t) adc_val_to_mV(end_adc_val);
    
    const int32_t delta_mV = end_adc_val_mV - start_adc_val_mV;
    if (delta_mV <= 0) {
        // cannot have 0 in the denom, so early return
        Disp2String("DEBUG: delta_mV<=0, so can't divide\n");
        return 0;
    }
    const float cap_F = convert_ctmu_exp_to_A(ctmu_exp_val) * ((float) charge_time_ms) / ((float) delta_mV) / 1000.0; // i * dt/dV, note t=msec, V=mV
    const uint32_t cap_pF = (uint32_t) F_to_pF(cap_F);

    char msg[255];
    sprintf(msg, "DEBUG: extra_adc_val_0=%d=%dmV, pre_ctmu_adc=%d=%lumV, start_adc=%d=%lumV, end_adc=%d=%lumV, delta_mV=%lu, cap=%lup=%lun=%luu\n",
            extra_adc_val_0, adc_val_to_mV(extra_adc_val_0),
            pre_ctmu_adc_val, pre_ctmu_adc_val_mV,
            start_adc_val, start_adc_val_mV,
            end_adc_val, end_adc_val_mV,
            delta_mV,
            cap_pF,
            ((uint32_t) F_to_nF(cap_F)),
            ((uint32_t) F_to_uF(cap_F))
        );
    Disp2String(msg);
    return cap_pF;
}
#endif


uint32_t c_sense_2_point_delta_pF_configurable(
    uint16_t charge_time_ms,
    int8_t ctmu_exp_val
) {
    // first, disable the CTMU and pull the pin low to discharge the cap
    // const uint16_t extra_adc_val_0 = read_adc_value();
    
    disable_ctmu_and_pull_pin_low();
    init_adc(); // must re-init after disable_ctmu_and_pull_pin_low()

    // while it's pulled to ground, wait for it to discharge
    uint32_t discharge_time_occupied_ms;
    for (discharge_time_occupied_ms = 0;
            discharge_time_occupied_ms < 100;
            discharge_time_occupied_ms++) {
        if (read_adc_value() < 10) {
            break;
        }
        delay32_ms(1);
    }
    
    // double-check that it's actually discharged
    const uint16_t pre_ctmu_adc_val = read_adc_value();
    
    // set the CTMU, and wait a bit for it to charge the cap
    init_ctmu(ctmu_exp_val);
    delay32_ms(10); // try waiting for CTMU to get going
    
    const uint16_t start_adc_val = read_adc_value();
    delay32_ms(charge_time_ms); // delay while capacitor charges from CTMU  
    const uint16_t end_adc_val = read_adc_value();
    
    const uint32_t pre_ctmu_adc_val_mV = (uint32_t) adc_val_to_mV(pre_ctmu_adc_val);
    const uint32_t start_adc_val_mV = (uint32_t) adc_val_to_mV(start_adc_val);
    const uint32_t end_adc_val_mV = (uint32_t) adc_val_to_mV(end_adc_val);
    
    const int32_t delta_mV = end_adc_val_mV - start_adc_val_mV;
    uint32_t cap_pF;
    if (delta_mV <= 10) { // require at least 10 mV for it to be "valid"
        // cannot have 0 in the denom, so early return
        // Disp2String("DEBUG: delta_mV<=0, so can't divide\n");
        cap_pF = FAKE_CAPACITANCE_TO_INDICATE_OVER_RANGE;
    }
    else {
        // NOTE: the 1000.0 multiplier is a mystery
        const float cap_F = convert_ctmu_exp_to_A(ctmu_exp_val) * ((float) charge_time_ms) / ((float) delta_mV) / 1000.0; // i * dt/dV, note t=msec, V=mV
        cap_pF = (uint32_t) F_to_pF(cap_F);
    }

    char msg[150];
    sprintf(msg, "DEBUG (deep): discharge_time=%lums, pre_ctmu_adc=%d=%lumV, start_adc=%d=%lumV, end_adc=%d=%lumV, delta_mV=%lu, cap=%lup=%lun=%luu\n",
            
            // extra_adc_val_0=%d=%dmV,
            // extra_adc_val_0, adc_val_to_mV(extra_adc_val_0),
            
            discharge_time_occupied_ms,
            pre_ctmu_adc_val, pre_ctmu_adc_val_mV,
            start_adc_val, start_adc_val_mV,
            end_adc_val, end_adc_val_mV,
            delta_mV,
            cap_pF,
            ((uint32_t) (cap_pF / 1000)),
            ((uint32_t) (cap_pF / 1000000))
        );
    Disp2String(msg);
    return cap_pF;
}

uint32_t c_sense_2_point_delta_pF() {
    // start around good for 1nF, and then adjust up or down
    int8_t ctmu_exp_val = 0;
    uint16_t charge_time_ms = 10;
    
    uint32_t cap_pF = 0;
    uint8_t retry_num = 0;

    for (retry_num = 0; retry_num < 15; retry_num++) {
        cap_pF = c_sense_2_point_delta_pF_configurable(charge_time_ms, ctmu_exp_val);

        // print out all the info (debugging only)
        char msg[150];
        sprintf(
            msg,
            "DEBUG: retry_num=%d, cap=%lup=%lun=%luu, ctmu_exp_val=%d, charge_time_ms=%d\n",
            retry_num,
            cap_pF,
            ((uint32_t) (cap_pF / 1000)),
            ((uint32_t) (cap_pF / 1000000)),
            ctmu_exp_val, charge_time_ms
        );
        Disp2String(msg);

        if (cap_pF == FAKE_CAPACITANCE_TO_INDICATE_OVER_RANGE) {
            // try again with a higher current, if we can
            // otherwise, add more time
            if (ctmu_exp_val < 1) {
                ctmu_exp_val++;
            }
            else if (charge_time_ms < 100) {
                charge_time_ms *= 2;
            }
            else {
                break; // give up, and return the OVER_RANGE value
            }
        }
        else if (cap_pF <= 10) {
            // try again with a lower current, if we can
            // otherwise, try will less time
            if (ctmu_exp_val > -1) {
                ctmu_exp_val--;
            }
            else if (charge_time_ms > 1) {
                charge_time_ms /= 2;
            }
            else {
                break; // give up, and return the too-low value
            }
        }
        else {
            // success, value is in range
            break;
        }
    }
    return cap_pF;
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

