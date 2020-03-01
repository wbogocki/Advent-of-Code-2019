/*
    ** Advent of Code 2019 **
    ** Day 5 Part 2 **
    ** Wojciech Bogócki **
    ** 5 Dec 2019 **
*/

#include <stdio.h>

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

int main(void)
{
    int Memory[1024];

    FILE* Program = fopen("day5_input.txt", "r");
    int Size = 0;
    while(fscanf(Program, "%d,", &Memory[Size]) != EOF)
    {
        ++Size;
    }

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
                printf("Input: ");
                scanf("%d", &Memory[P1]);
                break;
            }
            case OP_OUT:
            {
                int P1 = Memory[IP++];
                int R1 = Pval(Pmodes, 1, P1, Memory);
                printf("Output: %d\n", R1);
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
