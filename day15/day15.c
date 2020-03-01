/*
    ** Advent of Code 2019 **
    ** Day 15 Part 1 **
    ** Wojciech Bogócki **
    ** 17 Dec 2019 Taipei **
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Computer

#define MEMORY_SIZE (16 * 1024)

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
    PMOD_POS = 0,
    PMOD_IMM = 1,
    PMOD_REL = 2
};

enum interrupt
{
    INT_HLT = 0,
    INT_IN  = 1,
    INT_OUT = 2
};

enum flags
{
    F_HLT = 1,
    F_IN  = 2
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

icv
Pget(int Pmodes, int Pord, icv P, int Rbase, icv Memory[])
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord] % 10;
    icv Result;
    switch(Pmode)
    {
        case PMOD_POS:
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
        case PMOD_IMM:
        {
            Result = P;
            break;
        }
        case PMOD_REL:
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
        case PMOD_POS:
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
        case PMOD_REL:
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

#define Load(void) Pget(Pmodes, Pord++, Computer->Memory[Computer->IP++], Computer->Rbase, Computer->Memory)
#define Store(Value) Pset(Pmodes, Pord++, Computer->Memory[Computer->IP++], Computer->Rbase, Computer->Memory, (Value))
#define Set(Flag) (Computer->Flags |= (Flag))
#define Unset(Flag) (Computer->Flags &= ~(Flag))
#define Test(Flag) ((Computer->Flags & (Flag)) == (Flag))

enum interrupt
Run(struct computer* Computer)
{
    //printf("%-4s %-3s %-3s %-4s\n", "IP", "OP", "PM", "RB");

    if(Test(F_HLT))
    {
        return INT_HLT;
    }

    for(;;)
    {
        int I = Computer->IP++;
        int Opcode = Computer->Memory[I] % 100;
        int Pmodes = Computer->Memory[I] / 100;
        int Pord = 0;

        //printf("%04d %-3s %03d %04d\n", IP, OPCODES[Opcode], Pmodes, Rbase);

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
                if(Test(F_IN))
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
                if(P1 != 0)
                {
                    Computer->IP = P2;
                }
                break;
            }
            case OP_JF:
            {
                icv P1 = Load();
                icv P2 = Load();
                if(P1 == 0)
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

void
LoadMemory(const char* Path, icv Memory[])
{
    FILE* File = fopen(Path, "r");
    if(!File)
    {
        puts("No program file\n");
        exit(1);
    }
    int Size = 0;
    while(fscanf(File, "%lld,", &Memory[Size]) != EOF)
    {
        Size++;
    }
    fclose(File);
}

// Droid

struct vec
{
    int X;
    int Y;
};

enum mvcmd
{
    MV_NORTH = 1,
    MV_SOUTH = 2,
    MV_WEST  = 3,
    MV_EAST  = 4
};

const char* MVCMD[] =
{
    [MV_NORTH] = "N",
    [MV_SOUTH] = "S",
    [MV_WEST]  = "W",
    [MV_EAST]  = "E"
};

enum status
{
    ST_WALL = 0, // hit a wall
    ST_MVEM = 1, // moved on empty tile
    ST_MVOX = 2, // moved on oxygen system tile
};

const char* STATUS[] =
{
    "WALL",
    "MVEM",
    "MVOX"
};

enum tiletype
{
    TIL_NEXP = 0, // not explored
    TIL_EMPT = 1, // empty
    TIL_WALL = 2, // wall
    TIL_OXYS = 3  // oxygen system
};

struct tile
{
    enum tiletype Type;
    struct vec Position;
    int Distance;
};

enum search
{
    SE_EXPLORE,
    SE_RETURN
};

int
Min(int A, int B)
{
    return A < B ? A : B;
}

int
Max(int A, int B)
{
    return A > B ? A : B;
}

struct tile*
GetTile(int* Count, struct tile Tiles[], struct vec Position)
{
    for(int Index = 0;
        Index < *Count;
        ++Index)
    {
        if(Tiles[Index].Position.X == Position.X && Tiles[Index].Position.Y == Position.Y)
        {
            return &Tiles[Index];
        }
    }
    struct tile* Tile = &Tiles[(*Count)++];
    *Tile = (struct tile)
    {
        .Type = TIL_NEXP,
        .Position = Position,
        .Distance = 0
    };
    return Tile;
}

enum mvcmd
Opposite(enum mvcmd MovementCommand)
{
    switch(MovementCommand)
    {
        case MV_NORTH: return MV_SOUTH;
        case MV_SOUTH: return MV_NORTH;
        case MV_WEST:  return MV_EAST;
        case MV_EAST:  return MV_WEST;
    }
}

struct vec
GetPositionAfterMove(struct vec Position, enum mvcmd MoveCommand)
{
    switch(MoveCommand)
    {
        case MV_NORTH: return (struct vec){.X = Position.X,   .Y = Position.Y-1};
        case MV_SOUTH: return (struct vec){.X = Position.X,   .Y = Position.Y+1};
        case MV_WEST:  return (struct vec){.X = Position.X-1, .Y = Position.Y};
        case MV_EAST:  return (struct vec){.X = Position.X+1, .Y = Position.Y};
    }
}

enum mvcmd
DroidPickPath(struct vec* Position, int* PathCount, enum mvcmd Path[], int* TileCount, struct tile Tiles[])
{
    for(enum mvcmd MovementCommand = MV_NORTH;
        MovementCommand <= MV_EAST;
        ++MovementCommand)
    {
        if(GetTile(TileCount, Tiles, GetPositionAfterMove(*Position, MovementCommand))->Type == TIL_NEXP)
        {
            return MovementCommand;
        }
    }
    if(*PathCount > 0)
    {
        return Opposite(Path[(*PathCount)-1]);
    }
    else
    {
        return MV_NORTH;
    }
}

void
DroidMove(struct vec* Position, enum mvcmd MovementCommand, enum status Status, int* PathCount, enum mvcmd Path[], int* TileCount, struct tile Tiles[])
{
    struct vec PositionAfterMove = GetPositionAfterMove(*Position, MovementCommand);
    switch(Status)
    {
        case ST_WALL:
        {
            GetTile(TileCount, Tiles, PositionAfterMove)->Type = TIL_WALL;
            break;
        }
        case ST_MVEM:
        {
            GetTile(TileCount, Tiles, PositionAfterMove)->Type = TIL_EMPT;
            *Position = PositionAfterMove;
            if(*PathCount > 0 && MovementCommand == Opposite(Path[(*PathCount)-1]))
            {
                --(*PathCount);
            }
            else
            {
                Path[(*PathCount)++] = MovementCommand;
            }
            break;
        }
        case ST_MVOX:
        {
            GetTile(TileCount, Tiles, PositionAfterMove)->Type = TIL_OXYS;
            *Position = PositionAfterMove;
            if(*PathCount > 0 && MovementCommand == Opposite(Path[(*PathCount)-1]))
            {
                --(*PathCount);
            }
            else
            {
                Path[(*PathCount)++] = MovementCommand;
            }
            break;
        }
    }
}

#define MAX_PATH (16 * 1024)
#define MAX_TILES (16 * 1024)

int
main(void)
{
    struct computer Computer = {0};
    struct vec Position = {0,0};
    enum mvcmd MovementCommand = MV_NORTH;
    enum status Status = 0;
    enum mvcmd Path[MAX_PATH] = {0};
    int PathCount = 0;
    struct tile Tiles[MAX_TILES] = {0};
    int TileCount = 0;
    struct vec DrawMin = {-3,-3};
    struct vec DrawMax = {3,3};
    struct vec OxygenPosition = {0};
    bool OxygenFound = false;

    // Map

    GetTile(&TileCount, Tiles, (struct vec){0,0})->Type = TIL_EMPT;

    LoadMemory("day15_input.txt", Computer.Memory);

    while(!(OxygenFound && Position.X == 0 && Position.Y == 0))
    {
        enum interrupt Interrupt = Run(&Computer);
        switch(Interrupt)
        {
            case INT_IN:
            {
                MovementCommand = DroidPickPath(&Position, &PathCount, Path, &TileCount, Tiles);
                Computer.In = MovementCommand;
                break;
            }
            case INT_OUT:
            {
                enum status Status = Computer.Out;
                DroidMove(&Position, MovementCommand, Status, &PathCount, Path, &TileCount, Tiles);
                if(Status == 2)
                {
                    OxygenPosition = Position;
                    OxygenFound = true;
                }
                DrawMin.X = Min(DrawMin.X, Position.X);
                DrawMin.Y = Min(DrawMin.Y, Position.Y);
                DrawMax.X = Max(DrawMax.X, Position.X);
                DrawMax.Y = Max(DrawMax.Y, Position.Y);
                break;
            }
            case INT_HLT:
            {
                printf("Halted\n");
                return 0;
            }
            default:
            {
                printf("Bad interrupt: %d\n", Interrupt);
                return 1;
            }
        }
    }

    // Distance to each tile

    struct tile* Stack[MAX_TILES] = {0};
    int StackSize = 0;

    Stack[StackSize++] = GetTile(&TileCount, Tiles, (struct vec){0,0});

    while(StackSize > 0)
    {
        struct tile* Tile = Stack[--StackSize];

        for(enum mvcmd MovementCommand = MV_NORTH;
            MovementCommand <= MV_EAST;
            ++MovementCommand)
        {
            struct tile* Neighbour = GetTile(&TileCount, Tiles, GetPositionAfterMove(Tile->Position, MovementCommand));

            if(Neighbour->Type != TIL_WALL && Neighbour->Distance == 0)
            {
                Neighbour->Distance = Tile->Distance + 1;
                Stack[StackSize++] = Neighbour;
            }
        }
    }

    // Draw

    for(int Y = DrawMin.Y-1;
        Y <= DrawMax.Y+1;
        ++Y)
    {
        for(int X = DrawMin.X-1;
            X <= DrawMax.X+1;
            ++X)
        {
            if(X == 0 && Y == 0)
            {
                putchar('D');
            }
            else
            {
                switch(GetTile(&TileCount, Tiles, (struct vec){X,Y})->Type)
                {
                    case TIL_NEXP: putchar(' '); break;
                    case TIL_EMPT: putchar('.'); break;
                    case TIL_WALL: putchar('#'); break;
                    case TIL_OXYS: putchar('O'); break;
                }
            }
        }
        putchar('\n');
    }

    // for(int Y = DrawMin.Y-1;
    //     Y <= DrawMax.Y+1;
    //     ++Y)
    // {
    //     for(int X = DrawMin.X-1;
    //         X <= DrawMax.X+1;
    //         ++X)
    //     {
    //         if(X == 0 && Y == 0)
    //         {
    //             printf("000 ");
    //         }
    //         else
    //         {
    //             struct tile* Tile = GetTile(&TileCount, Tiles, (struct vec){X,Y});
    //             switch(Tile->Type)
    //             {
    //                 case TIL_NEXP: printf("%03d ", Tile->Distance); break;
    //                 case TIL_EMPT: printf("%03d ", Tile->Distance); break;
    //                 case TIL_WALL: printf("%03d ", Tile->Distance); break;
    //                 case TIL_OXYS: printf("%03d ", Tile->Distance); break;
    //             }
    //         }
    //     }
    //     putchar('\n');
    // }

    printf("Result: %d\n", GetTile(&TileCount, Tiles, OxygenPosition)->Distance);
}
