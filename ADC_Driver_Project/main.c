
#include "xc.h"
#include "clock.h"
#include "uart.h"
#include "delay.h"
#include "adc.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

// CLOCK CONTROL 
#pragma config IESO = OFF    // 2 Speed Startup disabled
#pragma config FNOSC = FRC  // Start up CLK = 8 MHz
#pragma config FCKSM = CSECMD // Clock switching is enabled, clock monitor disabled
#pragma config SOSCSEL = SOSCLP // Secondary oscillator for Low Power Operation
#pragma config POSCFREQ = MS  // Primary Oscillator/External clk freq betwn 100kHz and 8 MHz. Options: LS, MS, HS
#pragma config OSCIOFNC = ON  // CLKO output disabled on pin 8, use as IO. 
#pragma config POSCMOD = NONE  // Primary oscillator mode is disabled

// Pin Connections (28 Pins Total):
// - PIN_RB2_CN6 (Pin 6) = IR Receiver
// - RB8 (Pin 17) = debugging LED output

int main(void) {
    // Clock output on REFO
    TRISBbits.TRISB15 = 0;  // Set RB15 as output for REFO (DIP PIN 18)
    REFOCONbits.ROEN = 1; // Ref oscillator is enabled
    REFOCONbits.ROSSLP = 0; // Ref oscillator is disabled in sleep
    REFOCONbits.ROSEL = 0; // Output base clk showing clock switching
    REFOCONbits.RODIV = 0b0000;
    
    AD1PCFG = 0xFFFF; // disable analog inputs (incl. Pin 7)

    set_clock_freq(8000); // 8000 kHz => 9600 Baud
     
    // use 32 kHz to save power
//     set_clock_freq(32); // 32 kHz => 300 Baud
//     set_clock_freq(500); // 500 kHz => 4800 Baud
    
    InitUART2();
    
    init_adc();
    
    // init debugging LED
    TRISBbits.TRISB8 = 0; // Set LED as Output
    LATBbits.LATB8 = 1; // set init LED state
    
    delay32_ms(1000);
    
    
    // while(1) {} // pause forever
    
    uint16_t loop_count = 0;
    
    const uint8_t ENABLE_DEBUG = 1;
    const uint8_t ENABLE_BAR_CHART = 0;
    
    
//    if (ENABLE_DEBUG)
    Disp2String("DEBUG: Starting while(1)\n");
    
    while (1) {
        // Disp2String("DEBUG: Top of while(1)\n");
        
//        LATBbits.LATB8 = 1; // turn LED on
//        delay32_ms(250);
//        LATBbits.LATB8 = 0; // turn LED off
//        delay32_ms(250);
        
        const uint16_t adc_value = read_adc_value();
        
        char msg[200];
        sprintf(msg, "ADC Value: %04d  ", adc_value);
        
        const uint8_t main_msg_len = strlen(msg);
        
        // append a bar graph
        if (ENABLE_BAR_CHART) {
            const uint8_t bar_len = (adc_value / 16) + 1; // max_len = 1024/16 = 64 chars

            memset(msg + main_msg_len, '=', bar_len); // write the bar graph bar
            msg[main_msg_len + bar_len] = '\n';
            msg[main_msg_len + bar_len + 1] = 0; // null terminator
        }
        else {
            // add newline
            msg[main_msg_len] = '\n';
            msg[main_msg_len + 1] = 0; // null terminator
        }
        
        // print the message (ADC value AND bar chart)
        Disp2String(msg);
    }
    return 0;
}
