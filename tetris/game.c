#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "tetris.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

struct pcg32_random_t rng;
struct Leaderboard leaderboard;

int main(void) {
    startup();
    create_game();
    return 0;
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

static const uint8_t FRAMES_PER_DROP[] = {
    48,
    43,
    38,
    33,
    28,
    23,
    18,
    13,
    8,
    6,
    5,
    5,
    5,
    4,
    4,
    4,
    3,
    3,
    3,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    1
};

static const uint8_t LINES_PER_LEVEL[] = {
    20,
    30,
    40,
    50,
    60,
    70,
    80,
    90,
    100,
    100,
    100,
    100,
    100,
    100,
    100,
    110,
    120,
    130,
    140,
    150,
    160,
    170,
    180,
    190,
    200,
    200,
    200,
    200,
};

static const int TARGET_SECONDS_PER_FRAME = 10;

struct Tetromino TETROMINOS[] = {
    { .data = { {0, 0, 0, 0},
                {1, 1, 1, 1},
                {0, 0, 0, 0},
                {0, 0, 0, 0} }, .side = 4 },

    { .data = { {1, 0, 0},
                {1, 1, 1},
                {0, 0, 0} }, .side = 3 },

    { .data = { {0, 0, 1},
                {1, 1, 1},
                {0, 0, 0} }, .side = 3 },

    { .data = { {1, 1},
                {1, 1} }, .side = 2 },

    { .data = { {0, 1, 1},
                {1, 1, 0},
                {0, 0, 0} }, .side = 3 },

    { .data = { {1, 1, 0},
                {0, 1, 1},
                {0, 0, 0} }, .side = 3 },

    { .data = { {0, 1, 0},
                {1, 1, 1},
                {0, 0, 0} }, .side = 3 }
};

int SPAWN_POSITIONS[] = {9,12,15};

struct Game_State create_game_struct() {

    struct Game_State game = {
        .board = {0},
        .data_board = {0},
        .nextPieceIndex = 0,
        .phase = GAME_PHASE_START,
        .start_level = 1,
        .instDrop = 0,
        .level = 1,
        .score = 0,
        .lines = 0,
        .time = 0,
        .tick = 0,
    };

    return game;
}

void update_game_struct(struct Game_State *game) {
        for (int row = 0; row < 72; row++) {
            for (int col = 0; col < 30; col++) {
                game->board[row][col] = 0;
                game->data_board[row / 3][col / 3] = 0;
            }
        }

        game->nextPieceIndex = pcg32_boundedrand_r(&rng, 7);
        game->phase = GAME_PHASE_START;
        game->start_level = 1;
        game->instDrop = 0;
        game->level = 1;
        game->score = 0;
        game->lines = 0;
        game->time = 0;
        game->tick = 0;
}

struct Menu_State create_menu_struct() {
    struct Menu_State menu = {
         .screen = MENU_SCREEN,
        .currentOption = 0,
    };

    return menu;
}

void create_game() {
    struct Game_State game = create_game_struct();

    while (1) {
        switch (game.phase) {
            case GAME_PHASE_START:
            update_game_start(&game);
            break;
            case GAME_PHASE_PLAY:
            update_game_play(&game);
            break;
            case GAME_PHASE_GAMEOVER:
            update_game_gameover(&game);
            break;
        }
    }
}

void update_game_start(struct Game_State *game) {
    struct Menu_State menu = create_menu_struct();

    while (game->phase == GAME_PHASE_START) {

       // Display the menu options
        switch (menu.screen) {
            case MENU_SCREEN:
                while (menu.screen == MENU_SCREEN && game->phase == GAME_PHASE_START) {
                    seed(&rng);
                    handle_menu(game, &menu);
                    delay(100);
                    update_menu_screen(&menu);
                }
                break;
            case GAMEOPTION_SCREEN:
                while (menu.screen == GAMEOPTION_SCREEN && game->phase == GAME_PHASE_START) {
                    handle_menu(game, &menu);
                    delay(100);
                    update_gameoption_screen(game, &menu);
                }
                break;
            case SETTINGS_SCREEN:
                break;
            case ABOUT_SCREEN:
                break;
        }
    }
}

bool valid_soft_drop(struct Game_State *game) {
    if (game->tick >= get_time_to_next_drop(game->level)) {
        game->tick = 0;
        return true;
        }
    return false;
}

bool is_gameover(struct Game_State *game) {
    for (int col = 0; col < 30; col++) {
        if (game->board[60][col] == 1) {
            return true;
        }
    }
    return false;
}

void update_game_play(struct Game_State *game) {
    init_scoreboard();
    init_levelboard();

    game->nextPieceIndex = pcg32_boundedrand_r(&rng, 7);

    spawn_tetromino(game);

    while (game->phase == GAME_PHASE_PLAY) {
        if (IFS(0) & 0x100) {
            IFS(0) = 0;

            game->tick += TARGET_SECONDS_PER_FRAME;
            game->time += 1;

            clearScreen();

            int btns = getbtns();
            int sw = getsw();

            if (btns == 4) {
                moveLeft(game);
                delay(50);
                game->time = 0;
            }

            if (btns == 2) {
                moveRight(game);
                delay(50);
                game->time = 0;
            }

            if (btns == 1) {
                switch (sw + 0) {
                        case 0: 
                        moveDown(game);
                        break;
                        case 4: 
                        if (game->piece.pos_y < 60) { hardDrop(game); }
                        break;
                        case 8: rotate_tetromino(game);
                        delay(50);
                        break;
                    }
                
            }


            if (valid_soft_drop(game)) {
                moveDown(game);
            }

            if (!canMoveDown(game, game->piece) && game->time > 6) {
                board_to_databoard(game);
                clearAllCompletedRows(game);
                databoard_to_board(game);

                if (is_gameover(game)) {
                    game->phase = GAME_PHASE_GAMEOVER;
                }

                //Spawn new tetromino
                spawn_tetromino(game);


            }

            draw_screen(game);
        }
    }
}

void update_game_gameover(struct Game_State *game) {  
    while (game->phase == GAME_PHASE_GAMEOVER) {
        int btn = getbtns();
        if (btn > 0) {
            break;
        }

        render_highscore(&leaderboard, game->score);
    }

    insert_score(&leaderboard, game->score);

    while (game->phase == GAME_PHASE_GAMEOVER) {
        int btn = getbtns();
        if (btn > 0) {
            update_game_struct(game);
        }

        render_leaderboard(&leaderboard);
    }
}

double get_time_to_next_drop(int level)
{
    if (level > 29)
    {
        level = 29;
    }
    return (FRAMES_PER_DROP[level] / 3) * TARGET_SECONDS_PER_FRAME;
}

void set_piece_data(struct Game_State *game, struct Tetromino *tetromino)
{
    for (int row = 0; row < tetromino->side; row++) {
        for (int col = 0; col < tetromino->side; col++) {
            game->piece.data[row][col] = tetromino->data[row][col];
        }
    }

    game->piece.side = tetromino->side;
    set_piece_tetromino(game);
}

void set_piece_tetromino(struct Game_State *game) {
    for (int i = 0; i < game->piece.side; i++) {
      for (int j = 0; j < game->piece.side; j++) {

        int startY = i * 3;
        int startX = j * 3;

        for (int y = startY; y < startY + 3; y++) {
            for (int x = startX; x < startX + 3; x++) {
                game->piece.tetromino[y][x] = game->piece.data[i][j];
            }
        }
      }
    }
}

void spawn_tetromino(struct Game_State *game) {

    set_piece_data(game, &TETROMINOS[game->nextPieceIndex]);
    set_next_piece(game, pcg32_boundedrand_r(&rng, 7));

    game->piece.pos_x = SPAWN_POSITIONS[pcg32_boundedrand_r(&rng, 3)];
    game->piece.pos_y = 71;
    game->tick = 0;
    game->time = 0;

    merge_tetromino(game);
}

void set_next_piece(struct Game_State *game, int index) {
    game->nextPieceIndex = index;
    struct Tetromino tetromino = TETROMINOS[game->nextPieceIndex];

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++) {
            next_piece[i][j] = 0;
        }
    }

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 4; j++) {

        int startY = i * 3;
        int startX = j * 3;

        for (int y = startY; y < startY + 3; y++) {
            for (int x = startX; x < startX + 3; x++) {
                next_piece[y][x] = tetromino.data[i][j];
            }
        }
      }
    }
}

