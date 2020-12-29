/*
 ** Advent of Code 2019 **
 ** Day 18 Part 1 **
 ** Wojciech Bogócki **
 ** 21 Dec 2019 Taipei **
 ** 13 March 2020 Taipei **
 ** 26 Dec 2020 Taipei **
 */

#define HSET_IMPL
#define HMAP_IMPL

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmap.h"
#include "hset.h"

#define MAX_TILES 8192
#define MAX_KEYS_AND_DOORS 26
#define MAX_TILES_TO_CHECK 16
#define MAX_PATH 1024

#define MAX_PATH_FIND_OPEN_SET 128
#define MAX_PATH_FIND_CAME_FROM 128

struct position
{
    int x, y;
};

struct path
{
    struct position pos[MAX_PATH];
    int path_len;
    // Doors this path passes through
    char doors[MAX_KEYS_AND_DOORS];
    int doors_len;
};

struct game_fixed
{
    char tiles[MAX_TILES];
    size_t w, h;
    struct position keys_pos[MAX_KEYS_AND_DOORS];
    struct position door_pos[MAX_KEYS_AND_DOORS];
    struct path paths[MAX_KEYS_AND_DOORS][MAX_KEYS_AND_DOORS];
};

struct game
{
    struct game_fixed *fixed;
    struct position player_pos;
    char keys[MAX_KEYS_AND_DOORS];
    int keys_len;
};

size_t letter_idx(char letter)
{
    assert(isalpha(letter));
    return islower(letter) ? 'z' - letter : 'Z' - letter;
}

int distance(struct position a, struct position b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
}

bool position_equal(struct position from, struct position to)
{
    return (from.x == to.x && from.y == to.y);
}

bool position_valid(struct game *game, struct position pos)
{
    return ((pos.x >= 0 && pos.x < game->fixed->w) &&
            (pos.y >= 0 && pos.y < game->fixed->h));
}

struct position position_add(struct position a, struct position b)
{
    return (struct position){.x = a.x + b.x, .y = a.y + b.y};
}

bool position_in_array(struct position *array, size_t array_len, struct position pos)
{
    for (int i = 0; i < array_len; ++i)
    {
        if (position_equal(array[i], pos))
        {
            return true;
        }
    }
    return false;
}

size_t game_tile_index(struct game *game, struct position pos)
{
    assert(position_valid(game, pos));
    return (pos.y * game->fixed->w) + pos.x;
}

char game_tile(struct game *game, struct position pos)
{
    assert(position_valid(game, pos));
    return (game->fixed->tiles[(pos.y * game->fixed->w) + pos.x]);
}

void path_find_reverse_path(struct position *pos, size_t len)
{
    size_t left = 0;
    size_t right = len - 1;
    while (left < right)
    {
        struct position tmp = pos[left];
        pos[left++] = pos[right];
        pos[right--] = tmp;
    }
}

struct path path_find_reconstruct_path(struct game *game, struct position *came_from, struct position origin, struct position current)
{
    struct path path = {0};
    path.pos[path.path_len++] = current;

    while (!position_equal(current, origin))
    {
        current = came_from[game_tile_index(game, current)];

        char tile = game_tile(game, current);
        if (tile >= 'A' && tile <= 'Z')
        {
            assert(path.doors_len < MAX_KEYS_AND_DOORS);
            path.doors[path.doors_len++] = tile;
        }

        assert(path.path_len < MAX_PATH);
        path.pos[path.path_len++] = current;
    }

    // We recunstructed the path in reverse
    path_find_reverse_path(path.pos, path.path_len);

    return path;
}

int path_find_heuristic(struct position pos, struct position target)
{
    return distance(pos, target);
}

struct position path_find_pop_best_node(struct game *game, struct position *open_set, size_t *open_set_len, int *scores)
{
    int best_idx = 0, best_score = 0;
    for (int i = 0; i < *open_set_len; ++i)
    {
        int tile_idx = game_tile_index(game, open_set[i]);
        if (best_score < scores[tile_idx])
        {
            best_idx = 0;
            best_score = scores[tile_idx];
        }
    }
    struct position best_node = open_set[best_idx];
    open_set[best_idx] = open_set[--(*open_set_len)];
    return best_node;
}

struct path path_find(struct game *game, struct position from, struct position to)
{
    // Implements the A* algorithm

    struct position open_set[MAX_PATH_FIND_OPEN_SET] = {0};
    size_t open_set_len = 0;

    struct position came_from[MAX_TILES] = {0};

    int g_scores[MAX_TILES] = {0};
    int f_scores[MAX_TILES] = {0};

    // Initialize

    open_set[open_set_len++] = from;

    for (int i = 0; i < MAX_TILES; ++i)
    {
        g_scores[i] = INT_MAX;
        f_scores[i] = INT_MAX;
    }

    g_scores[game_tile_index(game, from)] = 0;
    f_scores[game_tile_index(game, from)] = path_find_heuristic(from, to);

    // Search

