#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "tetris.h"  
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

uint32_t pcg32_random_r(struct pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->increment;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void pcg32_srandom_r(struct pcg32_random_t* rng, uint64_t initstate, uint64_t initseq) {
    rng->state = 0U;
    rng->increment = (initseq << 1u) | 1u;
    pcg32_random_r(rng);
    rng->state += initstate;
    pcg32_random_r(rng);
}

uint32_t pcg32_boundedrand_r(struct pcg32_random_t* rng, uint32_t bound) {
    uint32_t threshold = -bound % bound;
    for (;;) {
        uint32_t r = pcg32_random_r(rng);
        if (r >= threshold) {
            return r % bound;
        }
    }
}

void seed(struct pcg32_random_t* rng) {
    uint64_t state = 0;
    uint64_t seq = 0;
    int btns = getbtns();

    if (IFS(0) & 0x100) {
        IFS(0) = 0;
        state++;

        if (state % 2) {
            seq++;
        }
    }

    if (btns > 0 && rng->has_seed != true) {
        pcg32_srandom_r(rng, state, seq);
        rng->has_seed = true;
    }
}