void merge_tetromino(struct Game_State *game) {
    for (int i = 0; i < game->piece.side * 3; i++) {
      for (int j = 0; j < game->piece.side * 3; j++) {
        if (game->piece.tetromino[i][j] == 1) {
          game->board[game->piece.pos_y - i][game->piece.pos_x + j] = 1;
        }
      }
    }
}

void remove_tetromino(struct Game_State *game) {
    for (int i = 0; i < game->piece.side * 3; i++) {
      for (int j = 0; j < game->piece.side * 3; j++) {
        if (game->piece.tetromino[i][j] == 1) {
          game->board[game->piece.pos_y - i][game->piece.pos_x + j] = 0;
        }
      }
    }
}

int can_rotate(struct Game_State *game, struct Piece_State piece) {
    for (int row = (piece.side * 3 - 1); row >= 0; row--) {
        for (int col = 0; col < piece.side * 3; col++) {

            //check if a piece already in the new rotated location
            if (piece.tetromino[row][col] == 1) {
                if (game->data_board[piece.pos_y - row][piece.pos_x + col] == 1) {
                    return 1;
                }
            }

            //Move Left Check
            if (piece.tetromino[row][col] == 1) {
                if (piece.pos_x + col < 0) {
                    return 2;
                }
            }

            //Move Right Check
            if (piece.tetromino[row][(piece.side - 1) - col] == 1) {
                if (piece.pos_x + col > 29) {
                    return 3;
                }
            }

            //Move Down Check
            if (piece.tetromino[row][col] == 1) {
                if (piece.pos_y - row < 0) {
                    return 4;
                }
            }
        }
    }
    return 0;
}

