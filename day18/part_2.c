/**
 * Challenge:   Advent of Code 2019
 * Day:         18
 * Part:        2
 * Author:      Wojciech Bogócki
 * Date:        08 Feb 2021 Taipei
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

// Too big and iteration becomes slow, too small and lookups become slow
#define TABLE_SIZE 1024

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

void board_patch(Board *board, size_t x, size_t y, char tile)
{
    assert(x < board->size.x);
    assert(y < board->size.y);
    char *tile_ptr = array_get(board->tiles, (y * board->size.x) + x);
    *tile_ptr = tile;
}

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

    // Patch the entrences
    Vector center = {.x = size.x / 2, .y = size.y / 2};
    board_patch(board, center.x - 1, center.y - 1, '@');
    board_patch(board, center.x + 0, center.y - 1, '#');
    board_patch(board, center.x + 1, center.y - 1, '@');
    board_patch(board, center.x - 1, center.y, '#');
    board_patch(board, center.x + 0, center.y, '#');
    board_patch(board, center.x + 1, center.y, '#');
    board_patch(board, center.x - 1, center.y + 1, '@');
    board_patch(board, center.x + 0, center.y + 1, '#');
    board_patch(board, center.x + 1, center.y + 1, '@');

    return board;
}

char board_tile(Board *board, size_t x, size_t y)
{
    assert(x < board->size.x);
    assert(y < board->size.y);
    return *(char *)array_get(board->tiles, (y * board->size.x) + x);
}

int board_quadrant(Board *board, size_t x, size_t y)
{
    size_t width = board->size.x;
    size_t height = board->size.y;

    if (x <= width / 2)
    {
        if (y <= height / 2)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }
    else
    {
        if (y <= height / 2)
        {
            return 0;
        }
        else
        {
            return 3;
        }
    }
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
// PATH
//
// Shortest path between two points on the map given a set of keys.
//

struct PathNode
{
    Vector pos;
    Board *board; // unowned
    Array *keys;  // unowned
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
    return abs((int)node->pos.x - (int)goal.x) + abs((int)node->pos.y - (int)goal.y);
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

Array *path_reconstruct(Table *came_from, PathNode *current)
{
    Array *total_path = array_create(sizeof(PathNode), 0);
    assert(total_path);

    while (current)
    {
        array_push(total_path, current);

        char *key = path_table_key(current);
        current = table_get(came_from, key);
        free(key);
    }

    array_pop(total_path, NULL); // pop the starting node
    array_reverse(total_path);

    return total_path;
}

Array *path_neighbors(PathNode *node, Vector goal)
{
    size_t x = node->pos.x;
    size_t y = node->pos.y;

    Vector neighbor_tiles[] = {
        {x, y - 1}, // top
        {x, y + 1}, // bottom
        {x - 1, y}, // left
        {x + 1, y}, // right
    };

    Array *out = array_create(sizeof(PathNode), 4);

    for (size_t i = 0; i < 4; ++i)
    {
        Vector pos = neighbor_tiles[i];

        if (pos.x >= 0 && pos.x < node->board->size.x &&
            pos.y >= 0 && pos.y < node->board->size.y)
        {
            char tile = board_tile(node->board, pos.x, pos.y);

            if (tile == '#')
            {
                // It's a wall!
                continue;
            }

            if (tile >= 'a' && tile <= 'z' && (pos.x != goal.x || pos.y != goal.y))
            {
                if (!array_contains(node->keys, &tile))
                {
                    // It's an uncollected key and not our goal!
                    continue;
                }
            }

            if (tile >= 'A' && tile <= 'Z')
            {
                char key = tolower(tile);
                if (!array_contains(node->keys, &key))
                {
                    // It's a locked door!
                    continue;
                }
            }

            PathNode neighbor = {.board = node->board, .keys = node->keys, .pos = pos};
            array_push(out, &neighbor);
        }
    }

    return out;
}

Array *path_find(PathNode start, Vector goal)
{
    char *key = path_table_key(&start);

    // Init open set
    Table *open_set = table_create(TABLE_SIZE);
    table_set(open_set, key, &start, sizeof(PathNode));

    // Init come-from set
    Table *came_from = table_create(TABLE_SIZE);

    // Init g-score set
    Table *g_score = table_create(TABLE_SIZE);
    int score = 0;
    table_set(g_score, key, &score, sizeof(int));

    // Init f-score set
    Table *f_score = table_create(TABLE_SIZE);
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

            // Double check
            PathNode *last_node = array_get(out, array_size(out) - 1);
            assert(last_node->pos.x == goal.x && last_node->pos.y == goal.y &&
                   "The path ends on the desired tile!");

            goto cleanup;
        }

        table_unset(open_set, current_key);

        Array *neighbors = path_neighbors(&current, goal);
        for (size_t i = 0; i < array_size(neighbors); ++i)
        {
            PathNode *neighbor = array_get(neighbors, i);
            char *neighbor_key = path_table_key(neighbor);

            int infinity = INT_MAX / 2;
            int g_score_current = *(int *)table_get_default(g_score, current_key, &infinity);
            int g_score_neighbor = *(int *)table_get_default(g_score, neighbor_key, &infinity);

            int distance = 1;

            int tentative_g_score = g_score_current + distance;

            if (tentative_g_score < g_score_neighbor)
            {
                int f_score_neighbor = g_score_neighbor + path_heuristic(neighbor, goal);

                table_set(came_from, neighbor_key, &current, sizeof(PathNode));
                table_set(g_score, neighbor_key, &tentative_g_score, sizeof(int));
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
// SEQUENCE
//
// Shortest key order to complete the labyrinth in.
//

struct SeqNode
{
    Board *board;          // unowned
    Array *keys_owned;     // owned
    Array *keys_not_owned; // owned
    Vector pos[4];
    int robot;          // current robot
    char tile;          // current tile
    int distance;       // distance travelled from the previous node
    int total_distance; // distance travelled from the first node
};

typedef struct SeqNode SeqNode;

SeqNode *seq_node_first(Board *board)
{
    Array *keys_owned = array_create(sizeof(char), 26);
    assert(keys_owned);

    Array *keys_not_owned = array_create(sizeof(char), 26);
    assert(keys_not_owned);

    Vector pos[4] = {0};

    for (size_t y = 0; y < board->size.y; ++y)
    {
        for (size_t x = 0; x < board->size.x; ++x)
        {
            char tile = board_tile(board, x, y);

            if (tile == '@')
            {
                int robot = board_quadrant(board, x, y);
                pos[robot] = (Vector){.x = x, .y = y};
            }
            else if (tile >= 'a' && tile <= 'z')
            {
                array_push(keys_not_owned, &tile);
            }
        }
    }

    SeqNode *node = malloc(sizeof(SeqNode));
    assert(node);

    *node = (SeqNode){
        .board = board,
        .keys_owned = keys_owned,
        .keys_not_owned = keys_not_owned,
        .pos = {pos[0], pos[1], pos[2], pos[3]},
        .robot = 0,
        .tile = '@',
        .distance = 0,
        .total_distance = 0,
    };

    return node;
}

void seq_node_destroy(SeqNode *node)
{
    // The board data is unowned so don't destory it
    array_destroy(node->keys_not_owned);
    array_destroy(node->keys_owned);
}

SeqNode *seq_node_neighbor(SeqNode *node, int robot, Vector robot_pos, int distance)
{
    char key = board_tile(node->board, robot_pos.x, robot_pos.y);

    Array *keys_owned = array_create(sizeof(char), 26);
    assert(keys_owned);

    Array *keys_not_owned = array_create(sizeof(char), 26);
    assert(keys_not_owned);

    for (int i = 0; i < array_size(node->keys_owned); ++i)
    {
        char owned_key = *(char *)array_get(node->keys_owned, i);
        array_push(keys_owned, &owned_key);
    }

    array_push(keys_owned, &key);

    for (int i = 0; i < array_size(node->keys_not_owned); ++i)
    {
        char not_owned_key = *(char *)array_get(node->keys_not_owned, i);
        if (not_owned_key != key)
        {
            array_push(keys_not_owned, &not_owned_key);
        }
    }

    Vector pos[4] = {0};
    memcpy(pos, node->pos, sizeof(Vector) * 4);
    pos[robot] = robot_pos;

    SeqNode *out = malloc(sizeof(SeqNode));
    assert(out);

    *out = (SeqNode){
        .board = node->board,
        .keys_owned = keys_owned,
        .keys_not_owned = keys_not_owned,
        .pos = {pos[0], pos[1], pos[2], pos[3]},
        .robot = robot,
        .tile = key,
        .distance = distance,
        .total_distance = node->total_distance + distance,
    };

    return out;
}

void seq_node_print(SeqNode *node)
{
    printf("Positions:\t[");
    for (size_t i = 0; i < 4; ++i)
    {
        printf("(%zu, %zu)", node->pos[i].x, node->pos[i].y);
        if (i != 3)
        {
            printf(", ");
        }
    }
    printf("]\n");

    printf("Keys owned:\t[");
    for (size_t i = 0; i < array_size(node->keys_owned); ++i)
    {
        putchar(*(char *)array_get(node->keys_owned, i));
        if (i != array_size(node->keys_owned) - 1)
        {
            printf(", ");
        }
    }
    printf("]\n");

    printf("Keys not owned:\t[");
    for (size_t i = 0; i < array_size(node->keys_not_owned); ++i)
    {
        putchar(*(char *)array_get(node->keys_not_owned, i));
        if (i != array_size(node->keys_not_owned) - 1)
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

// NOTE: Has the side effect of sorting the node->keys_owned array.
char *seq_table_key(SeqNode *node)
{
    size_t buf_size = 64;
    char *buf = calloc(buf_size, sizeof(char));
    assert(buf);

    array_qsort(node->keys_owned, char_cmp);

    snprintf(buf, buf_size, "%c,%c,%c,%c,%*s",
             board_tile(node->board, node->pos[0].x, node->pos[0].y),
             board_tile(node->board, node->pos[1].x, node->pos[1].y),
             board_tile(node->board, node->pos[2].x, node->pos[2].y),
             board_tile(node->board, node->pos[3].x, node->pos[3].y),
             (int)array_size(node->keys_owned),
             (char *)node->keys_owned->data);

    return buf;
}

int seq_heuristic(SeqNode *node)
{
    return array_size(node->keys_not_owned);
}

// The distance required to go to this node from the previous one
int seq_distance(SeqNode *current, SeqNode *neighbor)
{
    return neighbor->distance;
}

// The returned key needs to be freed
char *seq_next_node(Table *open_set, Table *f_score)
{
    int best_score = INT_MAX;
    char *best_key = NULL;

    for (TableEntry *entry = table_next(open_set, NULL);
         entry;
         entry = table_next(open_set, entry))
    {
        int score = *(int *)table_get(f_score, entry->key);
        if (score < best_score)
        {
            best_score = score;
            best_key = entry->key;
        }
    }

    return strdup(best_key);
}

bool seq_is_goal(SeqNode *node)
{
    return array_size(node->keys_not_owned) == 0;
}

// The reconstructed path will be backwards starting from the target node
Array *seq_reconstruct(Table *came_from, SeqNode *current)
{
    Array *total_path = array_create(sizeof(SeqNode *), 0);
    assert(total_path);

    while (current)
    {
        array_push(total_path, &current);

        char *key = seq_table_key(current);

        // printf("%s\n", key);

        SeqNode **current_ptr = (SeqNode **)table_get(came_from, key);
        current = current_ptr
                      ? *current_ptr
                      : NULL;
        free(key);
    }

    array_pop(total_path, NULL); // pop the starting node
    array_reverse(total_path);

    return total_path;
}

Array *seq_neighbors(SeqNode *node)
{
    Array *out = array_create(sizeof(SeqNode *), 32);

    for (size_t i = 0; i < array_size(node->keys_not_owned); ++i)
    {
        char key = *(char *)array_get(node->keys_not_owned, i);

        char key_str[] = {key, '\0'};
        Vector key_pos = *(Vector *)table_get(node->board->keys, key_str);

        int robot = board_quadrant(node->board, key_pos.x, key_pos.y);

        PathNode start = {
            .board = node->board,
            .keys = node->keys_owned,
            .pos = node->pos[robot],
        };

        Array *path = path_find(start, key_pos);
        if (path)
        {
            PathNode *last = (PathNode *)array_get(path, array_size(path) - 1);
            Vector pos = last->pos;
            int distance = array_size(path);
            array_destroy(path);

            SeqNode *neighbor = seq_node_neighbor(node, robot, pos, distance);
            array_push(out, &neighbor);
        }
    }

    return out;
}

Array *seq_find(SeqNode *start)
{
    char *key = seq_table_key(start);

    // Init open set
    Table *open_set = table_create(TABLE_SIZE);
    table_set(open_set, key, &start, sizeof(SeqNode *));

    // Init come-from set
    Table *came_from = table_create(TABLE_SIZE);

    // Init g-score set
    Table *g_score = table_create(TABLE_SIZE);
    int score = 0;
    table_set(g_score, key, &score, sizeof(int));

    // Init f-score set
    Table *f_score = table_create(TABLE_SIZE);
    int h = seq_heuristic(start);
    table_set(f_score, key, &h, sizeof(int));

    free(key);

    // Find the path
    Array *out = NULL;
    while (!table_empty(open_set))
    {
        char *current_key = seq_next_node(open_set, f_score);

        SeqNode *current = *(SeqNode **)table_get(open_set, current_key);

        // printf("Choose %c\n", current->tile);

        if (seq_is_goal(current))
        {
            free(current_key);

            out = seq_reconstruct(came_from, current);
            goto cleanup;
        }

        table_unset(open_set, current_key);

        Array *neighbors = seq_neighbors(current);
        for (size_t i = 0; i < array_size(neighbors); ++i)
        {
            SeqNode *neighbor = *(SeqNode **)array_get(neighbors, i);
            char *neighbor_key = seq_table_key(neighbor);

            // printf("%s\n", neighbor_key);

            int infinity = INT_MAX / 2;
            int g_score_current = *(int *)table_get_default(g_score, current_key, &infinity);
            int g_score_neighbor = *(int *)table_get_default(g_score, neighbor_key, &infinity);

            int distance = seq_distance(current, neighbor);

            // printf("%c -> %c\t%d\n", current->tile, neighbor->tile, distance);
            // seq_node_print(neighbor);

            int tentative_g_score = g_score_current + distance;

            if (tentative_g_score < g_score_neighbor)
            {
                int f_score_neighbor = tentative_g_score + seq_heuristic(neighbor);

                // printf("scores %d %d\n", g_score_neighbor, f_score_neighbor);

                table_set(came_from, neighbor_key, &current, sizeof(SeqNode *));
                table_set(g_score, neighbor_key, &tentative_g_score, sizeof(int));
                table_set(f_score, neighbor_key, &f_score_neighbor, sizeof(int));

                table_set(open_set, neighbor_key, &neighbor, sizeof(SeqNode *));
            }

            free(neighbor_key);
        }
        array_destroy(neighbors);

        free(current_key);

        // printf("---\n");
    }

cleanup:
    // We will leak a ton of node memory here, whatever...

    table_destroy(f_score);
    table_destroy(g_score);
    table_destroy(came_from);
    table_destroy(open_set);

    return out;
}

int main()
{
    // This code takes around an hour and tens of gigabytes of memory to execute before printing this:
    //
    // Path:   h,r1,8 -> q,r2,62 -> o,r2,224 -> u,r2,74 -> a,r2,70 -> x,r2,12 -> s,r0,264 -> y,r2,36 -> c,r0,30 -> g,r0,18 -> i,r0,20 -> w,r0,16 -> k,r0,18 -> f,r0,16 -> t,r0,18 -> l,r0,18 -> z,r0,22 -> p,r0,14 -> e,r1,34 -> m,r3,46 -> n,r3,100 -> d,r3,22 -> b,r1,92 ->
    // v,r1,226 -> r,r1,22 -> j,r1,56
    // Steps:  26
    // Dist:   1538

    // This solution assumes that the board is separated exactly into four quadrants and that there are
    // solid, impassable walls between the quadrants.

    Board *board = board_load();
    assert(board);

    // board_print(board);

    SeqNode *first = seq_node_first(board);
    assert(first);

    // seq_node_print(first);

    Array *path = seq_find(first);

    printf("Path:\t");
    for (size_t i = 0; i < array_size(path); ++i)
    {
        SeqNode *node = *(SeqNode **)array_get(path, i);
        printf("%c,r%d,%d", node->tile, node->robot, node->distance);
        if (i != array_size(path) - 1)
        {
            printf(" -> ");
        }
    }
    printf("\n");

    printf("Steps:\t%zu\n", array_size(path));

    SeqNode *last_node = *(SeqNode **)array_get(path, array_size(path) - 1);
    printf("Dist:\t%d\n", last_node->total_distance);
}
