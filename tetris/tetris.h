#ifndef TETRIS_H_URHXV5O2
#define TETRIS_H_URHXV5O2

/* mipslab.h
   Header file for all labs.
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "tetris.h"  
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

struct Tetromino
{
    int data[4][4];
    int side;
};

enum Game_Phase
{
    GAME_PHASE_START,
    GAME_PHASE_PLAY,
    GAME_PHASE_GAMEOVER,
    GAME_EXIT

};

struct Piece_State
{
    int data[4][4];
    int tetromino[12][12];
    int side;
    int pos_x;
    int pos_y;
    int rotation; // 0, 90, 180, 270
};

enum Screens
{
    MENU_SCREEN,
    GAMEOPTION_SCREEN,
    LEADERBOARD_SCREEN,
    OFF
};

struct Menu_State {
    enum Screens screen;
    int currentOption;
};

struct pcg32_random_t {
    uint64_t state;
    uint64_t increment;
    bool has_seed;
};

// When a player finishes a game, create a PlayerScore struct with the player's
// score and initials, and add it to the leaderboard array in the Leaderboard struct.
struct PlayerScore {
    int score;
    char initials[4]; // 3 initials max + 1 for the null terminator
};

// Leaderboard to hold all PlayerScores
struct Leaderboard {
    int currentHighscore;
    struct PlayerScore leaderboard[100]; // Pointer to PlayerScore elements
    int size; // Current size of the leaderboard
};

//Struct to hold the entire Game State
struct Game_State
{
    uint8_t board[72][30];
    uint8_t data_board[72][30];
    
    int nextPieceIndex;
    struct Piece_State piece;

    enum Game_Phase phase;

    int start_level;
    int level;
    int score;
    int lines;
    int time;
    int tick;

    struct pcg32_random_t rng;
    struct Leaderboard leaderboard;
};

//Startup related functions
void labinit(void);
void display_init(void);
void startup(void);

/* Game creation functions*/
struct Game_State create_game_struct(void);
struct Menu_State create_menu_struct(void);
void create_game(struct Game_State *game);

/* Declare display-related functions*/
void display_image(int x, const uint8_t *data);
void display_string(int line, char *s);
void clear_string_display(void);
void display_update(void);

void render_screen(void);
void clearScreen(void);
void render_gameboard(struct Game_State *game);
void draw_screen(struct Game_State *game);
void render_highscore(struct Game_State *game, int score);


/* Menu functions */
void update_menu_screen(struct Menu_State *menu);
void update_gameoption_screen(struct Game_State *game, struct Menu_State *menu);
void update_leaderboard_screen(struct Menu_State *menu, struct Game_State *game);

/* Menu handling functions */
void handle_menu(struct Game_State *game, struct Menu_State *menu);
void increaseLevel(struct Game_State *game);
void gameoptions(struct Game_State *game, struct Menu_State *menu);
void execute_option(struct Game_State *game, struct Menu_State *menu);


/* Tetris score and levelboard related functions */
void init_scoreboard(void);
void init_levelboard(void);
void update_score_text(int score);
void update_level_text(int level);
void update_score(struct Game_State *game, int LinesCleared);
void update_level(struct Game_State *game);
void insert_score(struct Game_State *game, struct PlayerScore playerScore);

/* Move related functions */
bool canMoveLeft(struct Game_State *game, struct Piece_State piece);
void moveLeft(struct Game_State *game);

bool canMoveRight(struct Game_State *game, struct Piece_State piece);
void moveRight(struct Game_State *game);

bool canMoveDown(struct Game_State *game, struct Piece_State piece);
void moveDown(struct Game_State *game);
void hardDrop(struct Game_State *game);
bool valid_soft_drop(struct Game_State *game);

int can_rotate(struct Game_State *game, struct Piece_State piece);
void rotate_tetromino(struct Game_State *game);

/* Tetromino handling functions */
void spawn_tetromino(struct Game_State *game);
void remove_tetromino(struct Game_State *game);
void set_piece_data(struct Game_State *game, struct Tetromino *tetromino);
void set_piece_tetromino(struct Game_State *game);
void merge_tetromino(struct Game_State *game);
void set_next_piece(struct Game_State *game, int index);

/* Gameboard handling functions */
void board_to_databoard(struct Game_State *game);
void databoard_to_board(struct Game_State *game);

/* Game phase handling functions */
void update_game_start(struct Game_State *game);
void update_game_gameover(struct Game_State *game);
bool is_gameover(struct Game_State *game);
void update_game_play(struct Game_State *game);
void choose_initials(struct Game_State *game, int score);

/* soft drop timer function */
double get_time_to_next_drop(int level);

/* Row completion functions */
bool isRowComplete(struct Game_State *game, int rowIndex);
bool check_and_clear_row(struct Game_State *game);
void removeRow(struct Game_State *game, int rowIndex);
void clearAllCompletedRows(struct Game_State *game);



//random pcg generator
void seed(struct Game_State *game);
void pcg32_srandom_r(struct Game_State *game, uint64_t initstate, uint64_t initseq);
uint32_t pcg32_random_r(struct Game_State *game);
uint32_t pcg32_boundedrand_r(struct Game_State *game, uint32_t bound);

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
/* Declare text buffer for display output */
extern char textbuffer[4][16];

extern uint8_t image[128*4];

extern uint8_t scoreboard[16][28];
extern uint8_t levelboard[16][28];

extern uint8_t next_piece[12][12];

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

#endif