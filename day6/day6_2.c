/*
    ** Advent of Code 2019 **
    ** Day 6 Part 2 **
    ** Wojciech Bogócki **
    ** 6 Dec 2019 **
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

struct object
{
    char Parent[4];
    char Name[4];
    int DistanceFromYOU;
    int DistanceFromSAN;
};

struct object* FindObject(char Name[], int Size, struct object MapData[])
{
    for(int I = 0; I < Size; ++I)
    {
        if(strcmp(MapData[I].Name, Name) == 0)
        {
            return &MapData[I];
        }
    }
    return NULL;
}

int main(void)
{
    FILE* InputFile = fopen("day6_input.txt", "r");

    struct object MapData[2048] = {0};
    int Size = 0;
    
    while(fscanf(InputFile, "%[A-Z0-9])%[A-Z0-9]\n", MapData[Size].Parent, MapData[Size].Name) != EOF)
    {
        MapData[Size].DistanceFromYOU = 9999;
        MapData[Size].DistanceFromSAN = 9999;
        ++Size;
    }

    struct object* YOU = FindObject("YOU", Size, MapData);
    int DistanceFromYOU = 0;
    struct object* IterYOU = FindObject(YOU->Parent, Size, MapData);
    while(IterYOU)
    {
        IterYOU->DistanceFromYOU = DistanceFromYOU++;
        IterYOU = FindObject(IterYOU->Parent, Size, MapData);
    }

    struct object* SAN = FindObject("SAN", Size, MapData);
    int DistanceFromSAN = 0;
    struct object* IterSAN = FindObject(SAN->Parent, Size, MapData);
    while(IterSAN)
    {
        IterSAN->DistanceFromSAN = DistanceFromSAN++;
        IterSAN = FindObject(IterSAN->Parent, Size, MapData);
    }

    int MinimumDistance = 9999;
    for(int I = 0; I < Size; ++I)
    {
        int Distance = MapData[I].DistanceFromSAN + MapData[I].DistanceFromYOU;
        if(Distance < MinimumDistance)
        {
            MinimumDistance = Distance;
        }
    }

    printf("Result: %d\n", MinimumDistance);
}
