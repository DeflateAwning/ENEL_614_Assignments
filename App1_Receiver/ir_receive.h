/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICUAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */


// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __INCLUDE_GUARD__IR_TRANSMIT_H__
#define	__INCLUDE_GUARD__IR_TRANSMIT_H__

#include "xc.h"
#include "clock.h"
#include "timer.h"

#define IR_CODE_POWER_ON_OFF   (0xE0E040BFU)
#define IR_CODE_CHANNEL_UP     (0xE0E048B7U)
#define IR_CODE_CHANNEL_DOWN   (0xE0E008F7U)
#define IR_CODE_VOLUME_UP      (0xE0E0E01FU)
#define IR_CODE_VOLUME_DOWN    (0xE0E0D02FU)

typedef enum {
    IR_BIT_0 = 0,
    IR_BIT_1 = 1,
    IR_BIT_START = 2
} ir_bit_enum_t;

void ir_tx_single_start();
void ir_tx_single_bit_0();
void ir_tx_single_bit_1();

void ir_tx_init();
void ir_tx_reset();
void ir_set_led_state(uint8_t en);

void ir_tx_32_bit_code(uint32_t code);

void debug_print_carrier_log(const uint8_t carrier_detect_log[], uint32_t carrier_detect_log_len);
uint32_t parse_carrier_log_to_code(const uint8_t carrier_detect_log[], uint32_t carrier_detect_log_len);


#endif	/* __INCLUDE_GUARD__IR_TRANSMIT_H__ */
