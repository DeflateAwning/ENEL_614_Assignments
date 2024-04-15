/*
 * File:   set_clock.c

 *
 * Created on January 22, 2024, 5:54 PM
 */


#include "xc.h"
#include "clock.h"

// set global store (extern)
uint16_t active_clk_freq_khz = 0;

//clk_freq_khz = 8000 for 8MHz;
//clk_freq_khz = 500 for 500kHz;
//clk_freq_khz = 32 for 32kHz;
void set_clock_freq(uint16_t clk_freq_khz)
{
    uint8_t COSCNOSC;
    if (clk_freq_khz == 8000)  // 8 MHz = 8000 kHz
    {
        COSCNOSC = 0x00;
        active_clk_freq_khz = 8000;
    }
    else if (clk_freq_khz == 500) // 500 kHz
    {
        COSCNOSC = 0x66;
        active_clk_freq_khz = 500;
    }
    else if (clk_freq_khz == 32) // 32 kHz
    {
        COSCNOSC = 0x55;
        active_clk_freq_khz = 32;
    }
    else // default 32 kHz
    {
        COSCNOSC = 0x55;
        active_clk_freq_khz = 32;
    }
    
    // Switch clock to new frequency
    SRbits.IPL = 7;  //Disable interrupts
    CLKDIVbits.RCDIV = 0;  // CLK division = 0
    __builtin_write_OSCCONH(COSCNOSC);   // (0x00) for 8MHz; (0x66) for 500kHz; (0x55) for 32kHz;
    __builtin_write_OSCCONL(0x01);
    OSCCONbits.OSWEN=1;
    while(OSCCONbits.OSWEN==1) {} 
    SRbits.IPL = 0;  //enable interrupts
}
