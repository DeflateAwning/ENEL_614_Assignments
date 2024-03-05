
#include "xc.h"
#include "clock.h"
#include "uart.h"
#include "timer.h"
#include "io.h"
#include "ir_transmit.h"
#include "delay.h"

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

typedef enum {
    VOL_CH_MODE_VOLUME,
    VOL_CH_MODE_CHANNEL
} VOL_CH_MODE_t;

int main(void) {
    // Clock output on REFO
    TRISBbits.TRISB15 = 0;  // Set RB15 as output for REFO (DIP PIN 18)
    REFOCONbits.ROEN = 1; // Ref oscillator is enabled
    REFOCONbits.ROSSLP = 0; // Ref oscillator is disabled in sleep
    REFOCONbits.ROSEL = 0; // Output base clk showing clock switching
    REFOCONbits.RODIV = 0b0000;
    
    AD1PCFG = 0xFFFF; // disable analog inputs (incl. Pin 7)

    set_clock_freq(8000); // 8000 kHz => 9600 Baud
    // at 8MHz, each clock period is 1.25e-7 sec = 0.125 us
     
     // Other options (won't work):
//     set_clock_freq(32); // 32 kHz => 300 Baud
//     set_clock_freq(500); // 500 kHz => 4800 Baud
//    set_clock_freq(32);
    
    InitUART2();
    
    // init GPIO
    TRISBbits.TRISB8 = 0; // Set LED as Output
    TRISBbits.TRISB9 = 0; // set IR LED state as output
    
    LATBbits.LATB8 = 1; // set init LED state
    LATBbits.LATB9 = 1; // set init IR LED state
    
    ir_set_led_state(0); // turn off to begin
    
    delay32_ms(1000);
    
//    init_io_inputs();
//    cn_init();
    
//    while(1) {} // pause forever
    
    uint16_t loop_count = 0;
    
    const uint8_t ENABLE_DEBUG = 1;
    
    
    // Req 1: Wakes up the PIC from idle or sleep when push buttons tied to:
    // RB4/CN1, RA4/CN0, RA2/CN30
    
    // Req 2: Displays the status of the push buttons on Teraterm window when one or more buttons
    // are pushed, i.e "CN1/RB4 is pressed" or "CN0/RA4 is pressed" or "CN1/RB4 and
    // CN0/RA4 are pressed". Do this for all button-press states.
    
    uint8_t last_sw_state = sw_state_as_int();
    
//    if (ENABLE_DEBUG)
    Disp2String("DEBUG: Starting while(1)\n");
    
    const uint32_t DEBOUNCE_DELAY_MS = 75;
    const uint32_t LOOP_DELAY_MS = 75;
    uint32_t ms_count_1_and_2_pressed = 0;
    VOL_CH_MODE_t vol_ch_mode = VOL_CH_MODE_VOLUME;
    
    // DEBUG: blink LED
    while (1) {
        ir_set_led_state(1);
        LATBbits.LATB8 = 1;
        delay32_ms(50);
        ir_set_led_state(0);
        LATBbits.LATB8 = 0;
        delay32_ms(50);
    }
    
    while (1) {
        Disp2String("DEBUG: start send IR_BIT_0 \n");
        ir_tx_single_bit(IR_BIT_0);
        Disp2String("DEBUG: done send IR_BIT_0 \n");
        
//        Disp2String("DEBUG: start send IR_BIT_1 \n");
//        ir_tx_single_bit(IR_BIT_1);
//        Disp2String("DEBUG: done send IR_BIT_1 \n");
//        
//        
//        Disp2String("DEBUG: start send IR_BIT_START \n");
//        ir_tx_single_bit(IR_BIT_START);
//        Disp2String("DEBUG: done send IR_BIT_START \n");
//        
//        
//        Disp2String("DEBUG: start send POWER_ON_OFF \n");
//        ir_tx_32_bit_code(IR_CODE_POWER_ON_OFF);
//        Disp2String("DEBUG: done send POWER_ON_OFF \n");
    }
    
    while (1) {
        if (ENABLE_DEBUG && 0)
            Disp2String("DEBUG: Top of while(1)\n");
        
//        LATBbits.LATB8 = 1; // turn LED on
//        delay32_ms(500);
//        LATBbits.LATB8 = 0; // turn LED off
//        delay32_ms(500);
        
        // light up LED if any button is pressed
//        LATBbits.LATB8 = is_any_sw_pressed();
        
        uint8_t cur_sw_state = sw_state_as_int();
        
        char msg[255];
        msg[0] = 0; // clear str
        
        // Let PB1 = PIN_RA4_CN0
        // Let PB2 = PIN_RB4_CN1
        
        
        if ((cur_sw_state != last_sw_state)) {
            // debouncing delay
            delay32_ms(DEBOUNCE_DELAY_MS);
            
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
            
            if (((last_sw_state & 0b110) > 0) && (cur_sw_state == 0)) {
                // if PB1 and PB2 were both pressed last time, and now no switches are pressed
                
                if (ms_count_1_and_2_pressed > 3000) {
                    Disp2String("PB1 and PB2 for >3 sec - POWER\n");
                    
                    ir_tx_32_bit_code(IR_CODE_POWER_ON_OFF);
                }
                
                else {
                    // <3sec press, so toggle the mode
                    if (vol_ch_mode == VOL_CH_MODE_CHANNEL) {
                        vol_ch_mode = VOL_CH_MODE_VOLUME;
                        Disp2String("Switched to VOLUME mode.\n");
                    }
                    else if (vol_ch_mode == VOL_CH_MODE_VOLUME) {
                        vol_ch_mode = VOL_CH_MODE_CHANNEL;
                        Disp2String("Switched to CHANNEL mode.\n");
                    }
                    else {
                        // safe default
                        vol_ch_mode = VOL_CH_MODE_VOLUME;
                        Disp2String("ERROR! Unknown vol_ch_mode value.\n");
                    }
                }
            }
            
            if (pressed_sw_count == 1) {
                // only a single switch is pressed, do whatever that action is
                
                if (is_sw_pressed(PIN_RA4_CN0)) { // "PB1" pressed: UP
                    if (vol_ch_mode == VOL_CH_MODE_VOLUME) {
                        ir_tx_32_bit_code(IR_CODE_VOLUME_UP);
                        Disp2String("VOLUME UP.\n");
                    }
                    else if (vol_ch_mode == VOL_CH_MODE_CHANNEL) {
                        ir_tx_32_bit_code(IR_CODE_CHANNEL_UP);
                        Disp2String("CHANNEL UP.\n");
                    }
                }
                
                else if (is_sw_pressed(PIN_RB4_CN1)) { // "PB2" pressed: DOWN
                    if (vol_ch_mode == VOL_CH_MODE_VOLUME) {
                        ir_tx_32_bit_code(IR_CODE_VOLUME_DOWN);
                        Disp2String("VOLUME DOWN.\n");
                    }
                    else if (vol_ch_mode == VOL_CH_MODE_CHANNEL) {
                        ir_tx_32_bit_code(IR_CODE_CHANNEL_DOWN);
                        Disp2String("CHANNEL DOWN.\n");
                    }
                }
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
        
        // if PB1 and PB2 both pressed, then increase the timer count
        if (is_sw_pressed(PIN_RA4_CN0) && is_sw_pressed(PIN_RB4_CN1)) {
            ms_count_1_and_2_pressed += DEBOUNCE_DELAY_MS;
            delay32_ms(LOOP_DELAY_MS);
        }
    }
    
    return 0;
}
