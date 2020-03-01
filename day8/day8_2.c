/*
    ** Advent of Code 2019 **
    ** Day 8 Part 2 **
    ** Wojciech Bogócki **
    ** 8 Dec 2019 Taipei **
*/

#include <stdio.h>
#include <string.h>

#define WIDTH 25
#define HEIGHT 6

enum color
{
    WHITE = 0,
    BLACK = 1,
    TRANSPARENT = 2
};

int main(void)
{
    int Image[1024*1024] = {0};
    int Size = 0;

    FILE* Input = fopen("day8_input.txt", "r");
    while(fscanf(Input, "%1d", &Image[Size]) != EOF)
    {
        ++Size;
    }

    int LayerSize = WIDTH * HEIGHT;
    int LayerCount = Size / LayerSize;

    int DecodedImage[LayerSize];
    for(int Pixel = 0; Pixel < LayerSize; ++Pixel)
    {
        DecodedImage[Pixel] = TRANSPARENT;
    }

    for(int Layer = 0; Layer < LayerCount; ++Layer)
    {
        for(int Pixel = 0; Pixel < LayerSize; ++Pixel)
        {
            if(DecodedImage[Pixel] == TRANSPARENT)
            {
                DecodedImage[Pixel] = Image[Layer * LayerSize + Pixel];
            }
        }
    }

    for(int Pixel = 0; Pixel < LayerSize; ++Pixel)
    {
        if(Pixel != 0 && Pixel % WIDTH == 0)
        {
            putchar('\n');
        }

        if(DecodedImage[Pixel] == BLACK)
        {
            putchar('#');
        }
        else
        {
            putchar('.');
        }
    }
    printf("\n");
}
