/*
    ** Advent of Code 2019 **
    ** Day 16 Part 1 **
    ** Wojciech Bogócki **
    ** 19 Dec 2019 Taipei **
*/

#include <stdio.h>

int main(void)
{
    int Signal[1024] = {0};
    int Count = 0;

    FILE* File = fopen("day16_input.txt", "r");
    while(fscanf(File, "%1d", &Signal[Count]) != EOF)
    {
        ++Count;
    }

    int BasePattern[] = {0, 1, 0, -1};

    int Pattern[1024][1024] = {0};
    for(int Round = 0;
        Round < Count;
        ++Round)
    {
        for(int Position = 0;
            Position < Count;
            ++Position)
        {
            Pattern[Round][Position] = BasePattern[((Position+1) / (Round+1)) % 4];
        }
    }

    for(int Phase = 0;
        Phase < 100;
        ++Phase)
    {
        int Output[1024] = {0};
        for(int Round = 0;
            Round < Count;
            ++Round)
        {
            for(int Position = 0;
                Position < Count;
                ++Position)
            {
                Output[Round] += Signal[Position] * Pattern[Round][Position];
            }
            Output[Round] %= 10;
            if(Output[Round] < 0)
            {
                Output[Round] *= -1;
            }
        }
        for(int Index = 0;
            Index < Count;
            ++Index)
        {
            Signal[Index] = Output[Index];
        }
    }

    for(int Index = 0;
        Index < 8;
        ++Index)
    {
        printf("%d", Signal[Index]);
    }
    printf("\n");
}