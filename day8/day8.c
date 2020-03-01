/*
    ** Advent of Code 2019 **
    ** Day 8 Part 1 **
    ** Wojciech Bogócki **
    ** 8 Dec 2019 Taipei **
*/

#include <limits.h>
#include <stdio.h>

int main(void)
{
    int Width = 25;
    int Height = 6;
    int Image[1024*1024] = {0};
    int Size = 0;

    FILE* Input = fopen("day8_input.txt", "r");
    while(fscanf(Input, "%1d", &Image[Size]) != EOF)
    {
        ++Size;
    }

    int LayerSize = Width * Height;
    int LayerCount = Size / LayerSize;

    printf("Layer size: %d\n", LayerSize);
    printf("Layer count: %d\n", LayerCount);

    int FewestZeros = INT_MAX;
    int FewestZerosLayer = 0;
    for(int LayerIndex = 0; LayerIndex < LayerCount; ++LayerIndex)
    {
        int Zeros = 0;
        for(int PixelIndex = 0; PixelIndex < LayerSize; ++PixelIndex)
        {
            int Pixel = Image[LayerIndex * LayerSize + PixelIndex];
            if(Pixel == 0)
            {
                ++Zeros;
            }
        }
        if(Zeros < FewestZeros)
        {
            FewestZeros = Zeros;
            FewestZerosLayer = LayerIndex;
        }
    }

    printf("Layer with fewest 0 digits: %d\n", FewestZerosLayer);

    int Ones = 0;
    int Twos = 0;
    for(int PixelIndex = 0; PixelIndex < LayerSize; ++PixelIndex)
    {
        int Pixel = Image[FewestZerosLayer * LayerSize + PixelIndex];
        if(Pixel == 1) ++Ones;
        if(Pixel == 2) ++Twos;
    }

    printf("Number of 1 digits: %d\n", Ones);
    printf("Number of 2 digits: %d\n", Twos);

    printf("Result: %d\n", Ones * Twos);
}
