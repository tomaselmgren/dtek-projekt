#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "tetris.h"  
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

/* Interrupt Service Routine */
void user_isr( void )
{
  if (IFS(0) & 0x100) {
    IFS(0) = 0;
  } 
}

/* Lab-specific initialization goes here */
void labinit(void) {
  // Initialize Port E so that bits 7 through 0 are outputs
  volatile int* trise = (volatile int*)0xbf886100;
   *trise &= ~0x00ff;

  // Initialize port D so that bits 11 through 5 are inputs
  TRISD = 0x0fe0;

  // Initialize Timer 2 for timeouts every 100 ms
  T2CON = 0x70; // Enable Timer 2, 1:256 prescaler
  PR2 = 31250; // Period register counts down from 3906 at 80 MHz, which is 100 ms
  TMR2 = 0;
  T2CONSET = 0x8000;

  return; 
}

/* This function is called repetitively from the main program */
void labwork( void )
{
  volatile int *porte = (volatile int*) 0xbf886110;
  volatile int *LED = (volatile int*) 0xbf886110; 

  int sw;
  int btn;

  

  // // Test the time-out event flag
  // if (IFS(0) & 0x100) {
  //   // Reset the time-out event flag
  //   IFS(0) = 0;
  //   // Increment the counter
  //   timeoutcount++;
  // } 
  //   sw = getsw();
  //   btn = getbtns();
    
  //   //button 4 (0100)
  //   if(btn == 4){
  //     mytime = mytime & 0x0fff;
  //     mytime = (sw<<12) | mytime ;
  //   }
  //   //button 3 (0010)
  //   if(btn == 2){
  //     mytime = mytime & 0xf0ff;
  //     mytime = (sw<<8) | mytime ;
  //   }
  //   //button 2 (0001)
  //   if(btn == 1){
  //     mytime = mytime & 0xff0f;
  //     mytime = (sw<<4) | mytime ;
  //   }

  //   // If the counter has reached 10, call the time2string, display_string, display_update, and tick functions
  //   if (timeoutcount == 10) {
  //     time2string( textstring, mytime );
  //     display_string( 3, textstring );
  //     tick( &mytime );
  //     *LED = *LED + 0x1;
  //     timeoutcount = 0;
      
  //   }
    // display_update();
    // display_image(0, icon);
}