/*
 * File:   adc.c
 *
 * Created on March 18, 2024, 5:22 PM
 */


#include "xc.h"
#include "adc.h"
#include "delay.h"

void init_adc(void) {
    TRISBbits.TRISB13 = 1; // AN11/RB13 as INPUT
    
    AD1CON1bits.ADON = 1; // Enable ADC module
    AD1CON1bits.ADSIDL = 0; // continue module operation during idle
    AD1CON1bits.FORM = 0b00; // save as 10-bit unsigned number
    
    // AD1CON1bits.SSRC = 0b000; // clearing SAMP bit ends sampling and starts conversion (0b000 for CTMU, 0b111 for general ADC driver)
     AD1CON1bits.ASAM = 0; // sampling begins when SAMP bit is set
    
    AD1CON1bits.SSRC = 0b111; // clearing SAMP bit ends sampling and starts conversion (0b000 for CTMU, 0b111 for general ADC driver)
//    AD1CON1bits.ASAM = 1; // sampling begins automatically, when the last conversion finishes
    
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
    AD1CHSbits.CH0SA = 0b1011; // select AN11 channel (same port as CTMU)
    
    AD1PCFG = 0xFF; // preset all to 1
    AD1PCFGbits.PCFG11 = 0; // analog mode
    
    
    // was in ADC scan, but not here
    AD1CSSLbits.CSSL11 = 0; // analog channel omitted from input scan
}

uint16_t read_adc_value(void) {
    // Enable ADC module
    AD1CON1bits.ADON = 1;
    
    AD1CON1bits.SAMP = 1; // Start sampling
    while (!AD1CON1bits.DONE); // Wait for conversion to complete // not needed for CTMU mode (SSRC=0b000), maybe
    
    const uint16_t adc_value = ADC1BUF0;
    
    AD1CON1bits.SAMP = 0; // End sampling
    AD1CON1bits.ADON = 0; // Turn off ADC - saves power
    
    return adc_value;
}
