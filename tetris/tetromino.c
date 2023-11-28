#include <stdint.h> 
#include "tetris.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/**
 * Temporary file, to be integrated with game functions once we can comprehensively test on board.
 * 
*/


#define TETROMINO_SIZE 4
// Define tetrominoes
int I_Tetromino[TETROMINO_SIZE][TETROMINO_SIZE] = {
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

int L_Tetromino[TETROMINO_SIZE][TETROMINO_SIZE] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0}
};

int O_Tetromino[TETROMINO_SIZE][TETROMINO_SIZE] = {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
};

int S_Tetromino[TETROMINO_SIZE][TETROMINO_SIZE] = {
    {0, 0, 0, 0},
    {0, 0, 1, 1},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
};

int Z_Tetromino[TETROMINO_SIZE][TETROMINO_SIZE] = {
    {0, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}};

int T_Tetromino[TETROMINO_SIZE][TETROMINO_SIZE] = {
    {0, 0, 0, 0},
    {0, 1, 1, 1},
    {0, 0, 1, 0},
    {0, 0, 0, 0}
};

// Rotate the 4x4 Matrix
void rotateTetromino(int tetromino[TETROMINO_SIZE][TETROMINO_SIZE], int *rotationState)
{
    int temp[TETROMINO_SIZE][TETROMINO_SIZE];

    for (int y = 0; y < TETROMINO_SIZE; y++)
    {
        for (int x = 0; x < TETROMINO_SIZE; x++)
        {
            temp[y][x] = tetromino[TETROMINO_SIZE - x - 1][y];
        }
    }

    // Copy back to the original tetromino
    for (int y = 0; y < TETROMINO_SIZE; y++)
    {
        for (int x = 0; x < TETROMINO_SIZE; x++)
        {
            tetromino[y][x] = temp[y][x];
        }
    }

    // Make sure rotation stays between 0-3
    *rotationState = (*rotationState + 1) % 4;
}

// Function to determine if the given cell is a part of a tetromino or not given the rotation, row and column
bool isTetrominoCell(int tetromino[TETROMINO_SIZE][TETROMINO_SIZE], int row, int col, int rotationState) {
    int rotatedRow, rotatedCol;

    // Adjust row and column indices based on the rotation state
    switch (rotationState % 4)
    {       // Use modulo 4 to cycle through rotation states
    case 0: // 0 degrees - no rotation
        rotatedRow = row;
        rotatedCol = col;
        break;
    case 1: // 90 degrees
        rotatedRow = col;
        rotatedCol = TETROMINO_SIZE - row - 1;
        break;
    case 2: // 180 degrees
        rotatedRow = TETROMINO_SIZE - row - 1;
        rotatedCol = TETROMINO_SIZE - col - 1;
        break;
    case 3: // 270 degrees
        rotatedRow = TETROMINO_SIZE - col - 1;
        rotatedCol = row;
        break;
    }

    // Check if the cell is filled in the tetromino matrix
    return tetromino[rotatedRow][rotatedCol] != 0;
};

    /**
     * These function below track the top left coordinate of the matrix.
     */

bool canMoveLeft(int tetromino[TETROMINO_SIZE][TETROMINO_SIZE], int x, int y, int rotationState)
{
    for (int col = 0; col < TETROMINO_SIZE; col++)
    {
        for (int row = 0; row < TETROMINO_SIZE; row++)
        {
            if (isTetrominoCell(tetromino, row, col, rotationState))
            {
                if (col + x - 1 < 0 || gameBoard[row + y][col + x - 1] != 0)
                {
                    return false; // Can't move left
                }
            }
        }
    }
    return true;
}

void moveLeft(int tetromino[TETROMINO_SIZE][TETROMINO_SIZE], int *x, int *y)
{
    if (canMoveLeft(tetromino, *x, *y))
    {
        (*x)--;
    }
}

bool canMoveRight(int tetromino[TETROMINO_SIZE][TETROMINO_SIZE], int x, int y)
{
    for (int col = TETROMINO_SIZE - 1; col >= 0; col--)
    {
        for (int row = 0; row < TETROMINO_SIZE; row++)
        {
            if (tetromino[row][col] != 0)
            {
                if (col + x + 1 >= BOARD_WIDTH || gameBoard[row + y][col + x + 1] != 0)
                {
                    return false; // Can't move right
                }
                break; // Only check the rightmost column of each row
            }
        }
    }
    return true;
}

