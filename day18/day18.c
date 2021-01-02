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
#define MAX_GAMES (8192 * 2)

struct position
{
    int x, y;
};

struct path
{
    struct position pos[MAX_PATH];
    int len;
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
    struct path init_paths[MAX_KEYS_AND_DOORS];                // Paths from entrance to any key
    struct path paths[MAX_KEYS_AND_DOORS][MAX_KEYS_AND_DOORS]; // Paths between keys
};

struct game
{
    struct game_fixed *fixed;
    struct position player_pos;
    bool player_keys[MAX_KEYS_AND_DOORS];
    struct path *paths[MAX_KEYS_AND_DOORS]; // Paths taken in this game
    int paths_len;
    int distance; // How many tiles did we travel
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
    path.pos[path.len++] = current;

    while (!position_equal(current, origin))
    {
        current = came_from[game_tile_index(game, current)];

        char tile = game_tile(game, current);
        if (tile >= 'A' && tile <= 'Z')
        {
            assert(path.doors_len < MAX_KEYS_AND_DOORS);
            path.doors[path.doors_len++] = tile;
        }

        assert(path.len < MAX_PATH);
        path.pos[path.len++] = current;
    }

    // We reconstructed the path in reverse
    path_find_reverse_path(path.pos, path.len);

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
            best_idx = i;
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

struct path *game_get_precomputed_path(struct game *game, char from_key_or_entrance, char to_key)
{
    struct path *path = from_key_or_entrance != '@'
                            ? &game->fixed->paths[letter_idx(from_key_or_entrance)][letter_idx(to_key)]
                            : &game->fixed->init_paths[letter_idx(to_key)];
    return path;
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

void game_print_path(struct game *game, struct path *path)
{
    if (path->len > 0)
    {
        char from_tile = game_tile(game, path->pos[0]);
        char to_tile = game_tile(game, path->pos[path->len - 1]);

        printf("path %c-%c through %d doors", from_tile, to_tile, path->doors_len);
    }
    else
    {
        printf("path (unknown) through %d doors", path->doors_len);
    }

    if (path->doors_len > 0)
    {
        putchar(' ');
        for (size_t i = 0; i < path->doors_len; ++i)
        {
            putchar(path->doors[i]);

            if (i != path->doors_len - 1)
            {
                putchar(',');
            }
        }
    }

    // printf(": ");

    // if (path->len == 0)
    // {
    //     printf("(none)\n");
    //     return;
    // }

    // for (size_t i = 0; i < path->len; ++i)
    // {
    //     printf("%d,%d", path->pos[i].x, path->pos[i].y);

    //     if (i != path->len - 1)
    //     {
    //         printf(" -> ");
    //     }
    // }

    putchar('\n');
}

void game_print_path_between(struct game *game, char from_key_or_entrance, char to_key)
{
    struct path *path = game_get_precomputed_path(game, from_key_or_entrance, to_key);
    game_print_path(game, path);
}

void game_print_all_paths(struct game *game)
{
    for (char key = 'a'; key <= 'z'; ++key)
    {
        struct path *path = game_get_precomputed_path(game, '@', key);
        if (path->len > 0)
        {
            game_print_path(game, path);
        }
    }

    for (char from_key = 'a'; from_key <= 'z'; ++from_key)
    {
        for (char to_key = 'a'; to_key <= 'z'; ++to_key)
        {
            struct path *path = game_get_precomputed_path(game, from_key, to_key);
            if (path->len > 0)
            {
                game_print_path(game, path);
            }
        }
    }
}

void game_print_all_paths_starting_at(struct game *game, char from_key_or_entrance)
{
    for (char key = 'a'; key <= 'z'; ++key)
    {
        if (from_key_or_entrance == key)
        {
            continue;
        }

        struct path *path = game_get_precomputed_path(game, from_key_or_entrance, key);
        if (path->len > 0)
        {
            game_print_path(game, path);
        }
    }
}

void game_print_taken_paths(struct game *game)
{
    printf("Taken paths:\n");
    for (size_t i = 0; i < game->paths_len; ++i)
    {
        game_print_path(game, game->paths[i]);
    }
}

struct game game_load(const char *path)
{
    struct game game = {
        .fixed = malloc(sizeof(struct game_fixed)),
        .player_pos = {0},
        .player_keys = {false},
        .paths = {0},
        .paths_len = 0,
        .distance = 0,
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
            else if (tile == '@')
            {
                game.player_pos = (struct position){x, y};
            }
        }
    }

