/*
    ** Advent of Code 2019 **
    ** Day 7 Part 2 **
    ** Wojciech Bogócki **
    ** 8 Dec 2019 Taipei **
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ioport
{
    int Data[16];
    int Head;
    int Tail;
};

int Read(struct ioport* Port)
{
    if(Port->Tail == Port->Head)
    {
        puts("ioport empty");
        exit(1);
    }
    return Port->Data[Port->Tail++];
}

void Write(struct ioport* Port, int Value)
{
    if(Port->Head == sizeof(Port->Data)-1)
    {
        puts("ioport full");
        exit(1);
    }
    Port->Data[Port->Head++] = Value;
}

bool Readable(struct ioport* Port)
{
    return Port->Tail != Port->Head;
}

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
    OP_HLT = 99
};

enum interrupt
{
    INT_NON,
    INT_IOP,
    INT_HLT
};

struct computer
{
    int Memory[1024];
    int IP;
    struct ioport* Input;
    struct ioport* Output;
};

void InitComputer(struct computer* Computer, int Size, int Memory[], struct ioport* Input, struct ioport* Output)
{
    memcpy(Computer->Memory, Memory, sizeof(int) * Size);
    Computer->IP = 0;
    Computer->Input = Input;
    Computer->Output = Output;
};

/*
    Resolves parameter to a value from:
    - parameter modes (Pmodes),
    - parameter ordinal number 0,1,2 (Pord),
    - the immediate value of the parameter (P),
    - and memory (Memory).
*/
int Pval(int Pmodes, int Pord, int P, int Memory[])
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord] % 10;
    return Pmode == 0 ? Memory[P] : P;
}

#define Load(void) Pval(Pmodes, Pord++, Computer->Memory[Computer->IP++], Computer->Memory)
#define Store(Value) Computer->Memory[Computer->Memory[Computer->IP++]] = (Value);

enum interrupt Step(struct computer* Computer)
{
    int I = Computer->IP++;
    int Opcode = Computer->Memory[I] % 100;
    int Pmodes = Computer->Memory[I] / 100;
    int Pord = 0;
    switch(Opcode)
    {
        case OP_ADD:
        {
            int P1 = Load();
            int P2 = Load();
            Store(P1 + P2);
            break;
        }
        case OP_MUL:
        {
            int P1 = Load();
            int P2 = Load();
            Store(P1 * P2);
            break;
        }
        case OP_IN:
        {
            if(Readable(Computer->Input))
            {
                Store(Read(Computer->Input));
            }
            else
            {
                --Computer->IP;
                return INT_IOP;
            }
            break;
        }
        case OP_OUT:
        {
            Write(Computer->Output, Load());
            break;
        }
        case OP_JT:
        {
            int P1 = Load();
            int P2 = Load();
            if(P1 != 0)
            {
                Computer->IP = P2;
            }
            break;
        }
        case OP_JF:
        {
            int P1 = Load();
            int P2 = Load();
            if(P1 == 0)
            {
                Computer->IP = P2;
            }
            break;
        }
        case OP_TLT:
        {
            int P1 = Load();
            int P2 = Load();
            Store(P1 < P2);
            break;
        }
        case OP_TEQ:
        {
            int P1 = Load();
            int P2 = Load();
            Store(P1 == P2);
            break;
        }
        case OP_HLT:
        {
            --Computer->IP;
            return INT_HLT;
        }
        default:
        {
            printf("Bad opcode: %d\n", Opcode);
            exit(1);
        }
    }
    return INT_NON;
}

int Run(int Size, int Program[], int PhaseSettings[])
{
    struct ioport Ports[5] = {0};

    Write(&Ports[0], PhaseSettings[0]);
    Write(&Ports[1], PhaseSettings[1]);
    Write(&Ports[2], PhaseSettings[2]);
    Write(&Ports[3], PhaseSettings[3]);
    Write(&Ports[4], PhaseSettings[4]);

    Write(&Ports[0], 0);

    struct computer Computers[5];

    InitComputer(&Computers[0], Size, Program, &Ports[0], &Ports[1]);
    InitComputer(&Computers[1], Size, Program, &Ports[1], &Ports[2]);
    InitComputer(&Computers[2], Size, Program, &Ports[2], &Ports[3]);
    InitComputer(&Computers[3], Size, Program, &Ports[3], &Ports[4]);
    InitComputer(&Computers[4], Size, Program, &Ports[4], &Ports[0]);

    int Phase = 0;
    for(;;)
    {
        enum interrupt Interrupt = Step(&Computers[Phase]);
        if(Phase == 4 && Interrupt == INT_HLT)
        {
            break;
        }
        if(Interrupt != INT_NON)
        {
            Phase = (Phase + 1) % 5;
        }
    }

    return Read(&Ports[0]);
}

int main(void)
{
    int Program[1024];

    FILE* Input = fopen("day7_input.txt", "r");
    int Size = 0;
    while(fscanf(Input, "%d,", &Program[Size]) != EOF)
    {
        ++Size;
    }

    int OptimalPhaseSettings[5] = {0};
    int MaximumThrust = 0;

    int ReferencePhaseSettings[5] = {9,8,7,6,5};

    for(int PosA = 0; PosA <= 4; ++PosA)
    for(int PosB = 0; PosB <= 4; ++PosB)
    for(int PosC = 0; PosC <= 4; ++PosC)
    for(int PosD = 0; PosD <= 4; ++PosD)
    for(int PosE = 0; PosE <= 4; ++PosE)
    {
        if(((1 << PosA) | (1 << PosB) | (1 << PosC) | (1 << PosD) | (1 << PosE)) != 0b11111)
        {
            continue;
        }

        int PhaseSettings[5] =
        {
            [0] = ReferencePhaseSettings[PosA],
            [1] = ReferencePhaseSettings[PosB],
            [2] = ReferencePhaseSettings[PosC],
            [3] = ReferencePhaseSettings[PosD],
            [4] = ReferencePhaseSettings[PosE]
        };

        int Thrust = Run(Size, Program, PhaseSettings);

        if(Thrust > MaximumThrust)
        {
            memcpy(OptimalPhaseSettings, PhaseSettings, sizeof(OptimalPhaseSettings));
            MaximumThrust = Thrust;
        }
    }

    printf("Phase settings = ");
    for(int Phase = 0; Phase < 5; ++Phase)
    {
        printf("%d%c", OptimalPhaseSettings[Phase], Phase == 4 ? '\n' : ',');
    }
    printf("Thrust = %d\n", MaximumThrust);
}
