#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "tetris.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

int main(void) {
    startup();
    struct Game_State game = create_game_struct();

    while (1) {
        int btns = getbtns();
        seed(&game);
        if (btns > 0) {
            //Clears the strings from the screen before creating game
            clear_string_display();

            delay(500);

            //Creates a game state
            create_game(&game);
        }

        //Start screen to enter game
        display_string(0, "|    TETRIS    |");
        display_string(1, "|              |");
        display_string(2, "|  START GAME  |");
        display_string(3, "| (Any Button) |");
        display_update();
    }
    return 0;
}

/* Array containing the amount of frames needed for each drop per level */
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

/* Array containing the amount of lines that needs to be cleared before you go up a level*/
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

/* How many milliseconds per frame */
static const int TARGET_SECONDS_PER_FRAME = 10;

/* Returns the next drop time */
double get_time_to_next_drop(int level)
{
    if (level > 29)
    {
        level = 29;
    }
    return (FRAMES_PER_DROP[level] / 3) * TARGET_SECONDS_PER_FRAME;
}

/* Creates a game state struct */
struct Game_State create_game_struct() {
    struct Game_State game = {
        .board = {0},
        .data_board = {0},
        .nextPieceIndex = 0,
        .phase = GAME_PHASE_START,
        .start_level = 1,
        .level = 1,
        .score = 0,
        .lines = 0,
        .time = 0,
        .tick = 0,
        .leaderboard.currentHighscore = 0,
    };

    return game;
}

/* Updates the current game state struct to reset the game */
void update_game_struct(struct Game_State *game) {
        for (int row = 0; row < 72; row++) {
            for (int col = 0; col < 30; col++) {
                game->board[row][col] = 0;
                game->data_board[row][col] = 0;
            }
        }
        game->nextPieceIndex = 0;
        game->start_level = 1;
        game->level = 1;
        game->score = 0;
        game->lines = 0;
        game->time = 0;
        game->tick = 0;
        game->phase = GAME_PHASE_START;
}

/* Creates a menu struct */
struct Menu_State create_menu_struct() {
    struct Menu_State menu = {
         .screen = MENU_SCREEN,
        .currentOption = 0,
    };

    return menu;
}

/* Creates the entire game */
void create_game(struct Game_State *game) {
    game->phase = GAME_PHASE_START;

    while (game->phase != GAME_EXIT) {
        switch (game->phase) {
            case GAME_PHASE_START:
            update_game_start(game);
            break;
            case GAME_PHASE_PLAY:
            update_game_play(game);
            break;
            case GAME_PHASE_GAMEOVER:
            update_game_gameover(game);
            break;
        }
    }
}

