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
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
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
    Array *tiles; // unowned
    Vector size;
    Table *keys;
};

typedef struct Board Board;

Board *board_load()
{
    FILE *file = fopen("input.txt", "r");
    assert(file);

    Array *tiles = array_create(sizeof(char), 0);
    assert(tiles);

    Vector size = {.x = 0, .y = 0};

    Table *keys = table_create(26);
    assert(keys);

    for (char c = fgetc(file); c != EOF; c = fgetc(file))
    {
        if (c >= 'a' && c <= 'z')
        {
            char key[] = {c, '\0'};
            Vector pos = {.x = size.x, .y = size.y};
            table_set(keys, key, &pos, sizeof(Vector));
        }

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

    *board = (Board){.tiles = tiles, .size = size, .keys = keys};

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
// PATH FINDING
//

struct PathNode
{
    Vector pos;
    Board *board;
    Array *keys;
};

typedef struct PathNode PathNode;

char *path_table_key(PathNode *node)
{
    size_t buf_size = 16;
    char *buf = calloc(buf_size, sizeof(char));

    snprintf(buf, buf_size, "%zu %zu", node->pos.x, node->pos.y);

    return buf;
}

int path_heuristic(PathNode *node, Vector goal)
{
    return abs(node->pos.x - goal.x) + abs(node->pos.y - goal.y);
}

// The returned key needs to be freed
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

bool path_is_goal(PathNode *node, Vector goal)
{
    return node->pos.x == goal.x && node->pos.y == goal.y;
}

// The reconstructed path will be backwards starting from the target node
Array *path_reconstruct(Table *came_from, PathNode *current)
{
    Array *total_path = array_create(sizeof(PathNode), 0);
    assert(total_path);

    while (true)
    {
        char *key = path_table_key(current);
        current = table_get(came_from, key);
        free(key);
        if (current)
        {
            array_push(total_path, current);
        }
        else
        {
            break;
        }
    }

    return total_path;
}

Array *path_neighbors(PathNode *node)
{
    size_t x = node->pos.x;
    size_t y = node->pos.y;

    Vector neighbor_tiles[8] = {
        {x + 0, y - 1}, // top
        {x + 1, y - 0}, // right
        {x - 0, y + 1}, // bottom
        {x - 1, y + 0}, // left
    };

    Array *out = array_create(sizeof(PathNode), 4);

    for (size_t i = 0; i < 4; ++i)
    {
        Vector pos = neighbor_tiles[i];

        if (pos.x >= 0 && pos.x < node->board->size.x &&
            pos.y >= 0 && pos.y < node->board->size.y)
        {
            char tile = board_tile(node->board, pos.x, pos.y);

            bool open = true;
            if (tile == '#')
            {
                continue;
            }

            if (tile >= 'A' && tile <= 'Z')
            {
                char key = tolower(tile);
                if (!array_contains(node->keys, &key))
                {
                    continue;
                }
            }

            if (open)
            {
                PathNode neighbor = {.board = node->board, .keys = node->keys, .pos = pos};
                array_push(out, &neighbor);
            }
        }
    }

    return out;
}

Array *path_find(PathNode start, Vector goal)
{
    char *key = path_table_key(&start);

    // Init open set
    Table *open_set = table_create(1024 * 1024);
    table_set(open_set, key, &start, sizeof(PathNode));

    // Init come-from set
    Table *came_from = table_create(1024 * 1024);

    // Init g-score set
    Table *g_score = table_create(1024 * 1024);
    int score = 0;
    table_set(g_score, key, &score, sizeof(int));

    // Init f-score set
    Table *f_score = table_create(1024 * 1024);
    int h = path_heuristic(&start, goal);
    table_set(f_score, key, &h, sizeof(int));

    free(key);

    // Find the path
    Array *out = NULL;
    while (!table_empty(open_set))
    {
        char *current_key = path_next_node(open_set, f_score);

        PathNode current = *(PathNode *)table_get(open_set, current_key);

        if (path_is_goal(&current, goal))
        {
            free(current_key);

            out = path_reconstruct(came_from, &current);
            goto cleanup;
        }

        table_unset(open_set, current_key);

        Array *neighbors = path_neighbors(&current);
        for (size_t i = 0; i < array_size(neighbors); ++i)
        {
            PathNode *neighbor = array_get(neighbors, i);
            char *neighbor_key = path_table_key(neighbor);

            int infinity = 9999;
            int g_score_current = *(int *)table_get_default(g_score, current_key, &infinity);
            int g_score_neighbor = *(int *)table_get_default(g_score, neighbor_key, &infinity);

            int distance = 1;

            int tentative_score = g_score_current + distance;

            if (tentative_score < g_score_neighbor)
            {
                int f_score_neighbor = g_score_neighbor + path_heuristic(neighbor, goal);

                table_set(came_from, neighbor_key, &current, sizeof(PathNode));
                table_set(g_score, neighbor_key, &tentative_score, sizeof(int));
                table_set(f_score, neighbor_key, &f_score_neighbor, sizeof(int));

                if (table_get(open_set, neighbor_key) == NULL)
                {
                    table_set(open_set, neighbor_key, neighbor, sizeof(PathNode));
                }
            }

            free(neighbor_key);
        }
        array_destroy(neighbors);

        free(current_key);
    }

cleanup:
    table_destroy(f_score);
    table_destroy(g_score);
    table_destroy(came_from);
    table_destroy(open_set);

    return out;
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

void player_destroy(Player *player)
{
    // The board data is unowned so don't destory it
    array_destroy(player->keys_not_owned);
    array_destroy(player->keys_owned);
}

Player *player_clone(Player *player)
{
    Player *out = player_create(player->board);

    for (int i = 0; i < array_size(player->keys_owned); ++i)
    {
        char key = *(char *)array_get(player->keys_owned, i);
        array_push(out->keys_owned, &key);
    }

    for (int i = 0; i < array_size(player->keys_not_owned); ++i)
    {
        char key = *(char *)array_get(player->keys_not_owned, i);
        array_push(out->keys_not_owned, &key);
    }

    out->pos = player->pos;

    return out;
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
    else if (ca == cb)
    {
        return 0;
    }
    else
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

int main()
{
    Board *board = board_load();
    assert(board);

    // board_print(board);

    Player *player = player_create(board);
    assert(player);

    // player_print(player);

    // A*

    PathNode start = {
        .board = board,
        .keys = array_create(sizeof(char), 0),
        .pos = player->pos,
    };

    const char *key = "q";
    Vector key_pos = *(Vector *)table_get(board->keys, key);
    Array *path = path_find(start, key_pos);

    if (path)
    {
        printf("Path:\t[");
        for (size_t i = 0; i < array_size(path); ++i)
        {
            PathNode node = *(PathNode *)array_get(path, i);
            printf("%zu,%zu", node.pos.x, node.pos.y);
            if (i != array_size(path) - 1)
            {
                printf(" -> ");
            }
        }
        printf("]\n");
    }
    else
    {
        printf("Path:\tnull");
    }
}
