/*
    ** Advent of Code 2019 **
    ** Day 2 Part 1 **
    ** Wojciech Bogócki **
    ** 2 Dec 2019 **
*/

#include <stdio.h>

int main(void)
{
    int Memory[1024];

    FILE* Input = fopen("input.txt", "r");

    int Count = 0;
    while (fscanf(Input, "%d", &Memory[Count]) != EOF)
    {
        Count++;
    }

    Memory[1] = 12;
    Memory[2] = 2;

    int IP = 0;
    for(;;)
    {
        int *Intcode = Memory + IP;
        int Opcode = Intcode[0];
        switch(Opcode)
        {
            case 1:
            {
                int P1 = Intcode[1];
                int P2 = Intcode[2];
                int P3 = Intcode[3];
                Memory[P3] = Memory[P1] + Memory[P2];
                printf("Add %d %d %d\n", P1, P2, P3);
                break;
            }
            case 2:
            {
                int P1 = Intcode[1];
                int P2 = Intcode[2];
                int P3 = Intcode[3];
                Memory[P3] = Memory[P1] * Memory[P2];
                printf("Mul %d %d %d\n", P1, P2, P3);
                break;
            }
            case 99:
            {
                printf("Hlt\n");
                printf("Result: %d\n", Memory[0]);
                return 0;
            }
            default:
            {
                printf("Bad opcode: %d\n", Opcode);
                return 1;
            }
        }
        IP += 4;
    }
}