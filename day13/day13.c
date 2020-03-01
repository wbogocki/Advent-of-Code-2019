/*
    ** Advent of Code 2019 **
    ** Day 13 Part 1 **
    ** Wojciech Bogócki **
    ** 14 Dec 2019 Taipei **
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE (16*1024)
#define OPCODE_DEBUG 0
#define SCREEN_WIDTH 48
#define SCREEN_HEIGHT 48

typedef long long icv;

enum opcodes
{
    OP_ADD = 1,
    OP_MUL = 2,
    OP_IN  = 3,
    OP_OUT = 4,
    OP_JT  = 5,
    OP_JF  = 6,
    OP_TLT = 7,
    OP_TEQ = 8,
    OP_RBO = 9,
    OP_HLT = 99
};

const char* OPCODES[] =
{
    [OP_ADD] = "ADD",
    [OP_MUL] = "MUL",
    [OP_IN]  = "IN",
    [OP_OUT] = "OUT",
    [OP_JT]  = "JT",
    [OP_JF]  = "JF",
    [OP_TLT] = "TLT",
    [OP_TEQ] = "TEQ",
    [OP_RBO] = "RBO",
    [OP_HLT] = "HLT"
};

enum pmodes
{
    M_POS = 0,
    M_IMM = 1,
    M_REL = 2
};

icv
Pget(int Pmodes, int Pord, icv P, int Rbase, icv Memory[])
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord] % 10;
    icv Result;
    switch(Pmode)
    {
        case M_POS:
        {
            int Address = P;
            if(Address < 0 || Address >= MEMORY_SIZE)
            {
                printf("Bad address: %d\n", Address);
                exit(1);
            }
            Result = Memory[Address];
            break;
        }
        case M_IMM:
        {
            Result = P;
            break;
        }
        case M_REL:
        {
            int Address = Rbase + P;
            if(Address < 0 || Address >= MEMORY_SIZE)
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

void
Pset(int Pmodes, int Pord, icv P, int Rbase, icv Memory[], icv Value)
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord] % 10;
    switch(Pmode)
    {
        case M_POS:
        {
            int Address = P;
            if(Address < 0 || Address >= MEMORY_SIZE)
            {
                printf("Bad address: %d\n", Address);
                exit(1);
            }
            Memory[Address] = Value;
            break;
        }
        case M_REL:
        {
            int Address = Rbase + P;
            if(Address < 0 || Address >= MEMORY_SIZE)
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

#define Load(void) Pget(Pmodes, Pord++, Memory[IP++], Rbase, Memory)
#define Store(Value) Pset(Pmodes, Pord++, Memory[IP++], Rbase, Memory, (Value));

enum mode
{
    MOD_X,
    MOD_Y,
    MOD_ID
};

enum tile
{
    TIL_EMPT = 0,
    TIL_WALL = 1,
    TIL_BLCK = 2,
    TIL_HPAD = 3,
    TIL_BALL = 4
};

int
main(void)
{
    icv* Memory = malloc(sizeof(icv) * MEMORY_SIZE);
    memset(Memory, 0, sizeof(icv) * MEMORY_SIZE);

    FILE* Input = fopen("day13_input.txt", "r");
    if(!Input)
    {
        puts("No program file\n");
        return 1;
    }
    int Size = 0;
    while(fscanf(Input, "%lld,", &Memory[Size]) != EOF)
    {
        Size++;
    }

    puts("Program loaded");

    enum tile Tiles[SCREEN_WIDTH][SCREEN_HEIGHT] = {0};
    enum mode Mode = MOD_X;
    int R_X = 0;
    int R_Y = 0;
    int R_ID = 0;

#if OPCODE_DEBUG
    printf("%-4s %-3s %-3s %-4s\n", "IP", "OP", "PM", "RB");
#endif

    int IP = 0;
    int Rbase = 0;
    for(;;)
    {
        // Screen

        for(int Y = 0; Y < SCREEN_HEIGHT; ++Y)
        {
            for(int X = 0; X < SCREEN_WIDTH; ++X)
            {
                switch(Tiles[X][Y])
                {
                    case TIL_EMPT: putchar(' '); break;
                    case TIL_WALL: putchar('|'); break;
                    case TIL_BLCK: putchar('#'); break;
                    case TIL_HPAD: putchar('-'); break;
                    case TIL_BALL: putchar('O'); break;
                }
            }
            putchar('\n');
        }

        // Computer

        int I = IP++;
        int Opcode = Memory[I] % 100;
        int Pmodes = Memory[I] / 100;
        int Pord = 0;

#if OPCODE_DEBUG
        printf("%04d %-3s %03d %04d\n", IP, OPCODES[Opcode], Pmodes, Rbase);
#endif

        switch(Opcode)
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
                printf("Input: ");
                icv Input;
                scanf("%lld", &Input);
                Store(Input);
                break;
            }
            case OP_OUT:
            {
                if(Mode == MOD_X)
                {
                    R_X = Load();
                    Mode = MOD_Y;
                }
                else if(Mode == MOD_Y)
                {
                    R_Y = Load();
                    Mode = MOD_ID;
                }
                else if(Mode == MOD_ID)
                {
                    R_ID = Load();
                    Tiles[R_X][R_Y] = R_ID;
                    Mode = MOD_X;
                }
                else
                {
                    printf("Bad mode: %d\n", Mode);
                    return 1;
                }
                break;
            }
            case OP_JT:
            {
                icv P1 = Load();
                icv P2 = Load();
                if(P1 != 0)
                {
                    IP = P2;
                }
                break;
            }
            case OP_JF:
            {
                icv P1 = Load();
                icv P2 = Load();
                if(P1 == 0)
                {
                    IP = P2;
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
                Rbase += Load();
                break;
            }
            case OP_HLT:
            {
                printf("Halted\n");
                int BlockCount = 0;
                for(int X = 0; X < SCREEN_WIDTH; ++X)
                {
                    for(int Y = 0; Y < SCREEN_HEIGHT; ++Y)
                    {
                        if(Tiles[X][Y] == TIL_BLCK)
                        {
                            ++BlockCount;
                        }
                    }
                }
                printf("Result: %d\n", BlockCount);
                return 0;
            }
            default:
            {
                printf("Bad opcode: %d\n", Opcode);
                return 1;
            }
        }
    }
}
