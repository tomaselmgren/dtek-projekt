#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "tetris.h"  
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Declare a helper function which is local to this file */
static void num32asc( char * s, int ); 

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)


/* display_debug
   A function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/   
void display_debug( volatile int * const addr )
{
  display_string( 1, "Addr" );
  display_string( 2, "Data" );
  num32asc( &textbuffer[1][6], (int) addr );
  num32asc( &textbuffer[2][6], *addr );
  display_update();
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_init(void) {
        DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);
	
	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);
	
	spi_send_recv(0x8D);
	spi_send_recv(0x14);
	
	spi_send_recv(0xD9);
	spi_send_recv(0xF1);
	
	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);
	
	spi_send_recv(0xA1);
	spi_send_recv(0xC8);
	
	spi_send_recv(0xDA);
	spi_send_recv(0x20);
	
	spi_send_recv(0xAF);
}

/* Helper function, local to this file.
   Converts a number to hexadecimal ASCII digits. */
static void num32asc( char * s, int n ) 
{
  int i;
  for( i = 28; i >= 0; i -= 4 )
    *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}

/// Display functions

void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;
	
	for(i = 0; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
}

//Define the different selections
#define START 0
#define SETTINGS 1
#define ABOUT 2
#define EXIT 3

void display_update(void) {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;
			
			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}

void display_image(int x, const uint8_t *data) {
	int i, j;
	
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 128; j++)
			spi_send_recv(data[i*128 + j]);
	}
}

void turnOffPixel(int x, int y) {
    short offset = 0;
    if (x > 0) { offset = x / 8; }
    image[offset * 128 + y] |= 0 << (x - offset * 8);
}

void turnOnPixel(int x, int y) {
    short offset = 0;
    if (x > 0) { offset = x / 8; }
    image[offset * 128 + y] |= 1 << (x - offset * 8);
}

int isPixelTurnedOn(int x, int y) {
    short offset = 0;
    if (x > 0) { offset = x / 8; }
    return (image[offset * 128 + y] >> (x - offset * 8)) & 1;
}

void clearScreen() {
    int i;
    for (i = 0; i < 512; i++) { image[i] = 0; }
}

void clear_string_display() {
    display_string(0, " ");
    display_string(1, " ");
    display_string(2, " ");
    display_string(3, " ");
    display_update();
}


//renders the entire gameboard
#define boardHeight 60
#define boardWidth 30

#define nextPieceHeight 12
#define nextPieceWidth 12

#define scoreHeight 16
#define scoreWidth 28

#define levelHeight 16
#define levelWidth 28

#define screenWidth  32
#define screenHeight 128

#define screenXStart 1
#define screenXEnd 30
#define screenYStart 22
#define screenYEnd 81

#define scoreXStart 2
#define scoreYStart 109

#define levelXStart 2
#define levelYStart 3

void render_gameboard(struct Game_State *game) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 128; j++) {
            int boardX = (i - screenXStart);
            int boardY = (j - screenYStart);

            int nextPieceX = (i - 10);
            int nextPieceY = (j - 85);

            int scoreX = (i - scoreXStart);
            int scoreY = (j - scoreYStart);

            int levelX = (i - levelXStart);
            int levelY = (j - levelYStart);

            if ((i < screenXStart || i > screenXEnd || j < 1 || (j < screenYStart && j > (screenYStart - 2)) || (j > screenYEnd && j < (screenYEnd + 2)) || j > 126)) {
              turnOnPixel(i, j); // Render border pixels
            }

            // Rendering game board blocks
            if (boardX >= 0 && boardX < boardWidth && boardY >= 0 && boardY < boardHeight) {
                if ((game->board[boardY][boardX] == 1)) {
                  turnOnPixel(i, j);
                }
            }

            // Rendering nextPiece
            if (nextPieceX >= 0 && nextPieceX < nextPieceWidth && nextPieceY >= 0 && nextPieceY < nextPieceHeight) {
                if ((next_piece[(nextPieceHeight - 1) - nextPieceY][nextPieceX] == 1)) {
                  turnOnPixel(i, j);
                }
            }

            if (j == (85 + nextPieceHeight + 2)) {
               turnOnPixel(i, j);
            }

            // Rendering scoreboard
            if (scoreX >= 0 && scoreX < scoreWidth && scoreY >= 0 && scoreY < scoreHeight) {
                if ((scoreboard[(scoreHeight - 1) - scoreY][scoreX] == 1)) {
                  turnOnPixel(i, j);
                }
            }

            // Rendering scoreboard
            if (levelX >= 0 && levelX < levelWidth && levelY >= 0 && levelY < levelHeight) {
                if ((levelboard[(levelHeight - 1) - levelY][levelX] == 1)) {
                  turnOnPixel(i, j);
                }
            }
        }
    }
}

