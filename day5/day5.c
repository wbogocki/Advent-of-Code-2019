/*
    ** Advent of Code 2019 **
    ** Day 5 Part 1 **
    ** Wojciech Bogócki **
    ** 5 Dec 2019 **
*/

#include <math.h>
#include <stdio.h>

int GetDigit(int Number, int Digit)
{
    int A = Number / (int) pow(10, Digit);
    return A - A / 10 * 10;
}

int Pmode(int Pmodes, int Pord)
{
    int Pmode = GetDigit(Pmodes, Pord - 1);
    //printf("Pmode in %03d of param %d: %d\n", Pmodes, Pord, Pmode);
    return Pmode;
}

int Read(int Pmode, int P, int Memory[])
{
    switch(Pmode)
    {
        case 0:
        {
            return Memory[P];
        }
        case 1:
        {
            return P;
        }
        default:
        {
            printf("Bad parameter mode: %d\n", Pmode);
            return 0;
        }
    }
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

    int *IP = Memory;
    for(;;)
    {
        int I = *IP++;
        int Opcode = I % 100;
        int Pmodes = I / 100;
        switch(Opcode)
        {
            case 1:
            {
                // Add
                int P1 = *IP++;
                int P2 = *IP++;
                int P3 = *IP++;
                Memory[P3] = Read(Pmode(Pmodes, 1), P1, Memory)
                           + Read(Pmode(Pmodes, 2), P2, Memory);
                break;
            }
            case 2:
            {
                // Mul
                int P1 = *IP++;
                int P2 = *IP++;
                int P3 = *IP++;
                Memory[P3] = Read(Pmode(Pmodes, 1), P1, Memory)
                           * Read(Pmode(Pmodes, 2), P2, Memory);
                break;
            }
            case 3:
            {
                // Input
                int P1 = *IP++;
                printf("Input: ");
                scanf("%d", &Memory[P1]);
                break;
            }
            case 4:
            {
                // Output
                int P1 = *IP++;
                printf("Output: %d\n", Read(Pmode(Pmodes, 1), P1, Memory));
                break;
            }
            case 99:
            {
                // Hlt
                return 0;
            }
            default:
            {
                printf("Bad opcode: %d\n", Opcode);
            }
        }
    }
}