void rotate_tetromino(struct Game_State *game)
{
    remove_tetromino(game);

    struct Piece_State piece = game->piece;
    int N = piece.side;

    // Traverse each cycle
    for (int i = 0; i < N / 2; i++) {
        for (int j = i; j < N - i - 1; j++) {

            // Swap elements of each cycle
            // in clockwise direction
            int temp = piece.data[i][j];
            piece.data[i][j] = piece.data[N - 1 - j][i];
            piece.data[N - 1 - j][i] = piece.data[N - 1 - i][N - 1 - j];
            piece.data[N - 1 - i][N - 1 - j] = piece.data[j][N - 1 - i];
            piece.data[j][N - 1 - i] = temp;
        }
    }

    for (int i = 0; i < piece.side; i++) {
      for (int j = 0; j < piece.side; j++) {

        int startY = i * 3;
        int startX = j * 3;

        for (int y = startY; y < startY + 3; y++) {
            for (int x = startX; x < startX + 3; x++) {
                piece.tetromino[y][x] = piece.data[i][j];
            }
        }
      }
    }

    switch (can_rotate(game, piece)) {
        case 0:
            game->piece = piece;
            break;
        case 1:
            if (piece.side == 4) { piece.pos_y += 3 * 3; } else { piece.pos_y += 3; }
            if (can_rotate(game, piece) == 0) {
                game->piece = piece;
            }
            update_level_text(1);
            break;
        case 2:
            if (piece.side == 4) { piece.pos_x += 3 * 3; } else { piece.pos_x += 3; }
            if (can_rotate(game, piece) == 0) {
                game->piece = piece;
            }
            update_level_text(2);
            break;
        case 3:
            if (piece.side == 4) { piece.pos_x -= 3 * 3; } else { piece.pos_x -= 3; }
            if (can_rotate(game, piece) == 0) {
                game->piece = piece;
            }
            update_level_text(3);
            break;
        case 4:
            if (piece.side == 4) { piece.pos_y += 3 * 3; } else { piece.pos_y += 3; }
            if (can_rotate(game, piece) == 0) {
                game->piece = piece;
            }
            update_level_text(4);
            break;
    }

    merge_tetromino(game);
}

void moveLeft(struct Game_State *game)
{
    remove_tetromino(game);
    struct Piece_State piece = game->piece;
    if (canMoveLeft(game, piece)) {
        piece.pos_x -= 3;
    }

    game->piece = piece;
    merge_tetromino(game);
}

void moveRight(struct Game_State *game)
{
    remove_tetromino(game);
    struct Piece_State piece = game->piece;
    if (canMoveRight(game, piece)) {
        piece.pos_x += 3;
    }

    game->piece = piece;
    merge_tetromino(game);
}