//Draws the screen for the game loop
void draw_screen(struct Game_State *game) {
  render_gameboard(game);
  render_screen();
}


void render_screen() {
	int i, j;

	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0 & 0xF);
		spi_send_recv(0x10 | ((0 >> 4) & 0xF));
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 128; j++)
			spi_send_recv(image[i*128 + j]);
	}
}

void init_scoreboard() {
  for (int y = 0; y < 7; y++ ) {
    for (int x = 0; x < 28; x++ ) {
      scoreboard[y][x] = scoretext[x + y * 28];
    }
  }
  update_score_text(0);
}

void init_levelboard() {
  for (int y = 0; y < 7; y++ ) {
    for (int x = 0; x < 28; x++ ) {
      levelboard[y][x] = leveltext[x + y * 28];
    }
  }

  update_level_text(0);
}

void update_level_text(int level) {
    int n = level;
    int arr[2] = {0, 0};
    int i = 0;

		while(n > 0) { 
			arr[1-i] = n % 10; 
			n /= 10; 
			i++; 
		} 

    // Loop through each row in the digit arrays
        for (int i = 0; i < 2; i++) {
            int digit = arr[i];

            uint8_t *current_digit_array;

            switch (digit) {
                case 0: current_digit_array = zero; break;
                case 1: current_digit_array = one; break;
                case 2: current_digit_array = two; break;
                case 3: current_digit_array = three; break;
                case 4: current_digit_array = four; break;
                case 5: current_digit_array = five; break;
                case 6: current_digit_array = six; break;
                case 7: current_digit_array = seven; break;
                case 8: current_digit_array = eight; break;
                case 9: current_digit_array = nine; break;
                default: break;
            }

            for (int y = 0; y < 7; y++ ) {
              for (int x = 0; x < 4; x++ ) {
                levelboard[y + 9][(x + 9) + i * 6] = current_digit_array[x + y * 4];
              }
            }
      }
}

void update_score_text(int score) {
    int n = score;
    int arr[5] = {0, 0, 0, 0, 0};
    int i = 0;

		while(n > 0) { 
			arr[4-i] = n % 10; 
			n /= 10; 
			i++; 
		} 

    // Loop through each row in the digit arrays
        for (int i = 0; i < 5; i++) {
            int digit = arr[i];

            uint8_t *current_digit_array;

            switch (digit) {
                case 0: current_digit_array = zero; break;
                case 1: current_digit_array = one; break;
                case 2: current_digit_array = two; break;
                case 3: current_digit_array = three; break;
                case 4: current_digit_array = four; break;
                case 5: current_digit_array = five; break;
                case 6: current_digit_array = six; break;
                case 7: current_digit_array = seven; break;
                case 8: current_digit_array = eight; break;
                case 9: current_digit_array = nine; break;
                default: break;
            }

            for (int y = 0; y < 7; y++ ) {
              for (int x = 0; x < 4; x++ ) {
                scoreboard[y + 9][x + i * 6] = current_digit_array[x + y * 4];
              }
            }
      }
}