/*
    ** Advent of Code 2019 **
    ** Day 14 Part 1 **
    ** Wojciech Bogócki **
    ** 16 Dec 2019 Taipei **
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHEMICALS 1024
#define ORE 0
#define FUEL 1

enum parser_mode
{
    I_SOAR, // input size or arrow
    I_NAME,
    O_SIZE,
    O_NAME
};

struct reaction
{
    int Inputs[MAX_CHEMICALS];
    int OutputChemical;
    int OutputQuantity;
};

int
GetID(const char* Name, int* Count, char Names[][16])
{
    for(int Index = 0; Index < *Count; ++Index)
    {
        if(strcmp(Names[Index], Name) == 0)
        {
            return Index;
        }
    }
    strcpy(Names[*Count], Name);
    return (*Count)++;
}

int
GetReaction(int Chemical, int Count, struct reaction Reactions[])
{
    for(int Index = 0; Index < Count; ++Index)
    {
        if(Reactions[Index].OutputChemical == Chemical)
        {
            return Index;
        }
    }
    return -1;
}

void
PrintReaction(struct reaction* Reaction, int Count, char Names[][16])
{
    bool FirstInput = true;
    for(int Index = 0;
        Index < Count;
        ++Index)
    {
        if(Reaction->Inputs[Index] != 0)
        {
            if (!FirstInput)
            {
                printf(", ");
            }
            else
            {
                FirstInput = false;
            }
            printf("%d %s", Reaction->Inputs[Index], Names[Index]);
        }
    }
    printf(" => %d %s\n", Reaction->OutputQuantity, Names[Reaction->OutputChemical]);
}

int
GetCost(int Chemical, int Quantity, int ChemicalCount, char Names[][16], int ReactionCount, struct reaction Reactions[])
{
    int Stash[MAX_CHEMICALS] = {0};
    Stash[Chemical] = -Quantity;

    bool Done = false;
    while(!Done)
    {
        Done = true;
        for(int MissingChemical = 0;
            MissingChemical < ChemicalCount;
            ++MissingChemical)
        {
            // Chemical is an ORE or not missing
            if(MissingChemical == ORE || Stash[MissingChemical] >= 0)
            {
                continue;
            }

            // Chemical is still missing
            Done = false;

            // Reaction to use
            int ReactionIndex = GetReaction(MissingChemical, ReactionCount, Reactions);
            if(ReactionIndex == -1)
            {
                printf("No reaction available for %s\n", Names[MissingChemical]);
                exit(1);
            }

            // TODO: The loop is inefficient, it's better to compute the number of times a reaction needs to happen then multiply

            // React while chemical is missing
            while(Stash[MissingChemical] < 0)
            {
                //PrintReaction(&Reactions[ReactionIndex], ChemicalCount, Names);

                Stash[MissingChemical] += Reactions[ReactionIndex].OutputQuantity;
                for(int InputChemical = 0;
                    InputChemical < ChemicalCount;
                    ++InputChemical)
                {
                    Stash[InputChemical] -= Reactions[ReactionIndex].Inputs[InputChemical];
                }
            }
        }
    }

    return -Stash[ORE];
}

int
main(void)
{
    char Names[MAX_CHEMICALS][16] = {"ORE", "FUEL"};
    int Count = 2;
    struct reaction Reactions[MAX_CHEMICALS] = {0};
    int ReactionCount = 0;

    // Read input
    {
        FILE* Input = fopen("day14_input.txt", "r");
        enum parser_mode ParserMode = I_SOAR;
        char Token[16] = {0};
        int Quantity = 0;
        struct reaction Reaction = {0};
        while(fscanf(Input, "%s", Token) != EOF)
        {
            switch(ParserMode)
            {
                case I_SOAR:
                {
                    if(strcmp(Token, "=>") == 0)
                    {
                        ParserMode = O_SIZE;
                    }
                    else
                    {
                        Quantity = atoi(Token);
                        ParserMode = I_NAME;
                    }
                    break;
                }
                case I_NAME:
                {
                    int Length = strlen(Token);
                    if(Token[Length-1] == ',')
                    {
                        Token[--Length] = '\0';
                    }
                    int ID = GetID(Token, &Count, Names);
                    Reaction.Inputs[ID] = Quantity;
                    ParserMode = I_SOAR;
                    break;
                }
                case O_SIZE:
                {
                    Reaction.OutputQuantity = atoi(Token);
                    ParserMode = O_NAME;
                    break;
                }
                case O_NAME:
                {
                    Reaction.OutputChemical = GetID(Token, &Count, Names);

                    Reactions[ReactionCount++] = Reaction;
                    Reaction = (struct reaction) {0};

                    ParserMode = I_SOAR;
                    break;
                }
            }
        }
        fclose(Input);
    }

    // for(int A = 0; A < ReactionCount; ++A)
    // {
    //     for(int B = 0; B < ReactionCount; ++B)
    //     {
    //         if(A != B && Reactions[A].OutputChemical == Reactions[B].OutputChemical)
    //         {
    //             printf("Reactions don't have unique results!\n");
    //             exit(1);
    //         }
    //     }
    // }
    // printf("Reactions are unique!\n");


    // for(int ReactionIndex = 0;
    //     ReactionIndex < ReactionCount;
    //     ++ReactionIndex)
    // {
    //     PrintReaction(&Reactions[ReactionIndex], Count, Names);
    // }

    int Cost = GetCost(FUEL, 1, Count, Names, ReactionCount, Reactions);
    printf("Result: %d\n", Cost);
}