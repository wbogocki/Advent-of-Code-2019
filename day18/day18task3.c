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
#define MAX_GAMES 8192

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

struct game find_shortest_path_pop_best_game(struct game *games, size_t *games_len)
{
    int best_idx = 0, best_paths_len = 0;
    for (int i = 0; i < *games_len; ++i)
    {
        if (best_paths_len < games[i].paths_len)
        {
            best_idx = i;
            best_paths_len = games[i].paths_len;
        }
    }
    struct game best_game = games[best_idx];
    games[best_idx] = games[--(*games_len)];
    return best_game;
}

struct game find_shortest_path_to_collect_all_keys(struct game *game)
{
    struct game *games = calloc(MAX_GAMES, sizeof(struct game));
    size_t games_len = 0;

    struct game *done_games = calloc(MAX_GAMES, sizeof(struct game));
    size_t done_games_len = 0;

    int cutoff_distance = INT_MAX; // Do not compute paths longer than this
    int current_best_distance = INT_MAX;
    int iter_count = 0;

    // Initialize

    games[games_len++] = *game;

    // Simulate all games

    while (games_len > 0)
    {
        // Pop a game from the games stack

        struct game current = find_shortest_path_pop_best_game(games, &games_len);
        //struct game current = games[--games_len];

        char current_tile = game_tile(&current, current.player_pos);

        //printf("player %d,%d (%c)\n", current.player_pos.x, current.player_pos.y, current_tile);

        assert(current_tile != '#' && current_tile != '.');

        // Find all possible paths at this point

        struct path *available_paths[MAX_KEYS_AND_DOORS] = {0};
        size_t available_paths_len = 0;

        bool cutoff_game = false; // The game will not be simulated if this is true

        for (char key = 'a'; key <= 'z'; ++key)
        {
            //printf("key %c\n", key);
            if (!current.player_keys[letter_idx(key)])
            {
                // We don't own this key yet, look for a path to get it

                struct path *path = game_get_precomputed_path(&current, current_tile, key);

                if (path->len > 0)
                {
                    // Check if the game has a chance of being the best and is under the cutoff distance

                    int distance_after_path = current.distance + path->len - 1;

                    if (distance_after_path < current_best_distance && distance_after_path < cutoff_distance)
                    {
                        // Check if we can take this path at this time by opening all the doors

                        bool blocked = false;
                        for (size_t i = 0; i < path->doors_len; ++i)
                        {
                            char door = path->doors[i];
                            if (!current.player_keys[letter_idx(door)])
                            {
                                blocked = true;
                                break;
                            }
                        }

                        //printf("%c %s\n", key, blocked ? "blocked" : "open");

                        if (!blocked)
                        {
                            assert(available_paths_len < MAX_KEYS_AND_DOORS);
                            available_paths[available_paths_len++] = path;
                        }
                    }
                    else
                    {
                        // printf("Rejected a bad path with distance %d compared to current best %d\n",
                        //        distance_after_path,
                        //        current_best_distance);

                        cutoff_game = true;
                    }
                }
            }
        }

        if (!cutoff_game)
        {
            //printf("available_paths_len %d\n", available_paths_len);

            if (available_paths_len > 0)
            {
                // Push games after taking each path onto the games stack

                for (size_t i = 0; i < available_paths_len; ++i)
                {
                    struct path *path = available_paths[i];

                    assert(path->len > 0);

                    struct position path_end = path->pos[path->len - 1];
                    char key = game_tile(&current, path_end);

                    struct game game = current;
                    game.player_pos = path_end;
                    game.player_keys[letter_idx(key)] = true;
                    assert(game.paths_len < MAX_KEYS_AND_DOORS);
                    game.paths[game.paths_len++] = path;
                    game.distance += path->len - 1;

                    assert(games_len < MAX_GAMES);
                    games[games_len++] = game;
                }
            }
            else
            {
                assert(done_games_len < MAX_GAMES);
                done_games[done_games_len++] = current;

                if (current.distance < current_best_distance)
                {
                    current_best_distance = current.distance;
                }
            }
        }

        if (iter_count % 1000000 == 0)
        {
            printf("%d: %zu games remaining, %zu games finished (current best: %d)\n",
                   iter_count, games_len, done_games_len, current_best_distance);
        }

        ++iter_count;
    }

