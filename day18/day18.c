/*
 ** Advent of Code 2019 **
 ** Day 18 Part 1 **
 ** Wojciech Bogócki **
 ** 21 Dec 2019 Taipei **
 ** 13 March 2020 Taipei **
 ** 26 Dec 2020 Taipei **
 */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Tunnels
//

#define MAX_TILES 1024
#define MAX_KEYS_AND_DOORS 26
#define MAX_TILES_TO_CHECK 16

struct position
{
    int x, y;
};

const struct position INVALID_POSITION = {.x = 0, .y = 0};

struct path
{
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

bool position_equal(struct position from, struct position to)
{
    return (from.x == to.x && from.y == to.y);
}

struct position position_add(struct position a, struct position b)
{
    return (struct position){.x = a.x + b.x, .y = a.y + b.y};
}

struct path path_find(struct game *game, struct position from, struct position to)
{
    struct path checked_tiles[MAX_TILES] = {0}; // Distances to tiles we checked

    struct position tiles_to_check[MAX_TILES_TO_CHECK] = {0};
    size_t tiles_to_check_len = 0;

    tiles_to_check[tiles_to_check_len++] = from;

    while (tiles_to_check_len > 0)
    {
        struct position tile = tiles_to_check[--tiles_to_check_len];

        // Check this tile

        if (position_equal(tile, to))
        {
        }

        // Check neighboring tiles

        struct position neighbor_offsets[4] = {
            [0] = position_add(from, (struct position){.x = +0, .y = -1}), // Top
            [1] = position_add(from, (struct position){.x = +1, .y = +0}), // Right
            [2] = position_add(from, (struct position){.x = +0, .y = +1}), // Bottom
            [3] = position_add(from, (struct position){.x = -1, .y = +0}), // Left
        };
    }

    return (struct path){0};
}

void game_print(struct game *game)
{
    printf("w=%d\nh=%d\n", game->fixed->w, game->fixed->h);
    for (size_t y = 0; y < game->fixed->h; ++y)
    {
        for (size_t x = 0; x < game->fixed->w; ++x)
        {
            putchar(game->fixed->tiles[(y * game->fixed->w) + x]);
        }
        putchar('\n');
    }
}

struct game game_load(const char *path)
{
    struct game game = {
        .fixed = malloc(sizeof(struct game_fixed)),
        .player_pos = INVALID_POSITION,
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
            char tile = game.fixed->tiles[(y * game.fixed->w) + x];
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

            game.fixed->paths[letter_idx(from_key)][letter_idx(to_key)] = path_find(&game, from_pos, to_pos);
        }
    }

    return game;
}

int main(void)
{
    /*

    Take 2:

    1. Precompute all paths between keys with the gates they pass through
    2. Filter out longer paths with the same or longer paths between two points
    with worse or equal requirements to some shorter paths (maybe even drop them
    in step (1) if I detect them)
    3. Do the search based on paths that are available at any given time give the
    set of keys player possesses
    4. Profit

    */

    struct game game = game_load("day18_input.txt");
    game_print(&game);
}