/*
 * File:   set_clock.c
 * Author: user
 *
 * Created on January 22, 2024, 5:54 PM
 */


#include "xc.h"
#include "clock.h"

// set global store
uint16_t active_clk_freq_khz = 8000;

//clk_freq_khz = 8000 for 8MHz;
//clk_freq_khz = 500 for 500kHz;
//clk_freq_khz = 32 for 32kHz;
void set_clock_freq(uint16_t clk_freq_khz)
{
    uint8_t COSCNOSC;
    if (clk_freq_khz == 8000)  // 8 MHz = 8000 kHz
    {
        COSCNOSC = 0x00;
    }
    else if (clk_freq_khz == 500) // 500 kHz
    {
        COSCNOSC = 0x66;
    }
    else if (clk_freq_khz == 32) // 32 kHz
    {
        COSCNOSC = 0x55;
    }
    else // default 32 kHz
    {
        COSCNOSC = 0x55;
    }
    
    // Switch clock to new frequency
     SRbits.IPL = 7;  //Disable interrupts
     CLKDIVbits.RCDIV = 0;  // CLK division = 0
     __builtin_write_OSCCONH(COSCNOSC);   // (0x00) for 8MHz; (0x66) for 500kHz; (0x55) for 32kHz;
     __builtin_write_OSCCONL(0x01);
     OSCCONbits.OSWEN=1;
     while(OSCCONbits.OSWEN==1)
     {} 
     SRbits.IPL = 0;  //enable interrupts

    // set global store
    active_clk_freq_khz = clk_freq_khz;
}
