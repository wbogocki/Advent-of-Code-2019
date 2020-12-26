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

#define MAX_X 256
#define MAX_Y 256

struct tunnels
{
    char terrain[MAX_X][MAX_Y];
    size_t width;
    size_t height;
};

struct tunnels load_tunnels(const char *path)
{
    struct tunnels tunnels = {0};

    FILE *file = fopen(path, "r");

    size_t x = 0;
    size_t y = 0;
    for (char tile = fgetc(file);
         tile != EOF;
         tile = fgetc(file))
    {
        if (tile == '\n')
        {
            assert(tunnels.width == 0 || tunnels.width == x + 1); // Rows are same width
            tunnels.width = x + 1;
            x = 0;
            ++y;
        }
        else
        {
            assert(y < MAX_Y && x < MAX_X);
            tunnels.terrain[y][x++] = tile;
        }
    }
    tunnels.height = y + 1; // This will break if input has an empty line at the end

    fclose(file);

    return tunnels;
}

void print_tunnels(struct tunnels *tunnels)
{
    for (size_t y = 0; y < tunnels->height; ++y)
    {
        for (size_t x = 0; x < tunnels->width; ++x)
        {
            putchar(tunnels->terrain[y][x]);
        }
        putchar('\n');
    }
    fflush(stdout);
}

//
// Game
//

struct position
{
    int x, y;
};

#define MAX_KEYS_AND_DOORS 256
#define MAX_PLAYER_KEYS 26

struct game
{
    struct tunnels tunnels;
    struct position player_position;
    struct position keys_positions[MAX_KEYS_AND_DOORS];
    struct position door_positions[MAX_KEYS_AND_DOORS];
    char player_keys[MAX_PLAYER_KEYS];
    int player_keys_length;

    int path_length;
    bool completed;
};

struct position locate_object_in_tunnels(struct tunnels *tunnels, char thing)
{
    for (size_t y = 0; y < tunnels->height; ++y)
    {
        for (size_t x = 0; x < tunnels->width; ++x)
        {
            if (tunnels->terrain[y][x] == thing)
            {
                return (struct position){.x = x, .y = y};
            }
        }
    }
    return (struct position){.x = -1, .y = -1};
}

bool check_has_key(struct game *game, char key)
{
    for (int index = 0; index < game->player_keys_length; ++index)
    {
        if (game->player_keys[index] == key)
        {
            return true;
        }
    }
    return false;
}

void collect_key(struct game *game, char key)
{
    assert(game->player_keys_length != MAX_PLAYER_KEYS);
    game->player_keys[game->player_keys_length++] = key;

    // game->tunnels.terrain[game->player_position.y][game->player_position.x] = '.';
    // game->player_position = game->keys_positions[key];
    // game->tunnels.terrain[game->player_position.y][game->player_position.x] = '@';
}

//
// Calculate move cost
//

#define MAX_TILES_ALREADY_CHECKED 8192
#define MAX_TILES_TO_CHECK 8192

struct tile_to_check
{
    struct position position;
    int distance_to_target;
    int move_cost;
};

struct check_result
{
    bool traversable;
    struct position neighbors[4];
};

int calculate_distance(struct position a, struct position b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
}

struct check_result check_tile(struct game *game, struct tile_to_check *tile)
{
    struct check_result check_result = {
        .traversable = true,
        .neighbors = {0},
    };

    // Traversability

    char tile_terrain = game->tunnels.terrain[tile->position.y][tile->position.x];

    if (tile->position.x < 0 || tile->position.x >= game->tunnels.width)
    {
        check_result.traversable = false;
    }
    else if (tile->position.y < 0 || tile->position.y >= game->tunnels.height)
    {
        check_result.traversable = false;
    }
    else if (tile_terrain == '#')
    {
        check_result.traversable = false;
    }
    else if (tile_terrain >= 'A' && tile_terrain <= 'Z')
    {
        // Open doors with the key

        if (!check_has_key(game, tolower(tile_terrain)))
        {
            check_result.traversable = false;
        }
    }