    // Find all paths from entrance to keys

    for (char key = 'a'; key <= 'z'; ++key)
    {
        struct position key_pos = game.fixed->keys_pos[letter_idx(key)];

        if (!position_equal(key_pos, (struct position){0}))
        {
            game.fixed->init_paths[letter_idx(key)] = path_find(&game, game.player_pos, key_pos);
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

//

int get_path_length(struct game *game, const char *path, size_t path_len)
{
    int length = 0;
    for (size_t i = 1; i < path_len; ++i)
    {
        struct path *precom_path = game_get_precomputed_path(game, path[i - 1], path[i]);
        length += precom_path->len - 1;
    }
    return length;
}

bool is_path_open(struct game *game, const char *path, size_t path_len)
{
    for (int i = 1; i < path_len; ++i)
    {
        struct path *precomp_path = game_get_precomputed_path(game, path[i - 1], path[i]);

        for (int j = 0; j < precomp_path->doors_len; ++j)
        {
            char key = tolower(precomp_path->doors[j]);
            if (memchr(path, key, i + 1) == NULL)
            {
                return false;
            }
        }
    }

    return true;
}

struct simulation
{
    char path[MAX_KEYS_AND_DOORS + 2]; // + '@' + '\0'
    size_t path_len;
    char keys[MAX_KEYS_AND_DOORS + 1]; // + '\0'
    size_t keys_len;
};

int find_shortest_path(struct game *game, const char *keys, size_t keys_len)
{
    struct simulation sims[MAX_GAMES] = {0};
    size_t sims_len = 0;

    struct simulation best_sim = {0};
    int best_sim_len = INT_MAX;

    sims[sims_len++] = (struct simulation){
        .path = "@",
        .path_len = 1,
        .keys = "",
        .keys_len = keys_len,
    };
    memcpy(sims[0].keys, keys, keys_len);

    int iterations = 0;

    while (sims_len)
    {
        struct simulation current = sims[--sims_len];
        char last_path_key = current.path[current.path_len - 1];

        // printf("Path: %s\n", current.path);
        // printf("Keys: %s\n", current.keys);
        // printf("---");

        // Get keys that we CAN put in the next position

        char next_keys[MAX_KEYS_AND_DOORS + 1] = {0};
        size_t next_keys_len = 0;

        for (char *key = current.keys; *key; ++key)
        {
            struct path *precomp_path = game_get_precomputed_path(game, last_path_key, *key);
            assert(precomp_path->len);

            bool open = true;
            for (size_t i = 0; i < precomp_path->doors_len; ++i)
            {
                char door_key = tolower(precomp_path->doors[i]);

                if (strchr(current.path, door_key) == NULL)
                {
                    open = false;
                    break;
                }
            }

            if (open)
            {
                assert(next_keys_len < MAX_KEYS_AND_DOORS);
                next_keys[next_keys_len++] = *key;
            }
        }

        // Finish this simulation or spawn next ones

        if (next_keys_len == 0)
        {
            int sim_len = get_path_length(game, current.path, current.path_len);
            if (sim_len < best_sim_len)
            {
                best_sim = current;
                best_sim_len = sim_len;
            }
        }
        else
        {
            for (char *key = next_keys; *key; ++key)
            {
                // Create new sim
                struct simulation next_sim = current;

                // Push key to path
                assert(next_sim.path_len < MAX_KEYS_AND_DOORS + 1);
                next_sim.path[next_sim.path_len++] = *key;

                int sim_len = get_path_length(game, next_sim.path, next_sim.path_len);
                if (sim_len < best_sim_len)
                {
                    // Pop key from keys
                    char *pop_key = strchr(next_sim.keys, *key);
                    assert(pop_key);
                    *pop_key = next_sim.keys[next_sim.keys_len - 1];
                    next_sim.keys[--next_sim.keys_len] = '\0';

                    // Push sim
                    assert(sims_len < MAX_GAMES);
                    sims[sims_len++] = next_sim;
                }
            }
        }

        //
        // BREAK UNTIL END OF BRUTE FORCE AT AROUND 2 BILLION ITERATIONS <----------------------------
        //

        if (iterations % 5000000 == 0)
        {
            printf("%d: path=%s, len=%d\n", iterations, best_sim.path, best_sim_len);
        }

        ++iterations;
    }

    printf("Done: path=%s, len=%d\n", best_sim.path, best_sim_len);

    return best_sim_len;
}

int main(void)
{
    /*

    1. Find one path that's valid
    2. Swap letters on that path if the new path would be shorter and still valid

    */

    struct game game = game_load("day18_input.txt");
    //game_print(&game);
    //game_print_all_paths(&game);
    game_print_all_paths_starting_at(&game, '@');

    // Find any path that's valid

    char path[MAX_KEYS_AND_DOORS + 2] = {0}; // keys we collected in order
    size_t path_len = 0;

    char keys[MAX_KEYS_AND_DOORS + 1] = {0}; // keys we stil need
    size_t keys_len = 0;

    path[path_len++] = '@';

    for (char key = 'a'; key <= 'z'; ++key)
    {
        if (game_get_precomputed_path(&game, '@', key)->len)
        {
            keys[keys_len++] = key;
        }
    }

    // // This path is open and has the right length, now we just have to generate it.
    // //
    // const char *test_path = "@afbjgnhdloepcikm";
    // printf("%s %d\n",
    //        is_path_open(&game, test_path, strlen(test_path)) ? "open" : "closed",
    //        get_path_length(&game, test_path, strlen(test_path)));
    // exit(0);

    int shortest_path = find_shortest_path(&game, keys, keys_len);
    printf("Shortest path: %d\n", shortest_path);
    exit(0);

    printf("Path: %s\n", path);
    printf("Keys: %s\n", keys);

    while (keys[0]) // while we still need a key
    {
        char current = path[path_len - 1];

        for (char *key = keys; key; ++key)
        {
            if (*key == current)
            {
                continue;
            }

            struct path *precomp_path = game_get_precomputed_path(&game, current, *key);

            bool open = true;
            for (int i = 0; i < precomp_path->doors_len; ++i)
            {
                char key = tolower(precomp_path->doors[i]);
                if (strchr(path, key) == NULL)
                {
                    open = false;
                    break;
                }
            }

            //printf("%c -> %c %s\n", current, *key, open ? "open" : "closed");

            if (open)
            {
                // Add the key to path

                assert(path_len < MAX_KEYS_AND_DOORS + 1);
                path[path_len++] = *key;

                // Pop the key from keys

                *key = keys[--keys_len];
                keys[keys_len] = '\0';

                break;
            }
        }
    }

    printf("---\n");
    printf("Path: %s\n", path);
    printf("Keys: %s\n", keys);

    // stuff above works

    // Swap letters on the path while it becomes shorter and still valid

    // NOTE: I may have to test out paths of equal length too

    // TODO: Check if the path given in the example solution could be computed by the system (do we find it valid), if yes, use a different permutation thing

    bool done = false;
    int best_length = get_path_length(&game, path, path_len);
    while (!done)
    {
        done = true;

        for (int i = 1; i < path_len; ++i) // the index of the character
        {
            for (int j = 1; j < path_len; ++j) // the index of place to insert the character
            {
                if (i == j)
                {
                    continue;
                }

                // Modify the path

                char new_path[MAX_KEYS_AND_DOORS + 2] = {0};
                int new_path_len = 0;
                for (int k = 0; k < path_len; ++k)
                {
                    if (i < j) // we will take this character out first
                    {
                        if (k != i)
                        {
                            new_path[new_path_len++] = path[k];
                        }
                        if (k == j)
                        {
                            new_path[new_path_len++] = path[i];
                        }
                    }
                    else
                    {
                        if (k == j)
                        {
                            new_path[new_path_len++] = path[i];
                        }
                        if (k != i)
                        {
                            new_path[new_path_len++] = path[k];
                        }
                    }
                }

                // Check if new path is open

                if (!is_path_open(&game, new_path, path_len))
                {
                    continue;
                }

                // Check if new path is better

                int new_length = get_path_length(&game, new_path, path_len);
                if (new_length < best_length)
                {
                    printf("New best path: %s (%d)\n", new_path, new_length);

                    memcpy(path, new_path, path_len);
                    best_length = new_length;
                    done = false;
                    break;
                }
            }
        }
    }

    printf("---\n");
    printf("Best path: %s\n", path);
    printf("Length: %d\n", best_length);
}