bool canMoveLeft(struct Game_State *game, struct Piece_State piece)
{
    bool value = true;

    for (int row = (piece.side * 3 - 1); row >= 0 && value == true; --row) {
        for (int col = 0; col < piece.side * 3 && value == true; col++) {
            if (piece.tetromino[row][col] == 1) {
                if ((piece.pos_x - 3 + col < 0) || (game->data_board[piece.pos_y - row][piece.pos_x - 1 + col] == 1)) {
                    return false;
                }
            }
        }
    }

    return value;
}

bool canMoveRight(struct Game_State *game, struct Piece_State piece)
{
    bool value = true;

    for (int row = (piece.side * 3 - 1); row >= 0 && value == true; --row) {
        for (int col = (piece.side * 3 - 1); col >= 0 && value == true; --col) {
            if (piece.tetromino[row][col] == 1) {
                if ((piece.pos_x + 3 + col > 29) || (game->data_board[piece.pos_y - row][piece.pos_x + 1 + col] == 1)) {
                    value = false;
                }
            }
        }
    }
    return value;
}


bool canMoveDown(struct Game_State *game, struct Piece_State piece)
{
    bool value = true;

    for (int row = (piece.side * 3 - 1); row >= 0 && value == true; row--) {
        for (int col = 0; col < piece.side * 3 && value == true; col++) {
            if (piece.tetromino[row][col] == 1) {
                if ((piece.pos_y - 1 - row < 0) || (game->data_board[piece.pos_y - 1 - row][piece.pos_x + col] == 1))  {
                    value = false;
                }
            }
        }
    }
    return value;
}

void moveDown(struct Game_State *game)
{
    struct Piece_State piece = game->piece;
    remove_tetromino(game);

    if (canMoveDown(game, piece)) {
        piece.pos_y -= 1;
    }

    game->piece = piece;
    merge_tetromino(game);

}

void hardDrop(struct Game_State *game)
{
    remove_tetromino(game);
    struct Piece_State piece = game->piece;
    while (canMoveDown(game, piece)) {
        piece.pos_y -= 1;
    }

    game->piece = piece;
    merge_tetromino(game);
}

void board_to_databoard(struct Game_State *game) {
    for (int i = 0; i < 72; i++) {
        for (int j = 0; j < 30; j++) {
            game->data_board[i][j] = game->board[i][j];
        }
    }
}

void databoard_to_board(struct Game_State *game) {
    for (int i = 0; i < 72; i++) {
        for (int j = 0; j < 30; j++) {
            game->board[i][j] = game->data_board[i][j];
        }
    }
}


// /// MENU RENDERING

void update_menu_screen(struct Menu_State *menu) {
    clearScreen();

    char start[20];
    char settings[20];
    char about[20];
    char exit[20];

    //Create the text
    if (menu->currentOption != 0) { strcpy(start, "* Start");  } else { strcpy(start, "> Start"); }
    if (menu->currentOption != 1) { strcpy(settings, "* Settings"); } else { strcpy(settings, "> Settings"); }
    if (menu->currentOption != 2) { strcpy(about, "* About"); } else { strcpy(about, "> About"); }
    if (menu->currentOption != 3) { strcpy(exit, "* Exit"); } else { strcpy(exit, "> Exit"); }

    display_string(0, start);
    display_string(1, settings);
    display_string(2, about);
    display_string(3, exit);

    //display_string(0, "WTF!");
    display_update();
}

void update_gameoption_screen(struct Game_State *game, struct Menu_State *menu) {
    clearScreen();

    char start[20];
    char level[20];
    char drop[20];
    char exit[20];


    //Create the text
    if (menu->currentOption != 0) { strcpy(start, "* Start");  } else { strcpy(start, "> Start"); }
    if (menu->currentOption != 1) { strcpy(level, "* Level: "); } else { strcpy(level, "> Level: "); }
    if (menu->currentOption != 2) { strcpy(drop, "* Insta Drop: "); } else { strcpy(drop, "> Insta Drop: "); }
    if (menu->currentOption != 3) { strcpy(exit, "* Exit"); } else { strcpy(exit, "> Exit"); }

    strcat(level, itoaconv(game->start_level));
    strcat(drop, itoaconv(game->instDrop));

    display_string(0, start);
    display_string(1, level);
    display_string(2, drop);
    display_string(3, exit);

    display_update();
}