void moveRight(int tetromino[TETROMINO_SIZE][TETROMINO_SIZE], int *x, int *y)
{
    if (canMoveRight(tetromino, *x, *y))
    {
        (*x)++;
    }
}

bool canMoveDown(int tetromino[TETROMINO_SIZE][TETROMINO_SIZE], int x, int y)
{
    for (int row = TETROMINO_SIZE - 1; row >= 0; row--)
    {
        for (int col = 0; col < TETROMINO_SIZE; col++)
        {
            if (tetromino[row][col] != 0)
            {
                if (row + y + 1 >= BOARD_HEIGHT || gameBoard[row + y + 1][col + x] != 0)
                {
                    return false; // Can't move down
                }
                break; // Only check the bottom row of each column
            }
        }
    }
    return true;
}

void moveDown(int tetromino[TETROMINO_SIZE][TETROMINO_SIZE], int *x, int *y)
{
    if (canMoveDown(tetromino, *x, *y))
    {
        (*y)++;
    }
}



// Temporary, integrate later with Olle's board logic
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

int gameBoard[BOARD_HEIGHT][BOARD_WIDTH];

/**
 * Checks if a row with rowIndex is completed. 
 * Returns true if it is completed, false if it has any empty cells.
*/
bool isRowComplete(int rowIndex)
{
    for (int col = 0; col < BOARD_WIDTH; col++)
    {
        if (gameBoard[rowIndex][col] == 0)
        {
            return false; // Found an empty cell, so the row is not complete
        }
    }
    return true; // No empty cells found, the row is complete
}


/**
 * Removes the given row and shifts down the rows above it.
*/
void removeRow(int rowIndex)
{
    // Shift all rows above down by one
    for (int y = rowIndex; y > 0; y--)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            gameBoard[y][x] = gameBoard[y - 1][x];
        }
    }

    // Clear the top row (now empty)
    for (int x = 0; x < BOARD_WIDTH; x++)
    {
        gameBoard[0][x] = 0;
    }
}

/**
 * Func that should be called in main game loop every collision.
 */
void checkAndClearRows()
{
    int linesCleared = 0;
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        if (isRowComplete(y))
        {
            removeRow(y);
            linesCleared++;
        }
    }

    if (linesCleared > 0)
    {
        updateScore(linesCleared);
    }
}

// TEMPORARY
int score = 0;

/**
 * Simple function to update the score, dependent on level and how many lines are cleared.
 * It's a basic version of this: https://tetris.wiki/Scoring.
 */
void updateScore(int linesCleared, int level)
{
    switch (linesCleared)
    {
    case 1:
        score += 100 * level;
        break;
    case 2:
        score += 300 * level;
        break;
    case 3:
        score += 500 * level;
        break;
    case 4:
        score += 800 * level;
        break;
    default:
        break; 
    }
}

void create_gameboard() {
  int small_gameboard[24][10] = {0};
  small_gameboard[0][0] = 1;
  int tetromino = convert_tetromino(S_Tetromino);
  merge_tetromino(tetromino, 18, 0);
}

int convert_tetromino(int tetromino[4][4]) {
  int largeTetromino[12][12] = {0};

    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {

        int startY = i * 3;
        int startX = j * 3;

        for (int x = startX; x < startX + 3; x++) {
          for (int y = startY; y < startY + 3; y++) {
            largeTetromino[y][x] = tetromino[i][j];
          }
        }
      }
    }

  return largeTetromino;
}

void merge_tetromino(int tetromino[12][12], int x, int y) {
  if ((x + 12) < 30 || x >= 0 || (y + 12) < 72 || y >= 0) {
    for (int i = 0; i < 12; i++) {
      for (int j = 0; j < 12; j++) {
        if (tetromino[11-i][j] == 1) {
          gameboard[y + i][x + j] = 1;
        }
      }
    }
  }
}

void convertArray(int smallArr[24][10], int largeArr[24*3][10*3]) {
    // Iterate through the elements of the smaller array
    for (int i = 0; i < 24; i++) {
        for (int j = 0; j < 10; j++) {

            int row = i * 3 + 1;
            int col = j * 3 + 1;
            if (smallArr[i][j] == 1) {
              render_block(col, row);
            }   
        }  
    }
}

void render_block(int i, int j) {
    int startX = i - 1; // Move back by one unit to center the 3x3 block
    int startY = j - 1; // Move back by one unit to center the 3x3 block
    
    for (int x = startX; x < startX + 3; x++) {
        for (int y = startY; y < startY + 3; y++) {
            gameboard[y][x] = 1;
        }
    }
}