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

#define START 0
#define SETTINGS 1
#define ABOUT 2
#define EXIT 3

// Add a global 
volatile int currentOption = START;

volatile int currentLevel = 1;
volatile int currentLevelIndex = 0;

volatile int instaDrop = 0;

// Function prototype declaration for execute_option
void execute_option(int option);

// Implement this function to get the user's input and update the currentOption accordingly
void handle_menu() {
  int btn = getbtns();
  if (btn == 4) { // Up
    currentOption = (currentOption - 1 + 4) % 4; // Ensure it wraps around
  } else if (btn == 2) { // Down
    currentOption = (currentOption + 1) % 4;
  } else if (btn == 1) { // Select
    execute_option(currentOption);
  }
}

// Implement this function to get the user's input and update the currentOption accordingly
void handle_gamemode() {
  int btn = getbtns();
  if (btn == 4) { // Up
    currentOption = (currentOption - 1 + 4) % 4; // Ensure it wraps around
  } else if (btn == 2) { // Down
    currentOption = (currentOption + 1) % 4;
  } else if (btn == 1) { // Select
    gameoptions(currentOption);
  }
}

//Increase the level
void increaseLevel() {
  int levels[5] = {1, 5, 10, 20, 25};
  currentLevelIndex = (currentLevelIndex + 1) % 5;
  currentLevel = levels[currentLevelIndex];
  gamestate_set_level(currentLevel);
}

// Implement this function to execute the selected menu option
void gameoptions(int option) {
    switch (option) {
        case 0:
            
            break;
        case 1:
             increaseLevel();
            break;
        case 2:
            gamestate_set_instDrop(!instaDrop);
            break;
        case 3:
            while ( 1 ) {
              handle_menu();
              delay(100);
              render_menu();
            }
            break;
    }
}

// Implement this function to execute the selected menu option
void execute_option(int option) {
    switch (option) {
        case START:
            while ( 1 ) {
              handle_gamemode();
              delay(100);
              gameoptions();
            }
            break;
        case SETTINGS:
            // printf("Settings selected\n");
            break;
        case ABOUT:
            // printf("About selected\n");
            break;
        case EXIT:
            // printf("Exit selected\n");
            break;
    }
}