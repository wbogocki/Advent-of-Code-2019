/*
    ** Advent of Code 2019 **
    ** Day 10 Part 2 **
    ** Wojciech Bogócki **
    ** 11 Dec 2019 Taipei **
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

// FUCK
// FUCK
// FUCK
// FUCK
// FUCK
// FUCK
// FUCK
// FUCK
// FUCK

// It works, OK

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

double
GetAngle(struct vec V)
{
    double Angle = atan2(V.Y, V.X);
    Angle /= 2*M_PI;
    Angle -= 0.75;
    while(Angle < 0) ++Angle;
    return Angle;
}

int
FindStar(int StarCount, struct vec Stars[], struct vec Position)
{
    for(int Index = 0; Index < StarCount; ++Index)
    {
        if(Stars[Index].X == Position.X && Stars[Index].Y == Position.Y)
        {
            return Index;
        }
    }
    return -1;
}

int
FindStarTowards(int StarCount, struct vec Stars[], struct vec P0, struct vec Angle)
{
    int Result = -1;
    double MinimumDistance = INT_MAX;
    for(int Index = 0; Index < StarCount; ++Index)
    {
        struct vec StarAngle =
        {
            .X = Stars[Index].X - P0.X,
            .Y = Stars[Index].Y - P0.Y
        };
        if(GetAngle(StarAngle) == GetAngle(Angle))
        {
            double Distance = sqrt(pow(StarAngle.X, 2) + pow(StarAngle.Y, 2));
            if(Distance < MinimumDistance)
            {
                MinimumDistance = Distance;
                Result = Index;
            }
        }
    }
    return Result;
}

struct vec
GetNextLaserAngle(struct vec LaserPosition, struct vec LaserAngle, int StarCount, struct vec Stars[])
{
    struct vec Result = {0,-1};
    double Current = GetAngle(LaserAngle);
    double Minimum = 1;
    for(int Index = 0; Index < StarCount; ++Index)
    {
        struct vec Angle =
        {
            .X = Stars[Index].X - LaserPosition.X,
            .Y = Stars[Index].Y - LaserPosition.Y
        };
        double AngleScalar = GetAngle(Angle);
        //printf("%llf %llf\n", Current, AngleScalar);
        if(AngleScalar > Current)
        {
            if(AngleScalar < Minimum)
            {
                Result = Angle;
                Minimum = AngleScalar;
            }
        }
    }
    Result = SimplifyFraction(Result);
    return Result;
}

int
main(void)
{
    puts("start");

    struct vec* Stars = malloc(sizeof(struct vec)*1024*1024);
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

    int Map[512][512] = {0};

    struct vec LaserPosition = {26,29};
    struct vec LaserAngle = {0,-1};
    struct vec VaporizedStar;
    int VaporizedCount = 0;
    while(VaporizedCount != 200)
    {
        int StarIndex = FindStarTowards(StarCount, Stars, LaserPosition, LaserAngle);
        if(StarIndex != -1)
        {
            VaporizedStar = Stars[StarIndex];
            Stars[StarIndex] = Stars[--StarCount];
            ++VaporizedCount;
            printf("%d) %02d %02d %02d %02d %.8lf\n",
                VaporizedCount, VaporizedStar.X, VaporizedStar.Y,
                LaserAngle.X, LaserAngle.Y,
                GetAngle(LaserAngle));
            Map[VaporizedStar.X][VaporizedStar.Y] = VaporizedCount;
        }
        LaserAngle = GetNextLaserAngle(LaserPosition, LaserAngle, StarCount, Stars);
    }

    for(int Y = 0; Y < Height; ++Y)
    {
        for(int X = 0; X < Width; ++X)
        {
            if(X == LaserPosition.X && Y == LaserPosition.Y)
            {
                printf("X");
            }
            else if(Map[X][Y] > 0 && Map[X][Y] < 9)
            {
                printf("%d", Map[X][Y]);
            }
            else
            {
                int Star = FindStar(StarCount, Stars, (struct vec){X,Y});
                printf(Star != -1 ? "#" : ".");
            }
        }
        printf("\n");
    }

    printf("Result: X=%d, Y=%d\n", VaporizedStar.X, VaporizedStar.Y);
}
