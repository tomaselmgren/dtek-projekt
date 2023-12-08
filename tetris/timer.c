#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "tetris.h"  
#include <stdio.h>
#include <string.h>
#include <math.h>
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
  PR2 = (31250 / 6); // Period register counts down from 3906 at 80 MHz, which is 100 ms. divide by 6 should make it 60 fps
  TMR2 = 0;
  T2CONSET = 0x8000;

  return; 
}

void startup() {
           /*
	  This will set the peripheral bus clock to the same frequency
	  as the sysclock. That means 80 MHz, when the microcontroller
	  is running at 80 MHz. Changed 2017, as recommended by Axel.
	*/
	SYSKEY = 0xAA996655;  /* Unlock OSCCON, step 1 */
	SYSKEY = 0x556699AA;  /* Unlock OSCCON, step 2 */
	while(OSCCON & (1 << 21)); /* Wait until PBDIV ready */
	OSCCONCLR = 0x180000; /* clear PBDIV bit <0,1> */
	while(OSCCON & (1 << 21));  /* Wait until PBDIV ready */
	SYSKEY = 0x0;  /* Lock OSCCON */

	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;

	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;

	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);

	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
        SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;

	display_init();
	display_update();
	labinit();
}
