/*
 * File:   io.c

 *
 * Created on February 26, 2024, 5:17 PM
 */

#include "io.h"

// IR Receiver: RB2/Pin 6/CN6


// keys are PIN_NAME_t enum values; 1 = pressed
volatile uint8_t input_states[4] = {0, 0, 0, 0};

void init_io_inputs(void) {
    
    // Configure RA4/CN0 as input
    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1; // Enable pull-up on CN0 pin
    CNEN1bits.CN0IE = 1; // Enable notification
    
    // Configure RB4/CN1 as input
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1; // Enable pull-up on CN1 pin
    CNEN1bits.CN1IE = 1; // Enable notification

    // Configure RA2/CN30 as input
    TRISAbits.TRISA2 = 1;
    CNPU2bits.CN30PUE = 1; // Enable pull-up on CN30 pin
    CNEN2bits.CN30IE = 1; // Enable notification
    
    // Configure RB2/CN6 as input
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN6PUE = 1; // Enable pull-up on CN1 pin
    CNEN1bits.CN6IE = 1; // Enable notification
}

void cn_init(void) {
    // Configure CNIP (priority)
    IPC4bits.CNIP = 0b110; // 6 out of 7 is high priority, but not top; timers are 7
    
    // Enable Interrupt
    IEC1bits.CNIE = 1;
}

uint8_t is_any_sw_pressed(void) {
    for (uint8_t i = 0; i < (sizeof(input_states) / sizeof(input_states[0])); i++) {
        if (input_states[i]) return 1;
    }
    return 0;
}

uint8_t is_sw_pressed(PIN_NAME_t pin) {
    return input_states[pin];
}

char* pin_name_to_string(PIN_NAME_t pin) {
    switch (pin) {
        case PIN_RA4_CN0:
            return "PIN_RA4_CN0";
        case PIN_RB4_CN1:
            return "PIN_RB4_CN1";
        case PIN_RA2_CN30:
            return "PIN_RA2_CN30";
        case PIN_RB2_CN6:
            return "PIN_RB2_CN6";
        default:
            return "Unknown PIN";
    }
}

uint8_t sw_state_as_int(void) {
    uint8_t val = 0;
    for (uint8_t i = 0; i < (sizeof(input_states) / sizeof(input_states[0])); i++) {
        if (input_states[i]) {
            val |= (input_states[i] << i);
        }
    }
    return val;
}

uint8_t get_ir_rx_state(void) {
    return input_states[PIN_RB2_CN6];
}


///// Change of pin Interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void) {
    if (IFS1bits.CNIF == 1) {
        const uint8_t cur_ir_state = !PORTBbits.RB2;
        
        input_states[PIN_RA4_CN0] = !PORTAbits.RA4;
        input_states[PIN_RB4_CN1] = !PORTBbits.RB4;
        input_states[PIN_RA2_CN30] = !PORTAbits.RA2;
        input_states[PIN_RB2_CN6] = cur_ir_state;
        
        // LATBbits.LATB8 = !LATBbits.LATB8; // DEBUG: toggle light
    }
    IFS1bits.CNIF = 0; // clear IF flag
    Nop();
}

// NOTE:
// LATBbits.LATB4 = 1; // to write to a pin
// if(PORTBbits.RB4 == 1) // to  read a pin
// Common interrupt routine for all CN inputs
// Interrupts triggered for any change in state i.e. hi to lo or lo to hi.
// Interrupts will be triggered for debounces on push buttons too
// Code should filter out debounce effects

