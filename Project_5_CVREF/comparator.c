/*
 * File:   comparator.c
 * Author: user
 *
 * Created on March 18, 2024, 6:19 PM
 */


#include "xc.h"
#include "comparator.h"
#include "uart.h"

#include <string.h>
#include <stdio.h>
#include <math.h> // for ceil/floor

void init_cvref(float vref) {
    // Sets the Pin 17 CVREF value (recall, total 20 pins)
    const float rsrc_val = 3.3; // supply voltage
    
    // Choose CVRR based on vref
    uint8_t cvrr_val = 0; // 1 bit (0 or 1)
    uint8_t cvr_val = 10; // 4 bits (values 0-15)
    
    // Calculate CVR3:0 to get the correct voltage references
    
    // Method:
    //   1. Calculate the closest/best value for CVR for each CVRR case
    //   2. Calculate the theoretical CVREF output value for that scenario
    //   3. Pick whichever is closer to vref
    // Notation:
    //   1. case0: cvrr_val = 0
    //   2. case1: cvrr_val = 1
    
    const float cvr_case0_f = (vref - (rsrc_val / 4.0)) * 32.0 / rsrc_val;
    const float cvr_case1_f = (vref * 24.0 / rsrc_val);
    
    // round the values: int n = (d - floor(d) > 0.5) ? ceil(d) : floor(d);
    const uint8_t cvr_case0 = (cvr_case0_f - floor(cvr_case0_f) > 0.5) ? ceil(cvr_case0_f) : floor(cvr_case0_f);
    const uint8_t cvr_case1 = (cvr_case1_f - floor(cvr_case1_f) > 0.5) ? ceil(cvr_case1_f) : floor(cvr_case1_f);
    
    const float vref_set_case0 = (rsrc_val / 4.0) + ((float)cvr_case0 / 32.0 * rsrc_val);
    const float vref_set_case1 = ((float)cvr_case1 / 24.0) * rsrc_val;
    
    if (cvr_case1 > 15) {
        // have to force case0, which is capable of approximating a little higher
        cvrr_val = 0; // case0
        cvr_val = cvr_case0;
    }
    else if (fabs(vref_set_case0 - vref) < fabs(vref_set_case1 - vref)) {
        // case0 wins
        cvrr_val = 0; // case0
        cvr_val = cvr_case0;
    }
    else {
        // case0 wins
        cvrr_val = 1; // case1
        cvr_val = cvr_case1;
    }
    
    // restrict the max value
    if (cvr_val > 15) {
        cvr_val = 15;
    }
    
    // Output the voltage reference to Cvref (PIN17) on the PIC 24F
    // (verify with scope or multimeter)
    CVRCONbits.CVREN = 1;
    CVRCONbits.CVROE = 1;
    CVRCONbits.CVRR = cvrr_val;
    CVRCONbits.CVRSS = 0;
    CVRCONbits.CVR = cvr_val;
    
    // forwards-calculate the cvref_set again, just to confirm
    float cvref_set = -1;
    if (cvrr_val == 1) {
        cvref_set = ((float)cvr_val / 24.0) * rsrc_val;
    }
    else if (cvrr_val == 0) {
        cvref_set = (rsrc_val / 4.0) + ((float)cvr_val / 32.0 * rsrc_val);
    }
    
    // Display the CVR and CVRR value selected by your code on the PC terminal.
    char msg[200];
    sprintf(msg, "init_cvref(vref_target=%f) -> CVR=%d, CVRR=%d -> vref_set=%f\n",
            vref, cvr_val, cvrr_val, cvref_set);
    Disp2String(msg);
}

