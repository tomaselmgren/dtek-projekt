#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "tetris.h"  
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

uint32_t pcg32_random_r(struct Game_State *game) {
    uint64_t oldstate = game->rng.state;
    game->rng.state = oldstate * 6364136223846793005ULL + game->rng.increment;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void pcg32_srandom_r(struct Game_State *game, uint64_t initstate, uint64_t initseq) {
    game->rng.state = 0U;
    game->rng.increment = (initseq << 1u) | 1u;
    pcg32_random_r(game);
    game->rng.state += initstate;
    pcg32_random_r(game);
}

uint32_t pcg32_boundedrand_r(struct Game_State *game, uint32_t bound) {
    uint32_t threshold = -bound % bound;
    for (;;) {
        uint32_t r = pcg32_random_r(game);
        if (r >= threshold) {
            return r % bound;
        }
    }
}

int volatile state = 0;
int volatile seq = 0;

void seed(struct Game_State *game) {
    int btns = getbtns();

    if (IFS(0) & 0x100) {
        IFS(0) = 0;
        state++;

        if (state % 2) {
           seq++;
        }
    }

    if (btns > 0 && game->rng.has_seed != true) {
        pcg32_srandom_r(game, state, seq);
        game->rng.has_seed = true;
    }
}