/*
    ** Advent of Code 2019 **
    ** Day 7 Part 1 **
    ** Wojciech Bogócki **
    ** 8 Dec 2019 Taipei **
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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

struct ioport
{
    int Data[16];
    int Head;
    int Tail;
};

int Read(struct ioport* Port)
{
    return Port->Data[Port->Tail++];
}

void Write(struct ioport* Port, int Value)
{
    Port->Data[Port->Head++] = Value;
}

/*
    Resolves parameter to a value from:
    - parameter modes (Pmodes),
    - parameter ordinal number (Pord),
    - the immediate value of the parameter (P),
    - and memory (Memory).
*/
int Pval(int Pmodes, int Pord, int P, int Memory[])
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord-1] % 10;
    return Pmode == 0 ? Memory[P] : P;
}

int Run(int Memory[], struct ioport* Input, struct ioport* Output)
{
    int IP = 0;
    for(;;)
    {
        int I = IP++;
        int Opcode = Memory[I] % 100;
        int Pmodes = Memory[I] / 100;
        switch(Opcode)
        {
            case OP_ADD:
            {
                int P1 = Memory[IP++];
                int P2 = Memory[IP++];
                int P3 = Memory[IP++];
                int R1 = Pval(Pmodes, 1, P1, Memory);
                int R2 = Pval(Pmodes, 2, P2, Memory);
                Memory[P3] = R1 + R2;
                break;
            }
            case OP_MUL:
            {
                int P1 = Memory[IP++];
                int P2 = Memory[IP++];
                int P3 = Memory[IP++];
                int R1 = Pval(Pmodes, 1, P1, Memory);
                int R2 = Pval(Pmodes, 2, P2, Memory);
                Memory[P3] = R1 * R2;
                break;
            }
            case OP_IN:
            {
                int P1 = Memory[IP++];
                Memory[P1] = Read(Input);
                break;
            }
            case OP_OUT:
            {
                int P1 = Memory[IP++];
                int R1 = Pval(Pmodes, 1, P1, Memory);
                Write(Output, R1);
                break;
            }
            case OP_JT:
            {
                int P1 = Memory[IP++];
                int P2 = Memory[IP++];
                int R1 = Pval(Pmodes, 1, P1, Memory);
                int R2 = Pval(Pmodes, 2, P2, Memory);
                if(R1 != 0)
                {
                    IP = R2;
                }
                break;
            }
            case OP_JF:
            {
                int P1 = Memory[IP++];
                int P2 = Memory[IP++];
                int R1 = Pval(Pmodes, 1, P1, Memory);
                int R2 = Pval(Pmodes, 2, P2, Memory);
                if(R1 == 0)
                {
                    IP = R2;
                }
                break;
            }
            case OP_TLT:
            {
                int P1 = Memory[IP++];
                int P2 = Memory[IP++];
                int P3 = Memory[IP++];
                int R1 = Pval(Pmodes, 1, P1, Memory);
                int R2 = Pval(Pmodes, 2, P2, Memory);
                Memory[P3] = R1 < R2;
                break;
            }
            case OP_TEQ:
            {
                int P1 = Memory[IP++];
                int P2 = Memory[IP++];
                int P3 = Memory[IP++];
                int R1 = Pval(Pmodes, 1, P1, Memory);
                int R2 = Pval(Pmodes, 2, P2, Memory);
                Memory[P3] = R1 == R2;
                break;
            }
            case OP_HLT:
            {
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

int main(void)
{
    int Program[1024];

    FILE* ProgramFile = fopen("day7_input.txt", "r");
    int Size = 0;
    while(fscanf(ProgramFile, "%d,", &Program[Size]) != EOF)
    {
        ++Size;
    }

    int OptimalPhaseSettings[5] = {0};
    int MaximumThrust = 0;

    int ReferencePhaseSettings[5] = {0,1,2,3,4};

    for(int PosA = 0; PosA < 5; ++PosA)
    for(int PosB = 0; PosB < 5; ++PosB)
    for(int PosC = 0; PosC < 5; ++PosC)
    for(int PosD = 0; PosD < 5; ++PosD)
    for(int PosE = 0; PosE < 5; ++PosE)
    {
        if(((1 << PosA) | (1 << PosB) | (1 << PosC) | (1 << PosD) | (1 << PosE)) != 0b11111)
        {
            continue;
        }

        int PhaseSettings[5];
        PhaseSettings[0] = ReferencePhaseSettings[PosA];
        PhaseSettings[1] = ReferencePhaseSettings[PosB];
        PhaseSettings[2] = ReferencePhaseSettings[PosC];
        PhaseSettings[3] = ReferencePhaseSettings[PosD];
        PhaseSettings[4] = ReferencePhaseSettings[PosE];

        struct ioport Input = {0};
        struct ioport Output = {0};

        for(int Phase = 0; Phase < 5; ++Phase)
        {
            int Memory[1024];
            memcpy(Memory, Program, sizeof(int) * Size);

            int PhaseSetting = PhaseSettings[Phase];
            int InputSignal = Phase == 0 ? 0 : Read(&Output);

            Write(&Input, PhaseSetting);
            Write(&Input, InputSignal);

            Run(Memory, &Input, &Output);
        }

        int Thrust = Read(&Output);

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
