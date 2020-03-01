/*
    ** Advent of Code 2019 **
    ** Day 3 Part 1 **
    ** Wojciech Bogócki **
    ** 3 Dec 2019 **
*/

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct vec
{
    int X, Y;
};

int LoadCable(FILE* File, struct vec Cable[])
{    
    int Count = 0;
    char Direction;
    int Distance;
    char Delimiter;
    while(fscanf(File, "%c%d%c[^\n]", &Direction, &Distance, &Delimiter) != EOF)
    {
        switch(Direction)
        {
            case 'U':
            {
                Cable[Count++] = (struct vec) { .Y = Distance };
                break;
            }
            case 'D':
            {
                Cable[Count++] = (struct vec) { .Y = -Distance };
                break;
            }
            case 'L':
            {
                Cable[Count++] = (struct vec) { .X = -Distance };
                break;
            }
            case 'R':
            {
                Cable[Count++] = (struct vec) { .X = Distance };
                break;
            }
            default:
            {
                puts("Bad direction");
                exit(1);
            }
        }
        if(Delimiter == '\n') break;
    }
    return Count;
}

bool Between(int X, int A, int B)
{
    return (A < X && B > X) || (B < X && A > X);
}

bool Intersection(struct vec Head1, struct vec Move1, struct vec Head2, struct vec Move2, struct vec* Intersection)
{
    if(Between(Head1.X, Head2.X, Head2.X + Move2.X))
    {
        Intersection->X = Head1.X;
    }
    else if(Between(Head2.X, Head1.X, Head1.X + Move1.X))
    {
        Intersection->X = Head2.X;
    }
    else
    {
        return false;
    }

    if(Between(Head1.Y, Head2.Y, Head2.Y + Move2.Y))
    {
        Intersection->Y = Head1.Y;
    }
    else if(Between(Head2.Y, Head1.Y, Head1.Y + Move1.Y))
    {
        Intersection->Y = Head2.Y;
    }
    else
    {
        return false;
    }

    return true;
}

int Distance(struct vec From, struct vec To)
{
    return abs(To.X - From.X) + abs(To.Y - From.Y);
}

int main(void)
{
    struct vec CentralPort = { 1, 1 };

    FILE* Input = fopen("day3_input.txt", "r");

    struct vec Cable1[1024];
    struct vec Cable2[1024];

    int Count1 = LoadCable(Input, Cable1);
    int Count2 = LoadCable(Input, Cable2);

    struct vec Intersections[1024];
    int IntersectionCount;

    int MinDistance = INT_MAX;

    struct vec Head1 = CentralPort;
    for(int i = 0; i < Count1; ++i)
    {
        struct vec Head2 = CentralPort;
        for(int k = 0; k < Count2; ++k)
        {
            struct vec IntersectionPoint;
            if(Intersection(Head1, Cable1[i], Head2, Cable2[k], &IntersectionPoint))
            {
                int CurrentDistance = Distance(CentralPort, IntersectionPoint);
                if(MinDistance > CurrentDistance)
                {
                    MinDistance = CurrentDistance;
                }
            }
            Head2.X += Cable2[k].X;
            Head2.Y += Cable2[k].Y;

        }
        Head1.X += Cable1[i].X;
        Head1.Y += Cable1[i].Y;
    }

    printf("Result: %d\n", MinDistance);
}
