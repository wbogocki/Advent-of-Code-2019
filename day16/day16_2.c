/*
    ** Advent of Code 2019 **
    ** Day 16 Part 2 **
    ** Wojciech Bogócki **
    ** 19 Dec 2019 Taipei **
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIGNAL_MAX (1<<24)

int
main(void)
{
    int* Signal = malloc(sizeof(int) * SIGNAL_MAX);
    int Count = 0;

    FILE* File = fopen("day16_input.txt", "r");
    while(fscanf(File, "%1d", &Signal[Count]) != EOF)
    {
        ++Count;
    }

    int Repeat = 10000;
    for(int Index = 1;
        Index < Repeat;
        ++Index)
    {
        memcpy(&Signal[Count*Index], Signal, sizeof(int) * Count);
    }
    Count *= Repeat;

    int Offset = Signal[0] * 1000000
               + Signal[1] * 100000
               + Signal[2] * 10000
               + Signal[3] * 1000
               + Signal[4] * 100
               + Signal[5] * 10
               + Signal[6] * 1;

    for(int Phase = 0;
        Phase < 100;
        ++Phase)
    {
        int Sum = 0;
        for(int Index = Count-1;
            Index >= Count/2;
            --Index)
        {
            Sum += Signal[Index];
            Signal[Index] = abs(Sum % 10);
        }
    }

    printf("Result: ");
    for(int Index = 0;
        Index < 8;
        ++Index)
    {
        printf("%d", Signal[(Index + Offset) % Count]);
    }
    printf("\n");
}