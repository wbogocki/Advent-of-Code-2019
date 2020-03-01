/*
    ** Advent of Code 2019 **
    ** Day 10 Part 1 **
    ** Wojciech Bogócki **
    ** 11 Dec 2019 Taipei **
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct vec {int X, Y;};

struct vec
SimplifyFraction(struct vec F)
{
    int AbsoluteX = F.X > 0 ? F.X : -F.X;
    int AbsoluteY = F.Y > 0 ? F.Y : -F.Y;
    for(int Denominator = (AbsoluteX > AbsoluteY) ? AbsoluteX : AbsoluteY;
        Denominator > 0;
        --Denominator)
    {
        if(F.X % Denominator == 0 && F.Y % Denominator == 0)
        {
            F.X /= Denominator;
            F.Y /= Denominator;
        }
    }
    return F;
}

struct vec
FindAngle(struct vec A, struct vec B)
{
    struct vec Result =
    {
        .X = B.X - A.X,
        .Y = B.Y - A.Y,
    };
    Result = SimplifyFraction(Result);
    return Result;
}

int
CountNeighbours(struct vec Star, int StarCount, struct vec Stars[])
{
    struct vec Angles[1024] = {0};
    int Count = 0;
    for(int StarIdx = 0; StarIdx < StarCount; ++StarIdx)
    {
        struct vec Angle = FindAngle(Star, Stars[StarIdx]);
        bool Occluded = false;
        for(int Index = 0; Index < Count; ++Index)
        {
            if(Angles[Index].X == Angle.X && Angles[Index].Y == Angle.Y)
            {
                Occluded = true;
                break;
            }
        }
        if(!Occluded)
        {
            Angles[Count++] = Angle;
        }
    }
    --Count; // not a naighbour to itself
    return Count;
}

int
main(void)
{
    struct vec Stars[1024] = {0};
    int StarCount = 0;

    FILE* Input = fopen("day10_input.txt", "rb");
    int Width = 0;
    int Height = 0;
    for(;;)
    {
        char Char = fgetc(Input);
        if(Char == EOF)
        {
            break;
        }
        if(Char == '\n')
        {
            ++Height;
            Width = 0;
            continue;
        }
        if(Char == '#')
        {
            Stars[StarCount++] = (struct vec) {Width, Height};
        }
        ++Width;
    }
    ++Height;

    int BestX, BestY;
    int BestCount = 0;
    for(int StarIdx = 0; StarIdx < StarCount; ++StarIdx)
    {
        int Count = CountNeighbours(Stars[StarIdx], StarCount, Stars);
        if(Count > BestCount)
        {
            BestX = Stars[StarIdx].X;
            BestY = Stars[StarIdx].Y;
            BestCount = Count;
        }
    }
    printf("Result: X=%d, Y=%d, Count=%d\n", BestX, BestY, BestCount);
}