/* Updates the start phase of the game where the menus are */
void update_game_start(struct Game_State *game) {
    struct Menu_State menu = create_menu_struct();

    while (game->phase == GAME_PHASE_START) {

       // Display the menu options
        switch (menu.screen) {
            case MENU_SCREEN:
                while (menu.screen == MENU_SCREEN && game->phase == GAME_PHASE_START) {
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
            case LEADERBOARD_SCREEN:
                while (menu.screen == LEADERBOARD_SCREEN && game->phase == GAME_PHASE_START) {
                   update_leaderboard_screen(&menu, game);
                }
                break;
        }

        delay(500);
        clear_string_display();
    }
}

/* Updates the gameplay */
void update_game_play(struct Game_State *game) {

    // Setup the scoreboard and Levelboard
    init_scoreboard();
    init_levelboard();
    update_level(game);

    //sets the nextPieceIndex to a random number between 0-6
    game->nextPieceIndex = pcg32_boundedrand_r(game, 7);

    //Spawn the first tetromino
    spawn_tetromino(game);

    while (game->phase == GAME_PHASE_PLAY) {

        //Each time the timer ticks it should update the frame
        if (IFS(0) & 0x100) {
            IFS(0) = 0;

            game->tick += TARGET_SECONDS_PER_FRAME;
            game->time += 1;

            clearScreen();

            // Button Handling
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
                switch (sw) {
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

            // Soft drops the tetromino everytime the timer has ticked the specified amount
            if (valid_soft_drop(game)) {
                moveDown(game);
            }

            /* 
            If current tetromino can't move downwards anymore place the piece where it is.
            Updates the databoard and clears all completed lines the convert the databoard back into the display board to update it.
            If a tetromino is found above the gameboard then it is gameover.
            It isn't gameover spawn a new tetromino.
            */
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

            // Draw the new display board
            draw_screen(game);
        }
    }
}

//Updates the gameover frames
void update_game_gameover(struct Game_State *game) {  
    while (game->phase == GAME_PHASE_GAMEOVER) {
        delay(1000);
        clear_string_display();

        render_highscore(game, game->score);

        delay(1000);
        clear_string_display();

        choose_initials(game, game->score);

        delay(1000);
        clear_string_display();

        update_game_struct(game);
    }
}

/* Checks if a the first column above the gameboard contains a tetromino */
bool is_gameover(struct Game_State *game) {
    for (int col = 0; col < 30; col++) {
        if (game->board[60][col] == 1) {
            return true;
        }
    }
    return false;
}

/* Render player score */
void render_highscore(struct Game_State *game, int score) {
    int btn = 0;
         
    if (score > game->leaderboard.currentHighscore) {
        while (btn == 0) {
            btn = getbtns();
            display_string(0, "New Highscore:");
            display_string(1, itoaconv(score));
            display_update();
            game->leaderboard.currentHighscore = score;
        } 
    } else {
        while (btn == 0) {
            btn = getbtns();
            display_string(0, "New Score:");
            display_string(1, itoaconv(score));
            display_update();
        }
    }
}

/* insert the score after a game */
void insert_score(struct Game_State *game, struct PlayerScore playerScore) {
    
    // Increment size of leaderboard by 1
    game->leaderboard.size++;
    
    int i;

    // Find the correct position to insert the new score
    for (i = 0; i < game->leaderboard.size; i++) {
        if (playerScore.score > game->leaderboard.leaderboard[i].score) {
            break;
        }
    }

    // Shift elements to make space for the new score
    for (int j = game->leaderboard.size - 1; j > i; j--) {
        game->leaderboard.leaderboard[j].score = game->leaderboard.leaderboard[j - 1].score;
        strcpy(game->leaderboard.leaderboard[j].initials, game->leaderboard.leaderboard[j - 1].initials);
    }

    // Insert the new score and initials
    game->leaderboard.leaderboard[i].score = playerScore.score;
    strcpy(game->leaderboard.leaderboard[i].initials, playerScore.initials);
}

/* Function to choose intials after game */
void choose_initials(struct Game_State *game, int score) {
    char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int numChars = sizeof(characters) - 1; // -1 to exclude the null terminator

    char initials[4] = "AAA"; // Array to store the three initials
    int currentIndex = 0;     // Index for the current character in the character array
    int editingInitial = 0;   // Index for which initial is being edited

    int btns = 0;

    while (btns != 1) {

        btns = getbtns();
        int sw = getsw();
        char underscore_text[16];

        switch (sw) {
            case 8:
            editingInitial = 0;
            strcpy(underscore_text, "-");
            break;
            case 4:
            editingInitial = 1;
            strcpy(underscore_text, " -");
            break;
            case 2:
            editingInitial = 2;
            strcpy(underscore_text, "  -");
            break;
            default:
            strcpy(underscore_text, " ");
            break;
        }

        if (btns == 4 && sw > 0) 
        { // move to previous character
            currentIndex = (currentIndex - 1 + numChars) % numChars;
            initials[editingInitial] = characters[currentIndex];
        }
        else if (btns == 2 && sw > 0)
        { // move to the next character
            currentIndex = (currentIndex + 1) % numChars;
            initials[editingInitial] = characters[currentIndex];
        }
        delay(50);

        char text[20];
        strcpy(text, initials);
        strcat(text, " - ");
        strcat(text, itoaconv(score));

        display_string(0, "Set Your Initials");
        display_string(1, text);   // Display the current initials and which initial is being edited.
        display_string(2, underscore_text);
         display_string(3, "Exit: Btn 2");
        display_update();
    }

        struct PlayerScore playerScore;
        playerScore.score = score;
        for (int i = 0; i < 4; i++) {
            playerScore.initials[i] = initials[i];
    }

    insert_score(game, playerScore);
}


/* Set the databoard to the current gameboard */
void board_to_databoard(struct Game_State *game) {
    for (int i = 0; i < 72; i++) {
        for (int j = 0; j < 30; j++) {
            game->data_board[i][j] = game->board[i][j];
        }
    }
}

/* Set the current databoard to the gameboard */
void databoard_to_board(struct Game_State *game) {
    for (int i = 0; i < 72; i++) {
        for (int j = 0; j < 30; j++) {
            game->board[i][j] = game->data_board[i][j];
        }
    }
}


/// MENU RENDERING

/* Updates the menu screen each frame update */
void update_menu_screen(struct Menu_State *menu) {
    clearScreen();

    char start[20];
    char leaderboard[20];
    char exit[20];

    //Create the text
    if (menu->currentOption != 0) { strcpy(start, "* Start");  } else { strcpy(start, "> Start"); }
    if (menu->currentOption != 1) { strcpy(leaderboard, "* Leaderboard"); } else { strcpy(leaderboard, "> Leaderboard"); }
    if (menu->currentOption != 2) { strcpy(exit, "* Exit"); } else { strcpy(exit, "> Exit"); }

    display_string(0, start);
    display_string(1, leaderboard);
    display_string(2, exit);

    display_update();
}

/* Updates the gameoption screen each frame update */
void update_gameoption_screen(struct Game_State *game, struct Menu_State *menu) {
    clearScreen();

    char start[20];
    char level[20];
    char exit[20];

    //Create the text
    if (menu->currentOption != 0) { strcpy(start, "* Start");  } else { strcpy(start, "> Start"); }
    if (menu->currentOption != 1) { strcpy(level, "* Level: "); } else { strcpy(level, "> Level: "); }
    if (menu->currentOption != 2) { strcpy(exit, "* Exit"); } else { strcpy(exit, "> Exit"); }

    strcat(level, itoaconv(game->start_level));

    display_string(0, start);
    display_string(1, level);
    display_string(2, exit);

    display_update();
}

/* Updates the leaderboard each frame update */
void update_leaderboard_screen(struct Menu_State *menu, struct Game_State *game) {
    
    clear_string_display();

    int btns = 0;
    int currentIndex = 0;

    // Find the index where leaderboard score becomes zero
    int maxIndex = 0;
    while (maxIndex < game->leaderboard.size) {
        maxIndex++;
    }

    while (menu->screen == LEADERBOARD_SCREEN) {

        btns = getbtns();

        switch (btns) {
            case 1:
                menu->screen = MENU_SCREEN;
                break;
            case 2:
                currentIndex++;
                if (currentIndex > maxIndex) {
                    currentIndex = maxIndex; // Limit maximum index
                    if (currentIndex < 0) {
                        currentIndex = 0; // Ensure not below 0
                    }
                }
                if (maxIndex < 3) {
                    currentIndex = 0;
                }
                break;
            case 4: 
                currentIndex--;
                if (currentIndex < 0) {
                    currentIndex = 0; // Limit minimum index to 0
                }
                break;
        }

        delay(50);
        display_string(0, "Highscores");
        for (int i = currentIndex; i < currentIndex + 3 && i < maxIndex; i++ ) {
            char scores[20];
            strcpy(scores, game->leaderboard.leaderboard[i].initials);
            strcat(scores, " - ");
            strcat(scores, itoaconv(game->leaderboard.leaderboard[i].score));
            display_string((i - currentIndex + 1), scores);
        }

        display_update();
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

/* Tries to find a completed row and removes it, returns true if it has removed and false if it hasn't */
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

/**
 * Func that should be called in main game loop every collision.
 * Clears Rows until there are no more rows to clear.
 */
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

/* Increases the level of the current game */
void update_level(struct Game_State *game)
{
    if (game->lines >= LINES_PER_LEVEL[game->level-1]) {
        game->lines -= LINES_PER_LEVEL[game->level-1];
        game->level++;
        update_level_text(game->level);
    }

}