    // Neighbors

    struct position neighbor_offsets[4] = {
        [0] = (struct position){.x = 0, .y = -1}, // Top
        [1] = (struct position){.x = 1, .y = 0},  // Right
        [2] = (struct position){.x = 0, .y = 1},  // Bottom
        [3] = (struct position){.x = -1, .y = 0}, // Left
    };

    for (size_t neighbor_idx = 0;
         neighbor_idx < 4;
         ++neighbor_idx)
    {
        check_result.neighbors[neighbor_idx] = (struct position){
            .x = tile->position.x + neighbor_offsets[neighbor_idx].x,
            .y = tile->position.y + neighbor_offsets[neighbor_idx].y,
        };
    }

    return check_result;
}

int calculate_move_cost(struct game *game, struct position target)
{
    // Initialize the distances array for searching

    int distances[MAX_Y][MAX_X] = {0};

    // Give a numerical value to every tile the player can reach

    struct tile_to_check tiles_already_checked[MAX_TILES_ALREADY_CHECKED] = {0};
    size_t tiles_already_checked_length = 0;

    struct tile_to_check tiles_to_check[MAX_TILES_TO_CHECK] = {
        [0] = (struct tile_to_check){
            .position = game->player_position,
            .distance_to_target = calculate_distance(game->player_position, target),
            .move_cost = 0,
        },
    };
    size_t tiles_to_check_length = 1;

    int move_cost = INT_MAX; // Do not search beyond this move cost

    while (tiles_to_check_length >= 1)
    {
        // Order of business:
        // 1. Check the tile
        // 2. Push eligible neighboring tiles to the tiles-to-check stack

        // Take a tile off of the tiles-to-check stack

        struct tile_to_check tile = tiles_to_check[--tiles_to_check_length];

        // Push the tile to the tiles already checked

        assert(tiles_already_checked_length != MAX_TILES_ALREADY_CHECKED);
        tiles_already_checked[tiles_already_checked_length++] = tile;

        // Check the tile

        struct check_result check_result = check_tile(game, &tile);

        if (check_result.traversable)
        {
            // Check if this is the target tile and set the move cost accordingly

            if (tile.distance_to_target == 0 && tile.move_cost < move_cost)
            {
                move_cost = tile.move_cost;
            }

            // Push the neighbors to tiles-to-check

            for (size_t neighbor_idx = 0;
                 neighbor_idx < 4;
                 ++neighbor_idx)
            {
                struct position neighbor = check_result.neighbors[neighbor_idx];

                int neighbor_distance_to_target = calculate_distance(neighbor, target);

                // Check if the tile needs to be checked for the first time

                bool first_check = true;
                for (size_t index = 0; index < tiles_already_checked_length; ++index)
                {
                    struct tile_to_check checked_tile = tiles_already_checked[index];
                    if (checked_tile.position.x == neighbor.x && checked_tile.position.y == neighbor.y)
                    {
                        first_check = false;
                        if (checked_tile.distance_to_target < neighbor_distance_to_target)
                        {
                            printf("NOT IMPLEMENTED: Tile has been revisited at a closer distance!");
                            exit(1);
                        }
                        break;
                    }
                }

                // Add tile to the list of already checked tiles

                if (first_check)
                {
                    assert(tiles_to_check_length != MAX_TILES_TO_CHECK);
                    tiles_to_check[tiles_to_check_length++] = (struct tile_to_check){
                        .position = neighbor,
                        .distance_to_target = neighbor_distance_to_target,
                        .move_cost = tile.move_cost + 1,
                    };
                }
            }
        }
    }

    return move_cost;
}

//
// Calculate shortest path
//

#define MAX_PATH_LENGTHS 256
#define MAX_POSSIBLE_MOVES 26

