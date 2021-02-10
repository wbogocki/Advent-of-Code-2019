/**
 * Challenge:   Advent of Code 2019
 * Day:         19
 * Part:        2
 * Author:      Wojciech Bogócki
 * Date:        10 Feb 2021 Taipei
 **/

#define INTCODE_IMPL

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intcode.h"

#define EXAMPLE 0

int CheckTractorBeam(icv Program[MEMORY_SIZE], int X, int Y)
{
#if EXAMPLE
    char ExampleBeam[40][40] = {
        "#.......................................",
        ".#......................................",
        "..##....................................",
        "...###..................................",
        "....###.................................",
        ".....####...............................",
        "......#####.............................",
        "......######............................",
        ".......#######..........................",
        "........########........................",
        ".........#########......................",
        "..........#########.....................",
        "...........##########...................",
        "...........############.................",
        "............############................",
        ".............#############..............",
        "..............##############............",
        "...............###############..........",
        "................###############.........",
        "................#################.......",
        ".................########OOOOOOOOOO.....",
        "..................#######OOOOOOOOOO#....",
        "...................######OOOOOOOOOO###..",
        "....................#####OOOOOOOOOO#####",
        ".....................####OOOOOOOOOO#####",
        ".....................####OOOOOOOOOO#####",
        "......................###OOOOOOOOOO#####",
        ".......................##OOOOOOOOOO#####",
        "........................#OOOOOOOOOO#####",
        ".........................OOOOOOOOOO#####",
        "..........................##############",
        "..........................##############",
        "...........................#############",
        "............................############",
        ".............................###########",
    };

    if (X < 40 && Y < 40 && (ExampleBeam[Y][X] == '#' || ExampleBeam[Y][X] == 'O'))
    {
        return 1;
    }
    else
    {
        return 0;
    }
#else
    computer Computer = {0};
    memcpy(Computer.Memory, Program, sizeof(icv) * MEMORY_SIZE);

    int Inputs[] = {X, Y};
    int InputsLength = 2;

    int Output = -1;

    bool Done = false;
    while (!Done)
    {
        interrupt Interrupt = Run(&Computer);
        switch (Interrupt)
        {
        case INT_HLT:
            Done = true;
            break;
        case INT_IN:
            assert(InputsLength > 0);
            assert(Inputs[InputsLength - 1] >= 0);
            Computer.In = Inputs[--InputsLength];
            break;
        case INT_OUT:
            Output = Computer.Out;
            break;
        default:
            printf("Bad interrupt: %d\n", Interrupt);
            exit(1);
        }
    }

    return Output;
#endif
}

int main(void)
{
    icv Program[MEMORY_SIZE] = {0};
    LoadMemory("input.txt", Program);

    int ShipSize = 100;

    int X = 0;
    int Y = ShipSize;

    // Catch the beam
    while (!CheckTractorBeam(Program, X, Y))
    {
        ++X;
    }

    // Find the square that fits the ship
    while (!CheckTractorBeam(Program, X + (ShipSize - 1), Y - (ShipSize - 1)))
    {
        ++Y;
        while (!CheckTractorBeam(Program, X, Y))
        {
            ++X;
        }

        // printf("%d,%d\n", X, Y);
    }

    // Top-left corner of the square
    Y -= (ShipSize - 1);

    // Find the point closest to the emitter
    int OutDist = INT_MAX;
    int OutX = 0;
    int OutY = 0;
    for (int x = X; x < X + ShipSize; ++x)
    {
        for (int y = Y; y < Y + ShipSize; ++y)
        {
            int Dist = x * x + y * y;
            if (Dist < OutDist)
            {
                OutDist = Dist;
                OutX = x;
                OutY = y;
            }
        }
    }

    // Print
    for (int y = Y - 5; y < Y + ShipSize + 5; ++y)
    {
        for (int x = X - 5; x < X + ShipSize + 5; ++x)
        {
            if (CheckTractorBeam(Program, x, y))
            {
                if (x == OutX && y == OutY)
                {
                    putchar('X');
                }
                else if (x >= X && x < X + ShipSize &&
                         y >= Y && y < Y + ShipSize)
                {
                    putchar('O');
                }
                else
                {
                    putchar('#');
                }
            }
            else
            {
                putchar('.');
            }
        }
        putchar('\n');
    }

    printf("Output: %d (%d,%d)\n", OutX * 10000 + OutY, OutX, OutY);
}
