#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "tetris.h"  
#include <stdio.h>
#include <string.h>
#include <math.h>

// Function to get the status of toggle switches SW4-SW1
int getsw(void) {
    // Read switches SW4-SW1 (bits 11-8 of PORTD)
    return (PORTD >> 8) & 0x000F; // Return the four least significant bits
}

// Function to get the status of push buttons BTN4, BTN3, and BTN2
int getbtns(void) {
    // Read buttons BTN4, BTN3, and BTN2 (bits 7, 6, and 5 of PORTD)
    return (PORTD >> 5) & 0x0007; // Return the three least significant bits
}

