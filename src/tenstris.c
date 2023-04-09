#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>
#include <string.h>

#include "tenstris.h"

typedef struct tetrimino_t
{
    int x, y;
    char block_offsets[8];
    unsigned char rot_stage;
    void (*rotate)(struct tetrimino_t*);
} tetrimino_t;

tetrimino_t* active;
char* playfield;

bool set_playfield(int x, int y, bool state)
{
    if (x < 0 || y < 0)
        return false;
    int index = x + (y * PLAYFIELD_WIDTH);
    int mask = 1 << (7 - (index % 8));
    if (state)
        playfield[index / 8] |= mask;
    else
        playfield[index / 8] &= ~mask;
    return state;
}

bool get_playfield(int x, int y)
{
    if (x < 0 || y < 0)
        return false;
    int index = x + (y * PLAYFIELD_WIDTH);
    int mask = 1 << (7 - (index % 8));
    return (playfield[index / 8] & mask) != 0;
}

bool tetrimino_position(int x, int y, tetrimino_t* mino)
{
    if (!mino)
        return false;
    for (unsigned i = 0; i < 8; i += 2)
    {
        if (x == (mino->x + mino->block_offsets[i]) && y == (mino->y + mino->block_offsets[i + 1]))
            return true;
    }
    return false;
}

void update_tetrimino_offsets(tetrimino_t* mino, int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8)
{
    mino->block_offsets[0] = i1;
    mino->block_offsets[1] = i2;
    mino->block_offsets[2] = i3;
    mino->block_offsets[3] = i4;
    mino->block_offsets[4] = i5;
    mino->block_offsets[5] = i6;
    mino->block_offsets[6] = i7;
    mino->block_offsets[7] = i8;
}

bool maybe_lock_tetrimino(tetrimino_t* mino)
{
    if (!mino)
        return false;
    bool update = false;
    for (unsigned i = 0; i < 8; i += 2)
    {  
        int x = mino->x + mino->block_offsets[i],
            y = mino->y + mino->block_offsets[i + 1];
        if (!update && (get_playfield(x, y) || get_playfield(x, y + 1) || (y + 1) >= PLAYFIELD_HEIGHT))
        {
            update = true;
            i = -2;
            continue;
        }
        if (update) set_playfield(x, y, true);
    }
    return update;
}

bool chk_game_over(tetrimino_t* mino)
{
    if (!mino)
        return false;
    for (unsigned i = 0; i < 8; i += 2)
    {  
        int x = mino->x + mino->block_offsets[i],
            y = mino->y + mino->block_offsets[i + 1];
        if (get_playfield(x, y))
            return true;
    }
    return false;
}

void drop_tetrimino(tetrimino_t* mino)
{
    ++(mino->y);
}

void clear_row(int y)
{
    memmove(playfield + PLAYFIELD_WIDTH, playfield, y * PLAYFIELD_WIDTH);
    memset(playfield, false, PLAYFIELD_WIDTH);
}

void chk_rows_for_clear()
{
    for (int r = 0; r < PLAYFIELD_HEIGHT; ++r)
    {
        bool clear = true;
        for (int c = 0; c < PLAYFIELD_WIDTH; ++c)
        {
            if (!get_playfield(c, r))
            {
                clear = false;
                break;
            }
        }
        if (clear) clear_row(r);
    }
}

void shift_tetrimino_left(tetrimino_t* mino)
{
    for (unsigned i = 0; i < 8; i += 2)
    {  
        int x = mino->x + mino->block_offsets[i],
            y = mino->y + mino->block_offsets[i + 1];
        if (x == 0 || get_playfield(x - 1, y))
            return;
    }
    --(mino->x);
}

void shift_tetrimino_right(tetrimino_t* mino)
{
    for (unsigned i = 0; i < 8; i += 2)
    {  
        int x = mino->x + mino->block_offsets[i],
            y = mino->y + mino->block_offsets[i + 1];
        if (x == PLAYFIELD_WIDTH - 1 || get_playfield(x + 1, y))
            return;
    }
    ++(mino->x);
}