struct possible_moves
{
    char next_keys[MAX_POSSIBLE_MOVES];
    size_t next_keys_length;
    int move_costs[MAX_KEYS_AND_DOORS];
};

struct possible_moves calculate_possible_moves(struct game *game)
{
    struct possible_moves possible_moves = {0};
    for (char key = 'a'; key <= 'z'; ++key)
    {
        if (check_has_key(game, key))
        {
            continue;
        }

        struct position position = game->keys_positions[key];
        if (position.x == -1)
        {
            continue;
        }

        int move_cost = calculate_move_cost(game, position);
        if (move_cost == INT_MAX)
        {
            continue;
        }

        assert(possible_moves.next_keys_length <= MAX_POSSIBLE_MOVES);
        possible_moves.next_keys[possible_moves.next_keys_length++] = key;
        possible_moves.move_costs[key] = move_cost;
    }
    return possible_moves;
}

#define MAX_GAMES (8192 * 16)

int follow_shortest_path(struct game *root_game)
{
    struct game *games = malloc(sizeof(struct game) * MAX_GAMES);
    games[0] = *root_game;

    size_t games_length = 1;

    bool all_games_done = false;
    while (!all_games_done)
    {
        printf("computing %d games\n", games_length);

        all_games_done = true;
        for (size_t game_index = 0;
             game_index < games_length;
             ++game_index)
        {
            struct game *game = &games[game_index];
            if (game->completed)
            {
                continue;
            }

            struct possible_moves possible_moves = calculate_possible_moves(game);
            if (possible_moves.next_keys_length == 0)
            {
                game->completed = true;
                continue;
            }

            all_games_done = false;

            // printf("possible moves %d\n", possible_moves.next_keys_length);

            // Do all moves other than the first one of new games

            for (size_t key_index = 1;
                 key_index < possible_moves.next_keys_length;
                 ++key_index)
            {
                struct game derived_game = *game;

                char next_key = possible_moves.next_keys[key_index];
                collect_key(&derived_game, next_key);
                derived_game.player_position = derived_game.keys_positions[next_key]; // Teleport player to the key
                derived_game.path_length += possible_moves.move_costs[next_key];

                assert(games_length < MAX_GAMES);
                games[games_length++] = derived_game;
            }

            // Do the first possible move on the game object itself

            char next_key = possible_moves.next_keys[0];
            collect_key(game, next_key);
            game->player_position = game->keys_positions[next_key]; // Teleport player to the key
            game->path_length += possible_moves.move_costs[next_key];
        }
    }

    int shortest_path = INT_MAX;
    for (size_t game_index = 0;
         game_index < games_length;
         ++game_index)
    {
        struct game *game = &games[game_index];

        printf("path length %d\n", game->path_length);

        if (game->path_length < shortest_path)
        {
            shortest_path = game->path_length;
        }
    }

    return shortest_path;
}

int main(void)
{
    struct tunnels tunnels = load_tunnels("day18_input.txt");
    struct game game = {
        .tunnels = tunnels,
        .player_position = locate_object_in_tunnels(&tunnels, '@'),
        .player_keys = {0},
        .player_keys_length = 0,
        .path_length = 0,
        .completed = false,
    };
    for (char key = 'a'; key <= 'z'; ++key)
    {
        struct position position = locate_object_in_tunnels(&tunnels, key);
        game.keys_positions[key] = position;
    }
    for (char door = 'A'; door <= 'Z'; ++door)
    {
        struct position position = locate_object_in_tunnels(&tunnels, door);
        game.door_positions[door] = position;
    }

    print_tunnels(&tunnels);

    // collect_key(&game, 'a');
    // collect_key(&game, 'b');
    // collect_key(&game, 'c');

    // int move_cost = calculate_move_cost(&game, game.keys_positions['d']);
    // printf("%d", move_cost);

    int shortest_path = follow_shortest_path(&game);

    printf("Shortest path: %d\n", shortest_path);
}