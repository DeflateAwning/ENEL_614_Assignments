
#include "xc.h"
#include "clock.h"
#include "uart.h"
#include "timer.h"
#include "io.h"
#include "ir_receive.h"
#include "delay.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define UINT32_T_MAX_VALUE 0xFFFFFFFF
#define UINT16_T_MAX_VALUE 0xFFFF
#define UINT8_T_MAX_VALUE 0xFF

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
    // at 8MHz, each clock period is 1.25e-7 sec = 0.125 us
     
     // Other options (won't work):
//     set_clock_freq(32); // 32 kHz => 300 Baud
//     set_clock_freq(500); // 500 kHz => 4800 Baud
//    set_clock_freq(32);
    
    InitUART2();
    
    // init debugging LED
    TRISBbits.TRISB8 = 0; // Set LED as Output
    LATBbits.LATB8 = 1; // set init LED state
    
    // init IR LED
    // TRISBbits.TRISB9 = 0; // set IR LED state as output
    //LATBbits.LATB9 = 1; // set init IR LED state
    // ir_set_led_state(0); // turn off to begin
    
    delay32_ms(1000);
    
    init_io_inputs();
    cn_init();
    
    // while(1) {} // pause forever
    
    uint16_t loop_count = 0;
    
    const uint8_t ENABLE_DEBUG = 1;
    
    
    // Req 1: Wakes up the PIC from idle or sleep when push buttons tied to:
    // RB4/CN1, RA4/CN0, RA2/CN30
    
    // Req 2: Displays the status of the push buttons on Teraterm window when one or more buttons
    // are pushed, i.e "CN1/RB4 is pressed" or "CN0/RA4 is pressed" or "CN1/RB4 and
    // CN0/RA4 are pressed". Do this for all button-press states.
    
    uint8_t last_sw_state = sw_state_as_int();
    
    
    const uint32_t DEBOUNCE_DELAY_MS = 75;
    const uint32_t LOOP_DELAY_MS = 75;
    uint8_t last_ir_rx_state = get_ir_rx_state();
    
    // DEBUG: blink LED
//    while (1) {
//        ir_set_led_state(1);
//        LATBbits.LATB8 = 1;
//        delay32_ms(50);
//        ir_set_led_state(0);
//        LATBbits.LATB8 = 0;
//        delay32_ms(50);
//    }
    
    // DEBUG: send a single type of output on repeat
//    while (1) {
//        ir_tx_single_bit_0();
//        ir_tx_single_bit_1();
//        ir_tx_single_start();
//        ir_tx_32_bit_code(IR_CODE_POWER_ON_OFF);
//    }
    
//    if (ENABLE_DEBUG)
    Disp2String("DEBUG: Starting while(1)\n");
    
    while (1) {
        if (ENABLE_DEBUG) // TODO: make it && 0 for final version   
            Disp2String("DEBUG: Top of while(1)\n");
        
//        LATBbits.LATB8 = 1; // turn LED on
//        delay32_ms(500);
//        LATBbits.LATB8 = 0; // turn LED off
//        delay32_ms(500);
        
        // carrier detect log represents the state of the envelope, each in a period of ~200us
        const uint32_t carrier_detect_log_len = 1000;
        uint8_t carrier_detect_log[carrier_detect_log_len]; // newest at 0
        
        uint8_t last_is_carrier_detected = 0;
        uint8_t consec_count_is_carrier_detected = 0;
        uint8_t carrier_detected_count_in_log = 0;
        
        for (uint32_t carrier_detect_log_idx = 0; carrier_detect_log_idx < carrier_detect_log_len; carrier_detect_log_idx++) {
            
            // detect the carrier: init variables
            uint32_t carrier_detect_buffer = 0;
            const uint8_t carrier_detect_buffer_len = 32; // matches carrier_detect_buffer number of bits
            const uint32_t carrier_detect_buffer_delay_per_element_us = 5; 
            // 1 us = 8 clocks, 5 us = 40 clocks
            // 5 us * 32 elements = 160 us (or really, probably closer to 200 us)
            // max duration of a message is:
            //   - 4500us ON carrier
            //   - 4500us OFF carrier
            //   - 32 bits * 560us (ON carrier) = 17920us
            //   - 32 bits * 1690us (OFF carrier) = 54080us
            //   = (4500*2) + (32*560) + (32*1690) = 81000us = 81ms = (405 detects * (200us per detect))

            // detect the carrier: the answer is "was the carrier going in the last 200us"
            for (uint8_t i = 0; i < carrier_detect_buffer_len; i++) {
                carrier_detect_buffer = (carrier_detect_buffer << 1) | (get_ir_rx_state());
                
                delay32_us(carrier_detect_buffer_delay_per_element_us);
            }
            
            // carrier is "detected" if it's not all 1's, and it's not all 0's
            const uint8_t is_carrier_detected = ((carrier_detect_buffer > 0) &&
                    (carrier_detect_buffer < UINT32_T_MAX_VALUE));
            
            // add the current is_carrier_detected at carrier_detect_log_idx
            carrier_detect_log[carrier_detect_log_idx] = is_carrier_detected;
            
            carrier_detected_count_in_log += is_carrier_detected;
        }

        uint32_t received_code = 0;

        // 25 is kinda arbitrary, but reasonable: (4500us start bit) / (200us per detect) = 22 detects minimum
        if (carrier_detected_count_in_log > 25) {
            debug_print_carrier_log(carrier_detect_log, carrier_detect_log_len);
            
            // run the parser
            received_code = parse_carrier_log_to_code(carrier_detect_log, carrier_detect_log_len);

            char msg[40];
            sprintf(msg, "Received code: 0x%08X", received_code);
            Disp2String(msg);

            if (received_code == IR_CODE_POWER_ON_OFF) {
                Disp2String(" (POWER_ON_OFF)");
            } else if (received_code == IR_CODE_CHANNEL_UP) {
                Disp2String(" (CHANNEL_UP)");
            } else if (received_code == IR_CODE_CHANNEL_DOWN) {
                Disp2String(" (CHANNEL_DOWN)");
            } else if (received_code == IR_CODE_VOLUME_UP) {
                Disp2String(" (VOLUME_UP)");
            } else if (received_code == IR_CODE_VOLUME_DOWN) {
                Disp2String(" (VOLUME_DOWN)");
            } else {
                Disp2String(" (OTHER)");
            }
            Disp2String("\n\n");
        }
        else {
            // TODO: maybe disable this printing, even during debug
            debug_print_carrier_log(carrier_detect_log, carrier_detect_log_len);
            Disp2String("No code received.\n\n");
        }
    }
    
    return 0;
}