    // Get the shortest path

    struct game *best_game = &done_games[0];

    for (size_t i = 0; i < done_games_len; ++i)
    {
        if (done_games[i].distance < best_game->distance)
        {
            best_game = &done_games[i];
        }
    }

    struct game best_game_copy = *best_game;

    free(done_games);
    free(games);

    return best_game_copy;
}

// Take 3

struct destinations
{
    char tiles[MAX_KEYS_AND_DOORS];
    size_t tiles_len;
};

struct destinations find_possible_destinations(struct game *game)
{
    struct destinations destinations = {0};

    char current_tile = game_tile(game, game->player_pos);

    for (char key = 'a'; key <= 'z'; ++key)
    {
        struct path *path = game_get_precomputed_path(game, current_tile, key);
        if (path->len)
        {
            bool blocked = false;
            for (int i = 0; i < path->doors_len; ++i)
            {
                char door = path->doors[i];
                //printf("door %c: ", door);
                if (!game->player_keys[letter_idx(door)])
                {
                    //printf("blocked\n");
                    blocked = true;
                    break;
                }
                //printf("open\n");
            }

            //printf("key %c: %s\n", key, blocked ? "blocked" : "open");

            if (!blocked)
            {
                assert(destinations.tiles_len < MAX_KEYS_AND_DOORS);
                destinations.tiles[destinations.tiles_len++] = game_tile(game, path->pos[path->len - 1]);
            }
        }
    }

    return destinations;
}

// struct solution
// {
//     char path[MAX_KEYS_AND_DOORS]; // keys that were already collected
//     size_t path_len;

//     char keys[MAX_KEYS_AND_DOORS]; // keys that still need to be picked up
//     size_t keys_len;
// };

// void find_possible_paths_through_game(struct game *game)
// {
//     char keys[MAX_KEYS_AND_DOORS] = {0};
//     int keys_len = 0;

//     struct solution sols[MAX_GAMES] = {0};
//     size_t sols_len = 0;

//     struct solution complete_sols[MAX_GAMES] = {0};
//     size_t complete_sols_len = 0;

//     // Initialize

//     for (char key = 'a'; key <= 'z'; ++key)
//     {
//         if (game->fixed->keys_pos[letter_idx(key)].x > 0)
//         {
//             keys[keys_len++] = key;
//         }
//     }

//     struct solution initial = {
//         .path = {'@'},
//         .path_len = 1,
//         .keys = *keys,
//         .keys_len = keys_len,
//     };

//     sols[sols_len++] = initial;

//     // Find solutions

//     while (sols_len)
//     {
//         struct solution current = sols[--sols_len];

//         char tile = current.path[current.path_len - 1];

//         // Find paths that we can take

//         char good_paths[MAX_KEYS_AND_DOORS] = {0};
//         size_t good_paths_len = 0;

//         for (size_t i = 0; i < current.keys_len; ++i)
//         {
//             struct path *path = game_get_precomputed_path(game, tile, current.keys[i]);
//             assert(path);

//             bool path_is_good = true; // let's just assume each path is good

//             if (path_is_good)
//             {
//                 assert(good_paths_len < MAX_KEYS_AND_DOORS);
//                 good_paths[good_paths_len++] = current.keys[i];
//             }
//         }

//         // Save good paths

//         for (size_t i = 0; i < good_paths_len; ++i)
//         {
//             // 15min break <-----------------------------------------------------
//         }
//     }
// }

int main(void)
{
    struct game game = game_load("day18_input.txt");
    //game_print(&game);
    game_print_all_paths(&game);

    // struct destinations destinations = find_possible_destinations(&game);

    // for (char i = 0; i < destinations.tiles_len; ++i)
    // {
    //     printf("%c\n", destinations.tiles[i]);
    // }

    // struct game best_game = find_shortest_path_to_collect_all_keys(&game);
    // game_print_taken_paths(&best_game);
    // printf("Shortest path: %d\n", best_game.distance);
}