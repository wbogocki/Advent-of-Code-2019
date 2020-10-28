/*
    ** Advent of Code 2019 **
    ** Day 18 Part 1 **
    ** Wojciech Bogócki **
    ** 21 Dec 2019 Taipei **
    ** 13 March 2020 Taipei **
*/

#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_X 128
#define MAX_Y 128
#define MAX_PATH 512

struct position
{
    int X, Y;
};

bool
FindPosition(int Width, int Height, char* Map[], char Character, struct position* Position)
{
    for(int Y = 0;
        Y < Height;
        ++Y)
    {
        for(int X = 0;
            X < Width;
            ++X)
        {
            if(Map[X][Y] == Character)
            {
                Position->X = X;
                Position->Y = Y;
                return true;
            }
        }
    }
    return false;
}

bool
FindShortestPath(int Width, int Height, char* Map[], int KeyCount, char* Keys, struct position From, struct position To, int* Length)
{
    int Distances[MAX_X][MAX_Y] = {{0}};
    struct position Path[MAX_PATH] = {0};
    int PathLength = 0;

    Path[PathLength++] = From;

    for(;;)
    {
        struct position Current = Path[PathLength - 1];

        if(Current.X > 0)
        {
            // Left
        }
        else if(Current.X < Width - 1)
        {
            // Right
        }
        else if(Current.Y > 0)
        {
            // Down
        }
        else if(Current.Y < Height - 1)
        {
            // Up
        }
        else
        {
            // Back
        }
    }

    return false;
}

int
main(void)
{
    char Map[MAX_X][MAX_Y] = {{0}};
    int Width = 0;
    int Height = 0;

    FILE* Input = fopen("day18_input.txt", "r");
    for(;;)
    {
        char Char = fgetc(Input);
        if(Char == EOF)
        {
            break;
        }
        else if(Char == '\n')
        {
            Width = 0;
            ++Height;
        }
        else
        {
            Map[Height][Width++] = Char;
        }
    }
    Width = strlen(Map[0]);

    for(int Y = 0;
        Y < Height;
        ++Y)
    {
        for(int X = 0;
            X < Width;
            ++X)
        {
            putchar(Map[X][Y]);
        }
        putchar('\n');
    }
    fflush(stdout);

    // shortest distance from each key to each other key
    // doors needed before a key is unlocked
    // check if there is more passageways that omit some doors?
}