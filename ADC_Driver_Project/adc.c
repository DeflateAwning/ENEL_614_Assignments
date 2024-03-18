/*
 * File:   adc.c
 *
 * Created on March 18, 2024, 5:22 PM
 */


#include "xc.h"
#include "adc.h"
#include "delay.h"

void init_adc(void) {
    // Configure ADC module
    AD1PCFG = 0xFFFE; // AN5 enabled as analog input (RA3/AN5)
    AD1CON1 = 0x00E0; // ADC Module off, Manual Sample, auto-conversion
    AD1CHS = 0x0005;  // Connect AN5 as S/H+ input
    AD1CSSL = 0;      // No scanning required
    AD1CON3 = 0x1F02; // Tad = 128*Tcy, Tad = 8*Tcy
    AD1CON2 = 0;      // No interrupts, AVdd and AVss as reference voltages
    
    // Turn on ADC module
    AD1CON1bits.ADON = 1; // Enable ADC module
}

uint16_t read_adc_value(void) {
    AD1CON1bits.SAMP = 1; // Start sampling
//    __delay_us(10); // Sampling time (depends on the specific requirements)
    delay32_us(1000);
    
    AD1CON1bits.SAMP = 0; // End sampling, start conversion
    
    while (!AD1CON1bits.DONE); // Wait for conversion to complete
    return ADC1BUF0; // Return the ADC conversion result
}
