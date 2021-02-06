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
char *player_table_key(Player *player)
{
    size_t buf_size = 64;
    char *buf = calloc(buf_size, sizeof(char));

    array_qsort(player->keys_owned, char_cmp);

    assert(array_size(player->keys_owned) <= INT_MAX);
    snprintf(buf, buf_size, "%zu,%zu,%*s", player->pos.x, player->pos.y,
             (int)array_size(player->keys_owned), (char *)player->keys_owned->data);

    return buf;
}

//
// PATH FINDING
//

struct Path
{
    size_t length;
};

typedef struct Path Path;

int path_heuristic(Player *player)
{
    return 0;
}

// NOTE: The returned key needs to be freed
char *path_next_node(Table *open_set, Table *f_score)
{
    int best_score = INT_MAX;
    char *best_node = NULL;

    for (TableEntry *entry = table_next(open_set, NULL);
         entry;
         entry = table_next(open_set, entry))
    {
        int score = *(int *)table_get(f_score, entry->key);
        if (score < best_score)
        {
            best_score = score;
            best_node = entry->key;
        }
    }

    return strdup(best_node);
}

bool path_is_goal(Player *player)
{
    return array_size(player->keys_not_owned) == 0;
}

Path *path_reconstruct(Table *came_from, Player *current)
{
    return NULL;
}

Array *path_neighbors(Player *player)
{
    return NULL;
}

int path_distance(Player *from, Player *to)
{
    (void)from;
    (void)to;
    return 1; // Distance from one tile to another, always 1
}

Path *path_find(Player *player, Vector goal)
{
    char *key = player_table_key(player);

    // Init open set
    Table *open_set = table_create(1024 * 1024);
    table_set(open_set, key, &player, sizeof(Player *));

    // Init come-from set
    Table *came_from = table_create(1024 * 1024);

    // Init g-score set
    Table *g_score = table_create(1024 * 1024);
    int score = 0;
    table_set(g_score, key, &score, sizeof(int));

    // Init f-score set
    Table *f_score = table_create(1024 * 1024);
    int h = path_heuristic(player);
    table_set(f_score, key, &h, sizeof(int));

    free(key);

    // Find the path
    Path *out = NULL;
    while (!table_empty(open_set))
    {
        char *current_key = path_next_node(open_set, f_score);

        Player *current = *(Player **)table_get(open_set, current_key);

        if (path_is_goal(current))
        {
            out = path_reconstruct(came_from, current);
            goto cleanup;
        }

        table_unset(open_set, current_key);

        Array *neighbors = path_neighbors(current);
        for (size_t i = 0; i < array_size(neighbors); ++i)
        {
            Player *neighbor = array_get(neighbors, i);
            char *neighbor_key = player_table_key(neighbor);

            int infinity = 9999;
            int g_score_current = *(int *)table_get_default(g_score, current_key, &infinity);
            int g_score_neighbor = *(int *)table_get_default(g_score, neighbor_key, &infinity);

            int tentative_score = g_score_current + path_distance(current, neighbor);

            if (tentative_score < g_score_neighbor)
            {
                int f_score_neighbor = g_score_neighbor + path_heuristic(neighbor);

                table_set(came_from, neighbor_key, &current, sizeof(Player *));
                table_set(g_score, neighbor_key, &tentative_score, sizeof(int));
                table_set(f_score, neighbor_key, &f_score_neighbor, sizeof(int));

                if (table_get(open_set, neighbor_key) == NULL)
                {
                    table_set(open_set, neighbor_key, &neighbor, sizeof(Player *));
                }
            }

            free(neighbor_key);
        }
        array_destroy(neighbors);

        free(current_key);
    }

cleanup:
    // TODO: Free visited nodes

    table_destroy(f_score);
    table_destroy(g_score);
    table_destroy(came_from);
    table_destroy(open_set);

    return out;
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
    player_table_key(player, repr, sizeof(repr));

    printf("repr: %s\n", repr);

    player_print(player);
}
