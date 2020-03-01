/*
    ** Advent of Code 2019 **
    ** Day 11 Part 1 & 2 **
    ** Wojciech Bogócki **
    ** 12 Dec 2019 Taipei **
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIGINT_IMPL
#include "bigint.h"

#define MEMORY_SIZE (16*1024)
#define OPCODE_DEBUG 0

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

bigint
Pget(int Pmodes, int Pord, bigint P, int Rbase, bigint Memory[])
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord] % 10;
    bigint Result;
    switch(Pmode)
    {
        case M_POS:
        {
            int Address = BigIntValue(P);
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
            int Address = Rbase + BigIntValue(P);
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
Pset(int Pmodes, int Pord, bigint P, int Rbase, bigint Memory[], bigint Value)
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord] % 10;
    switch(Pmode)
    {
        case M_POS:
        {
            int Address = BigIntValue(P);
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
            int Address = Rbase + BigIntValue(P);
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

struct pos
{
    int X,Y;
};

enum col
{
    COL_BLACK = 0,
    COL_WHITE = 1
};

enum dir
{
    DIR_LEFT  = 0,
    DIR_UP    = 1,
    DIR_RIGHT = 2,
    DIR_DOWN  = 3
};

enum mod
{
    MOD_COL,
    MOD_MOV
};

struct panel
{
    struct pos Pos;
    enum col Col;
    int PaintCount;
};

enum dir
Rotate(enum dir Direction, bool Right)
{
    enum dir New = Direction + (Right ? 1 : -1);
    New = (New + 4) % 4;
    return New;
}

struct pos
Advance(struct pos Position, enum dir Direction)
{
    struct pos New = Position;
    switch(Direction)
    {
        case DIR_LEFT:  --New.X; break;
        case DIR_UP:    --New.Y; break;
        case DIR_RIGHT: ++New.X; break;
        case DIR_DOWN:  ++New.Y; break;
    }
    return New;
}

int
GetPanel(int Count, struct panel Panels[], struct pos Position)
{
    for(int Index = 0;
        Index < Count;
        ++Index)
    {
        if( Panels[Index].Pos.X == Position.X &&
            Panels[Index].Pos.Y == Position.Y)
        {
            return Index;
        }
    }
    return -1;
}

int
main(void)
{
    bigint* Memory = malloc(sizeof(bigint) * MEMORY_SIZE);
    memset(Memory, 0, sizeof(bigint) * MEMORY_SIZE);

    FILE* Input = fopen("day11_input.txt", "r");
    if(!Input)
    {
        puts("No program file\n");
        exit(1);
    }
    int Size = 0;
    char Integer[128];
    while(fscanf(Input, "%[^,],", Integer) != EOF)
    {
        Memory[Size++] = BigIntParse(Integer);
    }

    puts("Program loaded");

    // Hull
    struct panel* Panels = malloc(sizeof(struct panel) * 16 * 1024);
    int PanelCount = 0;

    Panels[PanelCount++] = (struct panel)
    {
        .Pos = {0,0},
        .Col = COL_WHITE,
        .PaintCount = 0
    };

    // Robot
    enum col Col = COL_BLACK;
    enum dir Dir = DIR_UP;
    enum mod Mod = MOD_COL;
    struct pos Pos = {0,0};

#if OPCODE_DEBUG
    printf("%-4s %-3s %-3s %-4s\n", "IP", "OP", "PM", "RB");
#endif

    int IP = 0;
    int Rbase = 0;
    for(;;)
    {
        int I = IP++;
        int Opcode = BigIntValue(Memory[I]) % 100;
        int Pmodes = BigIntValue(Memory[I]) / 100;
        int Pord = 0;

#if OPCODE_DEBUG
        printf("%04d %-3s %03d %04d\n", IP, OPCODES[Opcode], Pmodes, Rbase);
#endif

        switch(Opcode)
        {
            case OP_ADD:
            {
                bigint P1 = Load();
                bigint P2 = Load();
                Store(BigIntAdd(P1, P2));
                break;
            }
            case OP_MUL:
            {
                bigint P1 = Load();
                bigint P2 = Load();
                Store(BigIntMul(P1, P2));
                break;
            }
            case OP_IN:
            {
                int Index = GetPanel(PanelCount, Panels, Pos);
                Store(BigIntMake(Index == -1 ? COL_BLACK : Panels[Index].Col));
                break;
            }
            case OP_OUT:
            {
                if(Mod == MOD_COL)
                {
                    Col = BigIntValue(Load());
                    //printf("%-8s %s\n", "Color:", Col == COL_BLACK ? "BLACK" : "WHITE");
                    Mod = MOD_MOV;
                }
                else // MOD_MOV
                {
                    // Paint
                    int Index = GetPanel(PanelCount, Panels, Pos);
                    if(Index == -1)
                    {
                        Panels[PanelCount++] = (struct panel)
                        {
                            .Pos = Pos,
                            .Col = Col,
                            .PaintCount = 1
                        };
                    }
                    else
                    {
                        Panels[Index].Col = Col;
                        Panels[Index].PaintCount += 1;
                    }
                    //printf("%-8s %2d %2d\n", "Paint:", Pos.X, Pos.Y);

                    // Advance
                    Dir = Rotate(Dir, BigIntValue(Load()));
                    Pos = Advance(Pos, Dir);

                    Mod = MOD_COL;
                }
                break;
            }
            case OP_JT:
            {
                int P1 = BigIntValue(Load());
                int P2 = BigIntValue(Load());
                if(P1 != 0)
                {
                    IP = P2;
                }
                break;
            }
            case OP_JF:
            {
                int P1 = BigIntValue(Load());
                int P2 = BigIntValue(Load());
                if(P1 == 0)
                {
                    IP = P2;
                }
                break;
            }
            case OP_TLT:
            {
                bigint P1 = Load();
                bigint P2 = Load();
                Store(BigIntLessThan(P1, P2) ? BigIntMake(1) : BigIntMake(0));
                break;
            }
            case OP_TEQ:
            {
                bigint P1 = Load();
                bigint P2 = Load();
                Store(BigIntEqual(P1, P2) ? BigIntMake(1) : BigIntMake(0));
                break;
            }
            case OP_RBO:
            {
                Rbase += BigIntValue(Load());
                break;
            }
            case OP_HLT:
            {
                for(int Y = -10; Y < +10; ++Y)
                {
                    for(int X = -10; X < +50; ++X)
                    {
                        if(Pos.X == X && Pos.Y == Y)
                        {
                            printf("R");
                        }
                        else
                        {
                            int Index = GetPanel(PanelCount, Panels, (struct pos){X,Y});
                            if(Index == -1 || Panels[Index].Col == COL_BLACK)
                            {
                                printf(".");
                            }
                            else
                            {
                                printf("#");
                            }
                        }
                    }
                    printf("\n");
                }
                printf("Painted %d panels\n", PanelCount);
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
