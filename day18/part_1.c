/**
 * Challenge:   Advent of Code 2019
 * Day:         18
 * Part:        1
 * Author:      Wojciech Bogócki
 * Take 1:      21 Dec 2019 Taipei
 * Take 2:      13 Mar 2020 Taipei
 * Take 3:      26 Dec 2020 Taipei
 * Take 4:      01 Feb 2021 Taipei
 **/

#define ARRAY_IMPL
#define TABLE_IMPL

#include "array.h"
#include "table.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

//
// VECTOR
//

struct Vector
{
    size_t x;
    size_t y;
};

typedef struct Vector Vector;

//
// BOARD
//

struct Board
{
    Array *tiles;
    Vector size;
};

typedef struct Board Board;

Board *board_load()
{
    FILE *file = fopen("input.txt", "r");
    assert(file);

    Array *tiles = array_create(sizeof(char), 0);
    assert(tiles);

    Vector size = {.x = 0, .y = 0};

    for (char c = fgetc(file); c != EOF; c = fgetc(file))
    {
        if (c == '\n')
        {
            size.x = 0;
            size.y += 1;
        }
        else
        {
            array_push(tiles, &c);
            size.x += 1;
        }
    }

    size.y += 1;

    fclose(file);

    Board *board = malloc(sizeof(Board));
    assert(board);

    *board = (Board){.tiles = tiles, .size = size};

    return board;
}

char board_tile(Board *board, size_t x, size_t y)
{
    assert(x < board->size.x);
    assert(y < board->size.y);
    return *(char *)array_get(board->tiles, (y * board->size.x) + x);
}

void board_print(Board *board)
{
    for (size_t y = 0; y < board->size.y; ++y)
    {
        for (size_t x = 0; x < board->size.x; ++x)
        {
            putchar(board_tile(board, x, y));
        }
        putchar('\n');
    }
}

//
// PLAYER
//

struct Player
{
    Board *board;
    Array *keys_owned;
    Array *keys_not_owned;
    Vector pos;
};

typedef struct Player Player;

Player *player_create(Board *board)
{
    Array *keys_owned = array_create(sizeof(char), 36);
    assert(keys_owned);

    Array *keys_not_owned = array_create(sizeof(char), 36);
    assert(keys_not_owned);

    Vector pos = {0};

    for (size_t y = 0; y < board->size.y; ++y)
    {
        for (size_t x = 0; x < board->size.x; ++x)
        {
            char tile = board_tile(board, x, y);

            if (tile == '@')
            {
                pos = (Vector){.x = x, .y = y};
            }
            else if (tile >= 'a' && tile <= 'z')
            {
                array_push(keys_not_owned, &tile);
            }
        }
    }

    Player *player = malloc(sizeof(Player));
    assert(player);

    *player = (Player){
        .board = board,
        .keys_owned = keys_owned,
        .keys_not_owned = keys_not_owned,
        .pos = pos,
    };

    return player;
}

void player_print(Player *player)
{
    printf("Position:\t(%zu, %zu)\n", player->pos.x, player->pos.y);

    printf("Keys owned:\t[");
    for (size_t i = 0; i < array_size(player->keys_owned); ++i)
    {
        putchar(*(char *)array_get(player->keys_owned, i));
        if (i != array_size(player->keys_owned) - 1)
        {
            printf(", ");
        }
    }
    printf("]\n");

    printf("Keys not owned:\t[");
    for (size_t i = 0; i < array_size(player->keys_not_owned); ++i)
    {
        putchar(*(char *)array_get(player->keys_not_owned, i));
        if (i != array_size(player->keys_not_owned) - 1)
        {
            printf(", ");
        }
    }
    printf("]\n");
}

int char_cmp(const void *a, const void *b)
{
    char ca = *(char *)a;
    char cb = *(char *)b;

    if (ca < cb)
    {
        return -1;
    }
    if (ca == cb)
    {
        return 0;
    }
    if (ca > cb)
    {
        return 1;
    }
}

// NOTE: Has the side effect of sorting the player->keys_owned array.
void player_repr(Player *player, char *buf, size_t buf_size)
{
    array_qsort(player->keys_owned, char_cmp);

    assert(array_size(player->keys_owned) <= INT_MAX);
    snprintf(buf, buf_size, "%zu,%zu,%*s", player->pos.x, player->pos.y,
             (int)array_size(player->keys_owned), (char *)player->keys_owned->data);
}

//
// PATH FINDING
//

struct Path
{
    size_t length;
};

typedef struct Path Path;

Path *path_find(Player *player, Vector goal)
{
    Table *open_set = table_create(1024 * 1024);
    Table *came_from = table_create(1024 * 1024);
    Table *g_score = table_create(1024 * 1024);
    Table *f_score = table_create(1024 * 1024);

    table_destroy(f_score);
    table_destroy(g_score);
    table_destroy(came_from);
    table_destroy(open_set);
}

void path_destroy(Path *path) {}

int main()
{
    Board *board = board_load();
    assert(board);

    board_print(board);

    Player *player = player_create(board);
    assert(player);

    player_print(player);

    char key[] = {'c', 'b', 'a'};
    array_push(player->keys_owned, &key[0]);
    array_push(player->keys_owned, &key[1]);
    array_push(player->keys_owned, &key[2]);

    player_print(player);

    char repr[64] = {0};
    player_repr(player, repr, sizeof(repr));

    printf("repr: %s\n", repr);

    player_print(player);
}
