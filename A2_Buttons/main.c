
#include "xc.h"
#include "clock.h"
#include "uart.h"
#include "timer.h"
#include "io.h"
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

int main(void) {
    // Clock output on REFO
    TRISBbits.TRISB15 = 0;  // Set RB15 as output for REFO (DIP PIN 18)
    REFOCONbits.ROEN = 1; // Ref oscillator is enabled
    REFOCONbits.ROSSLP = 0; // Ref oscillator is disabled in sleep
    REFOCONbits.ROSEL = 0; // Output base clk showing clock switching
    REFOCONbits.RODIV = 0b0000;
    
    AD1PCFG = 0xFFFF; // disable analog inputs (incl. Pin 7)

//     set_clock_freq(8000); // 8000 kHz => 9600 Baud
//     set_clock_freq(32); // 32 kHz => 300 Baud
     set_clock_freq(500); // 500 kHz => 4800 Baud
//    set_clock_freq(32);
    
    InitUART2();
    
    // Set LED as Output
    TRISBbits.TRISB8 = 0;
    LATBbits.LATB8 = 1; // set init LED state
    
    delay_ms(1000);
    
    init_io_inputs();
    cn_init();
    
//    while(1) {} // pause forever
    
    uint16_t loop_count = 0;
    
    const uint8_t ENABLE_DEBUG = 0;
    
    
    // Req 1: Wakes up the PIC from idle or sleep when push buttons tied to:
    // RB4/CN1, RA4/CN0, RA2/CN30
    
    // Req 2: Displays the status of the push buttons on Teraterm window when one or more buttons
    // are pushed, i.e "CN1/RB4 is pressed" or "CN0/RA4 is pressed" or "CN1/RB4 and
    // CN0/RA4 are pressed". Do this for all button-press states.
    
    uint8_t last_sw_state = sw_state_as_int();
    
    if (ENABLE_DEBUG)
    Disp2String("DEBUG: Starting while(1)\n");
    
    while (1) {
        if (ENABLE_DEBUG)
            Disp2String("DEBUG: Top of while(1)\n");
        
//        LATBbits.LATB8 = 1; // turn LED on
//        delay_ms(500);
//        LATBbits.LATB8 = 0; // turn LED off
//        delay_ms(500);
        
        // light up LED if any button is pressed
//        LATBbits.LATB8 = is_any_sw_pressed();
        
        uint8_t cur_sw_state = sw_state_as_int();
        
        char msg[255];
        msg[0] = 0; // clear str
        
        if ((cur_sw_state != last_sw_state)) {
            // debouncing delay
            delay_ms(75);
            
            uint8_t pressed_sw_count = 0;
            
            if (is_sw_pressed(PIN_RA4_CN0)) {
                strcat(msg + strlen(msg), "CN0/RA4 ");
                pressed_sw_count++;
            }
            if (is_sw_pressed(PIN_RB4_CN1)) {
                if (pressed_sw_count > 0)
                    strcat(msg + strlen(msg), "and ");
                strcat(msg + strlen(msg), "CN1/RB4 ");
                pressed_sw_count++;
            }
            if (is_sw_pressed(PIN_RA2_CN30)) {
                if (pressed_sw_count > 0)
                    strcat(msg + strlen(msg), "and ");
                strcat(msg + strlen(msg), "RA2/CN30 ");
                pressed_sw_count++;
            }
            
            if (pressed_sw_count > 1) {
                strcat(msg + strlen(msg), "are pressed.\n");
            }
            else if (pressed_sw_count == 1) {
                strcat(msg + strlen(msg), "is pressed.\n");
            }
            else {
                // do nothing, pressed_sw_count = 0
            }
            
            
            
            if (ENABLE_DEBUG) {
                char test_msg[255];
                sprintf(
                        test_msg,
                        "DEBUG: loop=%d, PIN_RA4_CN0=%d, PIN_RB4_CN1=%d, PIN_RA2_CN30=%d, last_sw_state=%d, cur_sw_state=%d\n",
                        loop_count++,
                        is_sw_pressed(PIN_RA4_CN0),
                        is_sw_pressed(PIN_RB4_CN1),
                        is_sw_pressed(PIN_RA2_CN30),
                        last_sw_state,
                        cur_sw_state
                );
                Disp2String(test_msg);
            }

            // set the last state to the state now, after debouncing
            // (because if the user pressed 2 buttons, must recheck which buttons are settled on)
            last_sw_state = sw_state_as_int();
        }
        
        if (strlen(msg) > 2) {
            Disp2String(msg);
        }
        
    }
    
    return 0;
}
