/* mipslab.h
   Header file for all labs.
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

/* Declare display-related functions from mipslabfunc.c */
void display_image(int x, const uint8_t *data);
void render_screen(void);
void clearScreen(void);
void render_gameboard(void);
void draw_screen(void);
void display_init(void);
void display_string(int line, char *s);
void display_update(void);
void update_score(int score);
void init_scoreboard(void);
void init_levelboard(void);
void handle_menu(void);
void render_menu(void);

uint8_t spi_send_recv(uint8_t data);

/* Declare lab-related functions from mipslabfunc.c */
char * itoaconv( int num );
void labwork(void);
int nextprime( int inval );
void quicksleep(int cyc);
void tick( unsigned int * timep );

/* Declare display_debug - a function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/
void display_debug( volatile int * const addr );

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];
/* Declare bitmap array containing icon */
extern const uint8_t const icon[128];
/* Declare bitmap array containing board */
extern const uint8_t const board[32*128];
/* Declare text buffer for display output */
extern char textbuffer[4][16];

extern uint8_t image[128*4];
extern uint8_t gameboard[24*3][10*3];

extern uint8_t scoreboard[16][28];
extern uint8_t levelboard[16][28];

/* Declare bitmap array containing the numbers used in the score- and levelboard*/
extern uint8_t zero[4*7];
extern uint8_t one[4*7];
extern uint8_t two[4*7];
extern uint8_t three[4*7];
extern uint8_t four[4*7];
extern uint8_t five[4*7];
extern uint8_t six[4*7];
extern uint8_t seven[4*7];
extern uint8_t eight[4*7];
extern uint8_t nine[4*7];

extern uint8_t scoretext[28*7];
extern uint8_t leveltext[28*7];

#ifndef GAMESTATE_H
#define GAMESTATE_H

struct game_state {
    int instDrop;
    int score;
    int level;
    int FpG;
};

// Declare a pointer to the game_state struct
extern struct game_state *game;

// Function prototypes for functions defined in a different source file
struct game_state* init_gamestate();
void update_gamestate(int instDrop, int score, int level, int FpG);
void gamestate_set_instDrop(int instDrop);
void gamestate_set_score(int score);
void gamestate_set_level(int level);
void gamestate_set_FpG(int FpG);

#endif // GAMESTATE_H


/* Declare functions written by students.
   Note: Since we declare these functions here,
   students must define their functions with the exact types
   specified in the laboratory instructions. */
/* Written as part of asm lab: delay, time2string */
void delay(int);
void time2string( char *, int );
/* Written as part of i/o lab: getbtns, getsw, enable_interrupt */
int getbtns(void);
int getsw(void);
void enable_interrupt(void);
