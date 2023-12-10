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



/* HANDLE MENU OPTIONS */

/* Function to get the user's input and update the currentOption accordingly */
void handle_menu(struct Game_State *game, struct Menu_State *menu) {
  int btn = getbtns();
  if (btn == 4) { // Up
    menu->currentOption = (menu->currentOption - 1 + 3) % 3;
  } else if (btn == 2) { // Down
    menu->currentOption = (menu->currentOption + 1) % 3;
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

/* HANDLE MENU EXECUTIONS */

volatile int currentLevelIndex = 0;

/* Function to increase the start level for each game */
void increaseLevel(struct Game_State *game) {
    int levels[5] = {1, 5, 10, 20, 25};
    currentLevelIndex = (currentLevelIndex + 1) % 5;
    game->start_level = levels[currentLevelIndex];
    game->level = levels[currentLevelIndex];
}

/* handling function to execute the selected gameoptions */
void gameoptions(struct Game_State *game, struct Menu_State *menu) {
    switch (menu->currentOption) {
        case 0:
            game->phase = GAME_PHASE_PLAY;
            break;
        case 1:
            increaseLevel(game);
            break;
        case 2:
            menu->screen = MENU_SCREEN;
            break;
    }
}

/* handling function to execute the selected menu options */
void execute_option(struct Game_State *game, struct Menu_State *menu) {
    switch (menu->currentOption) {
        case 0:
             menu->screen = GAMEOPTION_SCREEN;
            break;
        case 1:
            menu->screen = LEADERBOARD_SCREEN;
            break;
        case 2:
            game->phase = GAME_EXIT;
            break;
    }
}