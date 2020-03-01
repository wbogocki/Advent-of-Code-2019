/*
    ** Advent of Code 2019 **
    ** Day 9 Part 1 & 2 **
    ** Wojciech Bogócki **
    ** 9-10 Dec 2019 Taipei **
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIGINT_IMPL
#include "day9_bigint.h"

//
// As it turns out, this does not require bigint, regular int64 is fine!
//

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

bigint Pget(int Pmodes, int Pord, bigint P, int Rbase, bigint Memory[])
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord] % 10;
    bigint Result;
    switch(Pmode)
    {
        case M_POS:
        {
            Result = Memory[BigIntValue(P)];
            break;
        }
        case M_IMM:
        {
            Result = P;
            break;
        }
        case M_REL:
        {
            Result = Memory[Rbase + BigIntValue(P)];
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

void Pset(int Pmodes, int Pord, bigint P, int Rbase, bigint Memory[], bigint Value)
{
    int Pow10[] = {1, 10, 100};
    int Pmode = Pmodes / Pow10[Pord] % 10;
    switch(Pmode)
    {
        case M_POS:
        {
            Memory[BigIntValue(P)] = Value;
            break;
        }
        case M_REL:
        {
            Memory[Rbase + BigIntValue(P)] = Value;
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

int main(void)
{
    bigint* Memory = malloc(sizeof(bigint)*1024*1024);
    memset(Memory, 0, sizeof(bigint)*1024*1024);

    FILE* Input = fopen("day9_input.txt", "r");
    int Size = 0;
    char Integer[128];
    while(fscanf(Input, "%[^,],", Integer) != EOF)
    {
        Memory[Size++] = BigIntParse(Integer);
    }

    //printf("%-4s %-3s %-3s %-4s\n", "IP", "OP", "PM", "RB");

    int IP = 0;
    int Rbase = 0;
    for(;;)
    {
        int I = IP++;
        int Opcode = BigIntValue(Memory[I]) % 100;
        int Pmodes = BigIntValue(Memory[I]) / 100;
        int Pord = 0;

        //printf("%04d %-3s %03d %04d\n", IP, OPCODES[Opcode], Pmodes, Rbase);

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
                printf("Input: ");
                char Value[128];
                scanf("%s", Value);
                Store(BigIntParse(Value));
                break;
            }
            case OP_OUT:
            {
                printf("Output: ");
                BigIntPrint(Load());
                printf("\n");
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