void rotate_T_tetrimino(tetrimino_t* mino)
{
    unsigned char next = (mino->rot_stage + 1) % 4;
    if (next == T_TETRIMINO_DOWN)
    {
        if (get_playfield(mino->x - 1, mino->y))
            return;
        update_tetrimino_offsets(mino, 0, 0, 0, 1, -1, 0, 1, 0);
    }
    if (next == T_TETRIMINO_LEFT)
    {
        if (get_playfield(mino->x, mino->y - 1))
            return;
        update_tetrimino_offsets(mino, 0, 0, 0, 1, -1, 0, 0, -1);
    }
    if (next == T_TETRIMINO_UP)
    {
        if (get_playfield(mino->x + 1, mino->y))
            return;
        update_tetrimino_offsets(mino, 0, 0, 0, -1, -1, 0, 1, 0);
    }
    if (next == T_TETRIMINO_RIGHT)
    {
        if (get_playfield(mino->x, mino->y + 1))
            return;
        update_tetrimino_offsets(mino, 0, 0, 0, 1, 1, 0, 0, -1);
    }
    mino->rot_stage = next;
}

// unsafe
tetrimino_t* T_tetrimino()
{
    tetrimino_t* mino = calloc(1, sizeof *mino);
    update_tetrimino_offsets(mino, 0, 0, 0, 1, -1, 0, 1, 0);
    mino->x = 5;
    mino->y = 0;
    mino->rot_stage = T_TETRIMINO_DOWN;
    mino->rotate = rotate_T_tetrimino;
    return mino;
}

void print_playfield(FILE* file)
{
    printf("   ------------");
    for (unsigned i = 0; i < PLAYFIELD_SIZE; ++i)
    {
        int x = i % PLAYFIELD_WIDTH,
            y = i / PLAYFIELD_WIDTH;
        if (!i)
            printf("\n%02i |", y);
        else if (i % PLAYFIELD_WIDTH == 0)
            printf("|\n%02i |", y);
        printf(get_playfield(x, y) || tetrimino_position(x, y, active) ? "□" : " ");
    }
    printf("|\n   ------------\n");
}

int main()
{
    time_t t;
    srand((unsigned) time(&t));

    model_init(100);

    playfield = malloc(PLAYFIELD_BYTES);
    active = T_tetrimino();

    while (true)
    {
        printf("Tenstris\n");
        printf("(1) Educate the model\n");
        printf("(2) Watch the model play a game\n");
        printf("(3) About\n");
        printf("(4) Exit\n");
        printf("> ");
        char choice = fgetc(stdin);
        fgetc(stdin); // read newline lol
        switch (choice)
        {
            case '1':
                while (true)
                {
                    print_playfield(stdout);
                    printf("List out three numbers (scale from 0 to 3) of the quality of possible moves.\n");
                    printf("> ");
                    unsigned char left = fgetc(stdin) - '0',
                        right = fgetc(stdin) - '0',
                        rot = fgetc(stdin) - '0';
                    fgetc(stdin); // read newline lol
                    state_t* state = calloc(1, sizeof(state_t));
                    memcpy(state->playfield, playfield, PLAYFIELD_BYTES);
                    state->rot_stage = active->rot_stage;
                    state->x = active->x;
                    state->y = active->y;
                    model_put(state, (left << 4) | (right << 2) | rot);
                    unsigned char actions[4];
                    actions[0] = left;
                    actions[1] = right;
                    actions[2] = rot;
                    actions[3] = 0;
                    int largest = 3;
                    for (unsigned i = 0; i < 3; ++i)
                    {
                        if (actions[i] > actions[largest])
                            largest = i;
                        if (actions[i] == actions[largest])
                            largest = (rand() % 2) ? i : largest;
                    }
                    if (left == right && right == rot)
                        largest = rand() % 3;
                    if (largest == 0)
                        shift_tetrimino_left(active);
                    else if (largest == 1)
                        shift_tetrimino_right(active);
                    else
                        active->rotate(active);
                    drop_tetrimino(active);
                    if (maybe_lock_tetrimino(active))
                    {
                        free(active);
                        active = T_tetrimino();
                    }
                    if (chk_game_over(active))
                        break;
                    chk_rows_for_clear();
                }
                break;
            case '2':
                break;
            case '3':
                break;
            case '4':
                goto exit_program;
            default:
                break;
        }
    }
exit_program:
    
    free(playfield);
    free(active);

    model_delete();
}