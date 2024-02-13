/*
 * File:   main.c
 * Author: user
 *
 * Created on January 22, 2024, 5:54 PM
 */


#include "xc.h"
#include "clock.h"
#include "uart.h"
#include "timer.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

// CLOCK CONTROL 
#pragma config IESO = OFF    // 2 Speed Startup disabled
#pragma config FNOSC = FRC  // Start up CLK = 8 MHz
#pragma config FCKSM = CSECMD // Clock switching is enabled, clock monitor disabled
#pragma config SOSCSEL = SOSCLP // Secondary oscillator for Low Power Operation
#pragma config POSCFREQ = MS  //Primary Oscillator/External clk freq betwn 100kHz and 8 MHz. Options: LS, MS, HS
#pragma config OSCIOFNC = ON  //CLKO output disabled on pin 8, use as IO. 
#pragma config POSCMOD = NONE  // Primary oscillator mode is disabled

int main(void) {
    // Clock output on REFO
    TRISBbits.TRISB15 = 0;  // Set RB15 as output for REFO (DIP PIN 18)
    REFOCONbits.ROEN = 1; // Ref oscillator is enabled
    REFOCONbits.ROSSLP = 0; // Ref oscillator is disabled in sleep
    REFOCONbits.ROSEL = 0; // Output base clk showing clock switching
    REFOCONbits.RODIV = 0b0000;

    // set_clock_freq(8000); // 8000 kHz => 9600 Baud
    // set_clock_freq(32); // 32 kHz => 300 Baud
    // set_clock_freq(500); // 500 kHz => 4800 Baud
    set_clock_freq(32);
    
//    InitUART2(); // FIXME: re-enable if used
    
    // Set LED as Output
    TRISBbits.TRISB8 = 0;
    LATBbits.LATB8 = 0; // set init LED state
    
//    while(1) {} // pause forever
    
    uint16_t loop_count = 0;
    
    while (1) {
        LATBbits.LATB8 = 1; // turn LED on
        delay_ms(500);
        LATBbits.LATB8 = 0; // turn LED off
        delay_ms(500);
    }

    while(1) {
        char test_msg[255];
        sprintf(
            test_msg,
            "Hello world %d\n",
            loop_count++
        );
        
        Disp2String(test_msg);
    
    }
    
    return 0;
}
