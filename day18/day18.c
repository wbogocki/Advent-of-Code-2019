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

#define MAX_TILES 1024
#define MAX_KEYS_AND_DOORS 26
#define MAX_TILES_TO_CHECK 16

#define MAX_PATH_FIND_OPEN_SET 16
#define MAX_PATH_FIND_CAME_FROM 16

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

struct path_find_node
{
    struct position pos;
    int score;
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
    return ((pos.x >= 0 && pos.x <= game->fixed->w) &&
            (pos.y >= 0 && pos.y <= game->fixed->h));
}

struct position position_add(struct position a, struct position b)
{
    return (struct position){.x = a.x + b.x, .y = a.y + b.y};
}

char game_tile(struct game *game, struct position pos)
{
    assert(position_valid(game, pos));
    return (game->fixed->tiles[(pos.y * game->fixed->w) + pos.x]);
}

struct path path_find_reconstruct_path()
{
}

int path_find_score(struct position pos, struct position target)
{
    return 1000 - distance(pos, target);
}

struct path_find_node path_find_pop_best_node(struct path_find_node *open_set, size_t *open_set_len)
{
    int best_idx = 0, best_score = 0;
    for (int i = 0; i < *open_set_len; ++i)
    {
        if (best_score < open_set[i].score)
        {
            best_idx = 0;
            best_score = open_set[i].score;
        }
    }
    struct path_find_node best_node = open_set[best_idx];
    open_set[best_idx] = open_set[(*open_set_len)--];
    return best_node;
}

struct path path_find(struct game *game, struct position from, struct position to)
{
    // Implements the A* algorithm

    struct path_find_node open_set[MAX_PATH_FIND_OPEN_SET] = {0};
    size_t open_set_len = 0;

    struct position came_from[MAX_PATH_FIND_CAME_FROM] = {0};
    size_t came_from_len = 0;

    // Initialize

    open_set[open_set_len++] = (struct path_find_node){
        .pos = from,
        .score = path_find_score(from, to),
    };

    // Search

    while (open_set_len != 0)
    {
        // Get node with greatest score as next

        struct path_find_node current = path_find_pop_best_node(open_set, &open_set_len);
        if (position_equal(current.pos, to))
        {
            return path_find_reconstruct_path();
        }

        // Check neighbors

        struct position neighbor_offsets[4] = {
            [0] = {.x = +0, .y = -1}, // Top
            [1] = {.x = +1, .y = +0}, // Right
            [2] = {.x = +0, .y = +1}, // Bottom
            [3] = {.x = -1, .y = +0}, // Left
        };

        for (int i = 0; i < 4; ++i)
        {
            struct position neighbor = position_add(current.pos, neighbor_offsets[i]);
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

            int tentative_score = current.score;

            {
            }
        }
    }
}

void game_print(struct game *game)
{
    printf("w=%d\nh=%d\n", game->fixed->w, game->fixed->h);
    for (size_t y = 0; y < game->fixed->h; ++y)
    {
        for (size_t x = 0; x < game->fixed->w; ++x)
        {
            char tile = game_tile(&game, (struct position){x, y});
            putchar(tile);
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