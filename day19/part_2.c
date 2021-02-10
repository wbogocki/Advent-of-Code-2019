/**
 * Challenge:   Advent of Code 2019
 * Day:         19
 * Part:        2
 * Author:      Wojciech Bogócki
 * Date:        10 Feb 2021 Taipei
 **/

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Computer

#define MEMORY_SIZE 1024

typedef long long icv;

enum opcodes
{
    OP_ADD = 1,
    OP_MUL = 2,
    OP_IN = 3,
    OP_OUT = 4,
    OP_JT = 5,
    OP_JF = 6,
    OP_TLT = 7,
    OP_TEQ = 8,
    OP_RBO = 9,
    OP_HLT = 99
};

const char *OPCODES[] =
    {
        [OP_ADD] = "ADD",
        [OP_MUL] = "MUL",
        [OP_IN] = "IN",
        [OP_OUT] = "OUT",
        [OP_JT] = "JT",
        [OP_JF] = "JF",
        [OP_TLT] = "TLT",
        [OP_TEQ] = "TEQ",
        [OP_RBO] = "RBO",
        [OP_HLT] = "HLT",
};

enum pmodes
{
    PMOD_POS = 0,
    PMOD_IMM = 1,
    PMOD_REL = 2
};

enum interrupt
{
    INT_HLT = 0,
    INT_IN = 1,
    INT_OUT = 2
};

enum flags
{
    F_HLT = 1,
    F_IN = 2
};

struct computer
{
    icv Memory[MEMORY_SIZE];
    int IP;
    int Rbase;
    int Flags;
    icv In;
    icv Out;
};

icv Pget(int Pmodes, int Pord, icv P, int Rbase, icv Memory[])
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord] % 10;
    icv Result;
    switch (Pmode)
    {
    case PMOD_POS:
    {
        int Address = P;
        if (Address < 0 || Address >= MEMORY_SIZE)
        {
            printf("Bad address: %d\n", Address);
            exit(1);
        }
        Result = Memory[Address];
        break;
    }
    case PMOD_IMM:
    {
        Result = P;
        break;
    }
    case PMOD_REL:
    {
        int Address = Rbase + P;
        if (Address < 0 || Address >= MEMORY_SIZE)
        {
            printf("Bad address: %d\n", Address);
            exit(1);
        }
        Result = Memory[Address];
        break;
    }
    default:
    {
        printf("Bad parameter mode: %d\n", Pmode);
        exit(1);
    }
    }
    return Result;
}

void Pset(int Pmodes, int Pord, icv P, int Rbase, icv Memory[], icv Value)
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord] % 10;
    switch (Pmode)
    {
    case PMOD_POS:
    {
        int Address = P;
        if (Address < 0 || Address >= MEMORY_SIZE)
        {
            printf("Bad address: %d\n", Address);
            exit(1);
        }
        Memory[Address] = Value;
        break;
    }
    case PMOD_REL:
    {
        int Address = Rbase + P;
        if (Address < 0 || Address >= MEMORY_SIZE)
        {
            printf("Bad address: %d\n", Address);
            exit(1);
        }
        Memory[Address] = Value;
        break;
    }
    default:
    {
        printf("Bad parameter mode: %d\n", Pmode);
        exit(1);
    }
    }
}

#define Load(void) Pget(Pmodes, Pord++, Computer->Memory[Computer->IP++], Computer->Rbase, Computer->Memory)
#define Store(Value) Pset(Pmodes, Pord++, Computer->Memory[Computer->IP++], Computer->Rbase, Computer->Memory, (Value))
#define Set(Flag) (Computer->Flags |= (Flag))
#define Unset(Flag) (Computer->Flags &= ~(Flag))
#define Test(Flag) ((Computer->Flags & (Flag)) == (Flag))

enum interrupt
Run(struct computer *Computer)
{
    //printf("%-4s %-3s %-3s %-4s\n", "IP", "OP", "PM", "RB");

    if (Test(F_HLT))
    {
        return INT_HLT;
    }

    for (;;)
    {
        int I = Computer->IP++;
        int Opcode = Computer->Memory[I] % 100;
        int Pmodes = Computer->Memory[I] / 100;
        int Pord = 0;

        //printf("%04d %-3s %03d %04d\n", Computer->IP, OPCODES[Opcode], Pmodes, Computer->Rbase);

        switch (Opcode)
        {
        case OP_ADD:
        {
            icv P1 = Load();
            icv P2 = Load();
            Store(P1 + P2);
            break;
        }
        case OP_MUL:
        {
            icv P1 = Load();
            icv P2 = Load();
            Store(P1 * P2);
            break;
        }
        case OP_IN:
        {
            if (Test(F_IN))
            {
                Store(Computer->In);
                Unset(F_IN);
            }
            else
            {
                --Computer->IP;
                Set(F_IN);
                return INT_IN;
            }
            break;
        }
        case OP_OUT:
        {
            Computer->Out = Load();
            return INT_OUT;
        }
        case OP_JT:
        {
            icv P1 = Load();
            icv P2 = Load();
            if (P1 != 0)
            {
                Computer->IP = P2;
            }
            break;
        }
        case OP_JF:
        {
            icv P1 = Load();
            icv P2 = Load();
            if (P1 == 0)
            {
                Computer->IP = P2;
            }
            break;
        }
        case OP_TLT:
        {
            icv P1 = Load();
            icv P2 = Load();
            Store(P1 < P2);
            break;
        }
        case OP_TEQ:
        {
            icv P1 = Load();
            icv P2 = Load();
            Store(P1 == P2);
            break;
        }
        case OP_RBO:
        {
            Computer->Rbase += Load();
            break;
        }
        case OP_HLT:
        {
            Set(F_HLT);
            return INT_HLT;
        }
        default:
        {
            printf("Bad opcode: %d\n", Opcode);
            exit(1);
        }
        }
    }
}

void LoadMemory(const char *Path, icv Memory[MEMORY_SIZE])
{
    FILE *File = fopen(Path, "r");
    if (!File)
    {
        puts("No program file\n");
        exit(1);
    }
    int Size = 0;
    while (fscanf(File, "%lld,", &Memory[Size]) != EOF)
    {
        Size++;
    }
    fclose(File);
}

// Tractor Beam

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
    struct computer Computer = {0};
    memcpy(Computer.Memory, Program, sizeof(icv) * MEMORY_SIZE);

    int Inputs[] = {X, Y};
    int InputsLength = 2;

    int Output = -1;

    bool Done = false;
    while (!Done)
    {
        enum interrupt Interrupt = Run(&Computer);
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