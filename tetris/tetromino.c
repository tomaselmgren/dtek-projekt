#include <stdint.h> 
#include "tetris.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

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
    set_next_piece(game, pcg32_boundedrand_r(game, 7));

    game->piece.pos_x = SPAWN_POSITIONS[pcg32_boundedrand_r(game, 3)];
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
            break; 
        case 2:
            if (piece.side == 4) { piece.pos_x += 3 * 3; } else { piece.pos_x += 3; }
            if (can_rotate(game, piece) == 0) {
                game->piece = piece;
            }
            break;
        case 3:
            if (piece.side == 4) { piece.pos_x -= 3 * 3; } else { piece.pos_x -= 3; }
            if (can_rotate(game, piece) == 0) {
                game->piece = piece;
            }
            break;
        case 4:
            if (piece.side == 4) { piece.pos_y += 3 * 3; } else { piece.pos_y += 3; }
            if (can_rotate(game, piece) == 0) {
                game->piece = piece;
            }
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
    for (int row = (piece.side * 3 - 1); row >= 0; --row) {
        for (int col = 0; col < piece.side * 3; col++) {
            if (piece.tetromino[row][col] == 1) {
                if ((piece.pos_x - 3 + col < 0) || (game->data_board[piece.pos_y - row][piece.pos_x - 1 + col] == 1)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool canMoveRight(struct Game_State *game, struct Piece_State piece)
{

    for (int row = (piece.side * 3 - 1); row >= 0; --row) {
        for (int col = (piece.side * 3 - 1); col >= 0; --col) {
            if (piece.tetromino[row][col] == 1) {
                if ((piece.pos_x + 3 + col > 29) || (game->data_board[piece.pos_y - row][piece.pos_x + 1 + col] == 1)) {
                    return false;
                }
            }
        }
    }
    return true;
}


bool canMoveDown(struct Game_State *game, struct Piece_State piece)
{
    for (int row = (piece.side * 3 - 1); row >= 0; row--) {
        for (int col = 0; col < piece.side * 3; col++) {
            if (piece.tetromino[row][col] == 1) {
                if ((piece.pos_y - 1 - row < 0) || (game->data_board[piece.pos_y - 1 - row][piece.pos_x + col] == 1))  {
                   return false;
                }
            }
        }
    }
    return true;
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

bool valid_soft_drop(struct Game_State *game) {
    if (game->tick >= get_time_to_next_drop(game->level)) {
        game->tick = 0;
        return true;
        }
    return false;
}