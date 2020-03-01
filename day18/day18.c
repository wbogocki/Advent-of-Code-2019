/*
    ** Advent of Code 2019 **
    ** Day 18 Part 1 **
    ** Wojciech Bogócki **
    ** 21 Dec 2019 Taipei **
*/

#include <stdio.h>
#include <string.h>

int
main(void)
{
    char Map[128][128] = {{0}};
    int Width = 0;
    int Height = 0;

    FILE* Input = fopen("day18_input.txt", "r");
    for(;;)
    {
        char Char = fgetc(Input);
        if(Char == EOF)
        {
            break;
        }
        else if(Char == '\n')
        {
            Width = 0;
            ++Height;
        }
        else
        {
            Map[Height][Width++] = Char;
        }
    }
    Width = strlen(Map[0]);

    // shortest distance from each key to each other key
    // doors needed before a key is unlocked
    // check if there is more passageways that omit some doors?
}