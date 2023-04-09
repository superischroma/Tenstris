#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "tenstris.h"

static state_t** model_key;
static unsigned char* model_value;
unsigned model_size;
static unsigned model_capacity;

int state_cmp(state_t* s1, state_t* s2)
{
    return memcmp((char*) s1, (char*) s2, sizeof(state_t));
}

unsigned hash(state_t* state)
{
    unsigned r = 2166136261;
    #define mix(c) \
        r ^= (c); \
        r *= 16777619;
    char* p = state->playfield;
    for (; *p; ++p)
    {
        mix(*p)
    }
    mix(state->x)
    mix(state->y)
    mix(state->rot_stage)
    return r;
}

void model_init(int capacity)
{
    model_key = calloc(capacity, sizeof(state_t*));
    model_value = calloc(capacity, sizeof(unsigned char));
    model_size = 0;
    model_capacity = capacity;
}

static void model_chk_rehash()
{
    if (model_size < model_capacity * 0.35)
        return;
    state_t** nkey = calloc(model_capacity * 2, sizeof(state_t*));
    unsigned char* nvalue = calloc(model_capacity * 2, sizeof(unsigned char));
    for (int j = 0; j < model_capacity; j++)
    {
        if (model_key[j] == NULL)
            continue;
        for (int i = hash(model_key[j]) % (model_capacity * 2);; i++)
        {
            if (i >= (model_capacity * 2)) i = 0;
            if (nkey[i] != NULL)
                continue;
            nkey[i] = model_key[j];
            nvalue[i] = model_value[j];
            break;
        }
    }
    model_capacity *= 2;
    free(model_key);
    model_key = nkey;
    free(model_value);
    model_value = nvalue;
}

unsigned char model_put(state_t* k, unsigned char v)
{
    model_chk_rehash();
    for (int i = hash(k) % model_capacity;; i++)
    {
        if (i >= model_capacity) i = 0;
        if (model_key[i] != NULL && !state_cmp(model_key[i], k))
        {
            model_value[i] = v;
            break;
        }
        if (model_key[i] != NULL)
            continue;
        model_key[i] = k;
        model_value[i] = v;
        model_size++;
        break;
    }
    return v;
}

unsigned char model_get(state_t* k)
{
    for (int i = hash(k) % model_capacity; model_key[i] != NULL; i++)
    {
        if (i >= model_capacity) i = 0;
        if (model_key[i] && !state_cmp(model_key[i], k))
            return model_value[i];
    }
    return 0;
}

// does not deallocate memory at K
bool model_erase(state_t* k)
{
    for (int i = hash(k) % model_capacity; model_key[i] != NULL; i++)
    {
        if (i >= model_capacity) i = 0;
        if (!state_cmp(model_key[i], k))
        {
            model_key[i] = NULL;
            model_value[i] = 0;
            return true;
        }
    }
    return false;
}

void model_delete()
{
    free(model_key);
    free(model_value);
}