// ///HANDLE MENU OPTIONS

// Implement this function to get the user's input and update the currentOption accordingly
void handle_menu(struct Game_State *game, struct Menu_State *menu) {
  int btn = getbtns();
  if (btn == 4) { // Up
    menu->currentOption = (menu->currentOption - 1 + 4) % 4;
  } else if (btn == 2) { // Down
    menu->currentOption = (menu->currentOption + 1) % 4;
  } else if (btn == 1) { // Select
    switch (menu->screen) {
        case MENU_SCREEN:
        execute_option(game, menu);
        break;
        case GAMEOPTION_SCREEN:
        gameoptions(game, menu);
        break;
    }

  }
}

volatile int currentLevelIndex = 0;

// ///HANDLE MENU EXECUTIONS

void increaseLevel(struct Game_State *game) {
    int levels[5] = {1, 5, 10, 20, 25};
    currentLevelIndex = (currentLevelIndex + 1) % 5;
    game->start_level = levels[currentLevelIndex];
    game->level = levels[currentLevelIndex];
}

// Implement this function to execute the selected menu option
void gameoptions(struct Game_State *game, struct Menu_State *menu) {
    switch (menu->currentOption) {
        case 0:
            game->phase = GAME_PHASE_PLAY;
            break;
        case 1:
             increaseLevel(game);
            break;
        case 2:
            game->instDrop = !game->instDrop;
            break;
        case 3:
            menu->screen = MENU_SCREEN;
            break;
    }
}

// Implement this function to execute the selected menu option
void execute_option(struct Game_State *game, struct Menu_State *menu) {
    switch (menu->currentOption) {
        case 0:
             menu->screen = GAMEOPTION_SCREEN;
            break;
        case 1:
            menu->screen = SETTINGS_SCREEN;
            break;
        case 2:
            menu->screen = ABOUT_SCREEN;
            break;
        case 3:
            menu->screen = OFF;
            break;
    }
}

/**
 * Checks if a row with rowIndex is completed.
 * Returns true if it is completed, false if it has any empty cells.
*/
bool isRowComplete(struct Game_State *game, int rowIndex)
{
    for (int col = 0; col < 30; col++)
    {
        if (game->data_board[rowIndex][col] == 0)
        {
            return false; // Found an empty cell, so the row is not complete
        }
    }
    return true; // No empty cells found, the row is complete
}


/**
 * Removes the given row and shifts down the rows above it.
*/
void removeRow(struct Game_State *game, int rowIndex)
{
        for (int i = rowIndex; i < 71; i++) {
            for (int j = 0; j < 30; j++) {
                game->data_board[i][j] = game->data_board[i+1][j];
            }
        }

        for (int j = 0; j < 10; j++) {
            game->data_board[71][j] = 0;
        }
}

/**
 * Func that should be called in main game loop every collision.
 */
bool check_and_clear_row(struct Game_State *game)
{
    for (int y = 0; y < 60; y++)
    {
        if (isRowComplete(game, y))
        {
            removeRow(game, y);
            return true;
        }
    }
    return false;
}

void clearAllCompletedRows(struct Game_State *game) {
    int linesCleared = 0;
    while (check_and_clear_row(game)) {
        linesCleared++;
    }

    if (linesCleared > 0)
    {
        game->lines += (linesCleared / 3);
        update_score(game, (linesCleared / 3));
        update_level(game);
    }
}

/**
 * Simple function to update the score, dependent on level and how many lines are cleared.
 * It's a basic version of this: https://tetris.wiki/Scoring.
 */
void update_score(struct Game_State *game, int linesCleared)
{
    switch (linesCleared)
    {
    case 1:
        game->score += 10 * game->level;
        break;
    case 2:
        game->score += 30 * game->level;
        break;
    case 3:
        game->score += 50 * game->level;
        break;
    case 4:
        game->score += 80 * game->level;
        break;
    default:
        break;
    }

    update_score_text(game->score);
}

void update_level(struct Game_State *game)
{
    if (game->lines >= LINES_PER_LEVEL[game->level-1]) {
        game->lines -= LINES_PER_LEVEL[game->level-1];
        update_level_text(game->level);
    }

}