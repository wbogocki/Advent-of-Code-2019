/*
    ** Advent of Code 2019 **
    ** Day 12 Part 1 **
    ** Wojciech Bogócki **
    ** 12 Dec 2019 Taipei **
*/

#include <stdio.h>
#include <stdlib.h>

struct moon
{
    int Pos[3];
    int Vel[3];
    int Pot;
    int Kin;
    int Tot;
};

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

    for(int Tick = 0; Tick < 1000; ++Tick)
    {
        // // Print
        // printf("Tick %d\n", Tick);
        // for(int Index = 0; Index < Count; ++Index)
        // {
        //     printf("%d) Pos=(%3d,%3d,%3d) Vel=(%3d,%3d,%3d) Pot=%-3d Kin=%-3d Tot=%-4d\n", Index,
        //         Moons[Index].Pos[0], Moons[Index].Pos[1], Moons[Index].Pos[2],
        //         Moons[Index].Vel[0], Moons[Index].Vel[1], Moons[Index].Vel[2],
        //         Moons[Index].Pot, Moons[Index].Kin, Moons[Index].Tot);
        // }
        // getchar();

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

        // Total energy
        for(int Index = 0; Index < Count; ++Index)
        {
            Moons[Index].Pot = 0;
            Moons[Index].Kin = 0;
            for(int Axis = 0; Axis < 3; ++Axis)
            {
                Moons[Index].Pot += abs(Moons[Index].Pos[Axis]);
                Moons[Index].Kin += abs(Moons[Index].Vel[Axis]);
            }
            Moons[Index].Tot = Moons[Index].Pot * Moons[Index].Kin;
        }
    }

    int TotalEnergy = 0;
    for(int Index = 0; Index < Count; ++Index)
    {
        TotalEnergy += Moons[Index].Tot;
    }

    printf("Result: %d\n", TotalEnergy);
}