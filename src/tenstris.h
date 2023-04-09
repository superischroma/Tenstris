#ifndef TENSTRIS_H
#define TENSTRIS_H

#define PLAYFIELD_WIDTH 10
#define PLAYFIELD_HEIGHT 20
#define PLAYFIELD_SIZE (PLAYFIELD_WIDTH * PLAYFIELD_HEIGHT)
#define PLAYFIELD_BYTES (PLAYFIELD_SIZE / 8)

#define T_TETRIMINO_DOWN 0
#define T_TETRIMINO_LEFT 1
#define T_TETRIMINO_UP 2
#define T_TETRIMINO_RIGHT 3

#define LEFT 0
#define RIGHT 1

/* model.c */

typedef struct state_t
{
    char x, y;
    char rot_stage;
    unsigned char playfield[PLAYFIELD_BYTES];
} state_t;

extern unsigned model_size;

void model_init(int capacity);
static void model_chk_rehash();
unsigned char model_put(state_t* k, unsigned char v);
unsigned char model_get(state_t* k);
bool model_erase(state_t* k);
void model_delete();

#endif