    while (open_set_len != 0)
    {
        // Get node with best score as next

        struct position current = path_find_pop_best_node(game, open_set, &open_set_len, f_scores);
        if (position_equal(current, to))
        {
            return path_find_reconstruct_path(game, came_from, from, current);
        }

        //printf("Pop node with the best score: %d,%d\n", current.x, current.y);

        // Check neighbors

        struct position neighbor_offsets[4] = {
            [0] = {.x = +0, .y = -1}, // Top
            [1] = {.x = +1, .y = +0}, // Right
            [2] = {.x = +0, .y = +1}, // Bottom
            [3] = {.x = -1, .y = +0}, // Left
        };

        for (int i = 0; i < 4; ++i)
        {
            struct position neighbor = position_add(current, neighbor_offsets[i]);
            if (!position_valid(game, neighbor))
            {
                continue;
            }

            char tile = game_tile(game, neighbor);
            if (tile == '#')
            {
                continue;
            }

            // Calculate the score

            int current_tile_idx = game_tile_index(game, current);
            int neighbor_tile_idx = game_tile_index(game, neighbor);

            int tentative_g_score = g_scores[current_tile_idx] + 1;
            if (tentative_g_score < g_scores[neighbor_tile_idx])
            {
                came_from[neighbor_tile_idx] = current;
                g_scores[neighbor_tile_idx] = tentative_g_score;
                f_scores[neighbor_tile_idx] = tentative_g_score + path_find_heuristic(neighbor, to);

                if (!position_in_array(open_set, open_set_len, neighbor))
                {
                    assert(open_set_len < MAX_PATH_FIND_OPEN_SET);
                    open_set[open_set_len++] = neighbor;
                }
            }
        }
    }

    return (struct path){0};
}

void game_print(struct game *game)
{
    printf("w=%zu\nh=%zu\n", game->fixed->w, game->fixed->h);

    printf("  ");
    for (size_t x = 0; x < game->fixed->w; ++x)
    {
        printf("%zu", x % 10);
    }
    putchar('\n');

    for (size_t y = 0; y < game->fixed->h; ++y)
    {
        printf("%zu ", y % 10);
        for (size_t x = 0; x < game->fixed->w; ++x)
        {
            char tile = game_tile(game, (struct position){.x = x, .y = y});
            putchar(tile);
        }
        putchar('\n');
    }
}

void game_print_path(struct game *game, char from_key, char to_key)
{
    struct path path = game->fixed->paths[letter_idx(from_key)][letter_idx(to_key)];

    printf("path %c%c through %d doors", from_key, to_key, path.doors_len);

    if (path.doors_len > 0)
    {
        putchar(' ');
        for (size_t i = 0; i < path.doors_len; ++i)
        {
            putchar(path.doors[i]);

            if (i != path.doors_len - 1)
            {
                putchar(',');
            }
        }
    }

    printf(": ");

    if (path.path_len == 0)
    {
        printf("(none)\n");
        return;
    }

    for (size_t i = 0; i < path.path_len; ++i)
    {
        printf("%d,%d", path.pos[i].x, path.pos[i].y);

        if (i != path.path_len - 1)
        {
            printf(" -> ");
        }
    }

    putchar('\n');
}

struct game game_load(const char *path)
{
    struct game game = {
        .fixed = malloc(sizeof(struct game_fixed)),
        .player_pos = {0},
        .keys = {0},
        .keys_len = 0,
    };

    // Load the map file

    FILE *file = fopen(path, "r");

    size_t tiles_len = 0;
    size_t tiles_h = 1;

    char tile = fgetc(file);
    while (tile != EOF)
    {
        if (tile == '\n')
        {
            ++tiles_h;
        }
        else
        {
            assert(tiles_len < MAX_TILES);
            game.fixed->tiles[tiles_len++] = tile;
        }

        tile = fgetc(file);
    }

    game.fixed->w = tiles_len / tiles_h;
    game.fixed->h = tiles_h;

    fclose(file);

    // Find all the keys and doors

    for (size_t y = 0; y < game.fixed->h; ++y)
    {
        for (size_t x = 0; x < game.fixed->w; ++x)
        {
            char tile = game_tile(&game, (struct position){x, y});
            if (tile >= 'a' && tile <= 'z')
            {
                game.fixed->keys_pos[letter_idx(tile)] = (struct position){x, y};
            }
            else if (tile >= 'A' && tile <= 'Z')
            {
                game.fixed->door_pos[letter_idx(tile)] = (struct position){x, y};
            }
        }
    }

    // Find all paths between keys

    for (char from_key = 'a'; from_key <= 'z'; ++from_key)
    {
        for (char to_key = 'a'; to_key <= 'z'; ++to_key)
        {
            if (from_key == to_key)
            {
                continue;
            }

            struct position from_pos = game.fixed->keys_pos[letter_idx(from_key)];
            struct position to_pos = game.fixed->keys_pos[letter_idx(to_key)];

            // Look for paths if these keys exist

            if (!position_equal(from_pos, (struct position){0}) && !position_equal(to_pos, (struct position){0}))
            {
                //printf("Looking for a path between %c and %c\n", from_key, to_key);
                game.fixed->paths[letter_idx(from_key)][letter_idx(to_key)] = path_find(&game, from_pos, to_pos);
            }
        }
    }

    return game;
}

int main(void)
{
    /*

    Take 2:

    1. [x] Precompute all paths between keys with the gates they pass through
    2. [ ] Filter out longer paths with the same or longer paths between two points with worse or equal requirements to some shorter paths (maybe even drop them in step (1) if I detect them)
    3. [ ] Do the search based on paths that are available at any given time give the set of keys player possesses
    4. [ ] Profit

    */

    struct game game = game_load("day18_input.txt");
    game_print(&game);
    game_print_path(&game, 'a', 'z');
}