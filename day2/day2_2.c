/*
    ** Advent of Code 2019 **
    ** Day 2 Part 2 **
    ** Wojciech Bogócki **
    ** 2 Dec 2019 **
*/

#include <stdio.h>
#include <string.h>

void Run(int *Memory)
{
    int *IP = Memory;
    for(;;)
    {
        int Opcode = *IP++;
        switch(Opcode)
        {
            case 1:
            {
                int P1 = *IP++;
                int P2 = *IP++;
                int P3 = *IP++;
                Memory[P3] = Memory[P1] + Memory[P2];
                // printf("Add %d %d %d\n", P1, P2, P3);
                break;
            }
            case 2:
            {
                int P1 = *IP++;
                int P2 = *IP++;
                int P3 = *IP++;
                Memory[P3] = Memory[P1] * Memory[P2];
                // printf("Mul %d %d %d\n", P1, P2, P3);
                break;
            }
            case 99:
            {
                // printf("Hlt\n");
                // printf("Result: %d\n", Memory[0]);
                return;
            }
            default:
            {
                printf("Bad opcode: %d\n", Opcode);
                return;
            }
        }
    }
}

int main(void)
{
    int Program[1024];

    FILE* Input = fopen("input.txt", "r");

    int Count = 0;
    while (fscanf(Input, "%d", &Program[Count]) != EOF)
    {
        Count++;
    }

    for(int Noun = 0; Noun <= 99; ++Noun)
    {
        for(int Verb = 0; Verb <= 99; ++Verb)
        {
            int Memory[1024];
            memcpy(Memory, Program, sizeof(Program));

            Memory[1] = Noun;
            Memory[2] = Verb;

            Run(Memory);

            int Output = Memory[0];
            if(Output == 19690720)
            {
                int Result = 100 * Noun + Verb;
                printf("Result: %d\n", Result);
            }
        }
    }
}