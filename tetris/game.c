#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "tetris.h"  
#include <stdio.h>
#include <string.h>
#include <math.h>

int main(void) {
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
	// display_string(0, "LEVEL");
	// display_string(1, "0123456789");
	// display_string(2, "SCORE");
	// display_string(3, "level-score");
	display_update();
	labinit(); /* Do any lab-specific initialization */
	// init_scoreboard();
	// init_levelboard();
	// update_score(12345);
	// update_level(89);
		
		while( 1 )
		{
			// draw_screen();
			handle_menu_input();
			delay(100);
			render_menu();
		}
}

/* Interrupt Service Routine */
void user_isr( void )
{
  if (IFS(0) & 0x100) {
    IFS(0) = 0;
  } 
}

// Initialize the game_state struct pointer
struct game_state *game;

// Function to initialize the game_state struct
struct game_state* init_gamestate() {
    game = (struct game_state*) malloc(sizeof(struct game_state));
    game->instDrop = 0;
    game->score = 0;
    game->level = 1;
    game->FpG = 43;
    return game;
}

// Function to update the game_state struct
void update_gamestate(int instDrop, int score, int level, int FpG) {
    game->instDrop = instDrop;
    game->score = score;
    game->level = level;
    game->FpG = FpG;
}

// Functions to set individual fields in the game_state struct
void gamestate_set_instDrop(int instDrop) {
    game->instDrop = instDrop;
}

void gamestate_set_score(int score) {
    game->score = score;
}

void gamestate_set_level(int level) {
    game->level = level;
}

void gamestate_set_FpG(int FpG) {
    game->FpG = FpG;
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
