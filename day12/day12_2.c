/*
    ** Advent of Code 2019 **
    ** Day 12 Part 2 **
    ** Wojciech Bogócki **
    ** 12 Dec 2019 Taipei **
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct moon
{
    int Pos[3];
    int Vel[3];
};

void
Print(int Count, struct moon Moons[])
{
    for(int Index = 0; Index < Count; ++Index)
    {
        printf("%d) Pos=(%3d,%3d,%3d) Vel=(%3d,%3d,%3d)\n", Index,
            Moons[Index].Pos[0], Moons[Index].Pos[1], Moons[Index].Pos[2],
            Moons[Index].Vel[0], Moons[Index].Vel[1], Moons[Index].Vel[2]);
    }
}

long long
GCD(long long A, long long B)
{
    /*
        ** Stein's algorithm **
        Input: a, b positive integers
        Output: g and d such that g is odd and gcd(a, b) = g × 2^d
        d := 0
        while a and b are both even
            a := a/2
            b := b/2
            d := d + 1
        while a ≠ b
            if a is even then a := a/2
            else if b is even then b := b/2
            else if a > b then a := (a – b)/2
            else b := (b – a)/2
        g := a
        output g, d
    */
    long long D = 0;
    while((A % 2 == 0) && (B % 2 == 0))
    {
        A /= 2;
        B /= 2;
        ++D;
    }
    while(A != B)
    {
        if(A % 2 == 0)
        {
            A /= 2;
        }
        else if(B % 2 == 0)
        {
            B /= 2;
        }
        else if(A > B)
        {
            A = (A - B) / 2;
        }
        else
        {
            B = (B - A) / 2;
        }
    }
    return A * (1 << D);
}

long long
LCM(long long A, long long B)
{
    return (A * B) / GCD(A, B);
}

int
main(void)
{
    struct moon Moons[128] = {0};
    int Count = 0;

    FILE* Input = fopen("day12_input.txt", "r");
    while(fscanf(Input, "<x=%d, y=%d, z=%d>\n", &Moons[Count].Pos[0], &Moons[Count].Pos[1], &Moons[Count].Pos[2]) != EOF)
    {
        ++Count;
    }

    struct moon InitialState[128] = {0};
    memcpy(&InitialState, &Moons, sizeof(struct moon) * Count);

    long long OrbitTimes[3] = {-1,-1,-1};

    long long Tick = 0;
    while(OrbitTimes[0] == -1 || OrbitTimes[1] == -1 || OrbitTimes[2] == -1)
    {
        //printf("Tick %lld\n", Tick);

        // Compare with initial positions
        if(Tick != 0)
        {
            // TODO: This way of doing things sucks for cache
            for(int Axis = 0; Axis < 3; ++Axis)
            {
                if(OrbitTimes[Axis] == -1)
                {
                    bool Good = true;
                    for(int Index = 0; Index < Count; ++Index)
                    {
                        if(Moons[Index].Pos[Axis] != InitialState[Index].Pos[Axis])
                        {
                            Good = false;
                            break;
                        }
                    }
                    if(Good)
                    {
                        OrbitTimes[Axis] = Tick + 1;
                    }
                }
            }
        }

        // Gravity
        for(int A = 0; A < Count; ++A)
        {
            for(int B = 0; B < Count; ++B)
            {
                for(int Axis = 0; Axis < 3; ++Axis)
                {
                    if(Moons[A].Pos[Axis] < Moons[B].Pos[Axis])
                    {
                        ++Moons[A].Vel[Axis];
                        --Moons[B].Vel[Axis];
                    }
                }
            }
        }

        // Velocity
        for(int Index = 0; Index < Count; ++Index)
        {
            for(int Axis = 0; Axis < 3; ++Axis)
            {
                Moons[Index].Pos[Axis] += Moons[Index].Vel[Axis];
            }
        }

        ++Tick;
    }

    printf("Orbit times: %4lld, %lld, %lld\n", OrbitTimes[0], OrbitTimes[1], OrbitTimes[2]);

    long long CommonOrbitTime = LCM(OrbitTimes[0], LCM(OrbitTimes[1], OrbitTimes[2]));

    printf("Result: %lld\n", CommonOrbitTime);
}