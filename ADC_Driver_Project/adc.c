/*
 * File:   adc.c
 *
 * Created on March 18, 2024, 5:22 PM
 */


#include "xc.h"
#include "adc.h"
#include "delay.h"

void init_adc(void) {
    AD1CON1bits.ADON = 1; // Enable ADC module
    
    AD1CON1bits.SSRC = 0b111;
    
    AD1CON1bits.ASAM = 0;
    
    AD1CON2bits.VCFG = 0b000; // voltage range VDD to VSS
    
    AD1CON2bits.CSCNA = 0; // do not scan inputs
    
    AD1CON2bits.BUFM = 0; // buffer holds a single 16-bit value
    
    AD1CON2bits.ALTS = 0; // always use Mux A
    
    // Configure the ADC’s sample time by setting bits in AD1CON3 shown in slide 17
    AD1CON3bits.ADRC = 0; // system clock
    
    AD1CON3bits.SAMC = 0b11111;
    AD1CON3bits.ADCS = 0b11111;
    
    // General note: Ensure sample time is 1/10th of signal being sampled or as per application’s speed and needs
    
    AD1CHSbits.CH0NA = 0;
    AD1CHSbits.CH0SA = 0b0101; // select AN5 channel
    
    AD1PCFGbits.PCFG5 = 0; // analog mode, ADC port disabled
    
    AD1CSSLbits.CSSL5 = 0; // analog channel omitted from input scan
    
}

uint16_t read_adc_value(void) {
    // Enable ADC module
    AD1CON1bits.ADON = 1;
    
    
    AD1CON1bits.SAMP = 1; // Start sampling
    while (!AD1CON1bits.DONE); // Wait for conversion to complete
    
    const uint16_t adc_value = ADC1BUF0;
    
    AD1CON1bits.SAMP = 0; // End sampling
    AD1CON1bits.ADON = 0; // Turn off ADC - saves power
    
    return adc_value;
}
