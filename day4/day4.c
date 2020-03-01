/*
    ** Advent of Code 2019 **
    ** Day 4 Part 1 **
    ** Wojciech Bogócki **
    ** 4 Dec 2019 **
*/

#include <stdbool.h>
#include <stdio.h>
#include <math.h>

static inline
int GetDigit(int Number, int Digit)
{
    int A = Number / (int) pow(10, Digit);
    return A - A / 10 * 10;
}

int main(void)
{
    int Count = 0;
    for(int Guess = 108457; Guess <= 562041; ++Guess)
    {
        bool NeighbourDigitsTest = false;
        bool IncreasingDigitsTest = true;
        for(int Digit = 0; Digit < 6; ++Digit)
        {
            int Left = GetDigit(Guess, Digit+1);
            int Right = GetDigit(Guess, Digit);
            if(Left == Right) NeighbourDigitsTest = true;
            if(Left > Right) IncreasingDigitsTest = false;
        }
        if(!NeighbourDigitsTest || !IncreasingDigitsTest)
        {
            if(Guess == 111111) puts("111111 failed (BAD)");
            if(Guess == 223450) puts("223450 failed (GOOD)");
            if(Guess == 123789) puts("123789 failed (GOOD)");
            continue;
        }
        ++Count;
    }
    printf("Result: %d\n", Count);
}
