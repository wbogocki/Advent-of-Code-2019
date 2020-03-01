/*
    ** Advent of Code 2019 **
    ** Day 6 Part 1 **
    ** Wojciech Bogócki **
    ** 6 Dec 2019 **
*/

#include <stdio.h>
#include <string.h>

struct object
{
    char Parent[4];
    char Name[4];
    int Orbits;
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

    struct object MapData[2048] = {};
    int Size = 0;

    while(fscanf(InputFile, "%[A-Z0-9])%[A-Z0-9]\n", MapData[Size].Parent, MapData[Size].Name) != EOF)
    {
        ++Size;
    }

    for(int I = 0; I < Size; ++I)
    {
        struct object* Object = FindObject(MapData[I].Name, Size, MapData);
        while(Object)
        {
            ++Object->Orbits;
            Object = FindObject(Object->Parent, Size, MapData);
        }
    }

    int Orbits = 0;
    for(int I = 0; I < Size; ++I)
    {
        Orbits += MapData[I].Orbits;
    }

    printf("Result: %d\n", Orbits);
}
