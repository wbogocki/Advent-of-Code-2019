/*
    ** Advent of Code 2019 **
    ** Day 14 Part 2 **
    ** Wojciech Bogócki **
    ** 16 Dec 2019 Taipei **
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

struct reaction*
GetReaction(int Chemical, int Count, struct reaction Reactions[])
{
    for(int Index = 0; Index < Count; ++Index)
    {
        if(Reactions[Index].OutputChemical == Chemical)
        {
            return &Reactions[Index];
        }
    }
    return NULL;
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

long long
GetCost(int Chemical, long long Quantity, int ChemicalCount, char Names[][16], int ReactionCount, struct reaction Reactions[])
{
    long long Stash[MAX_CHEMICALS] = {0};
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
            struct reaction* Reaction = GetReaction(MissingChemical, ReactionCount, Reactions);
            if(!Reaction)
            {
                printf("No reaction available for %s\n", Names[MissingChemical]);
                exit(1);
            }

            // Number of reactions to perform
            long long ReactionNumber = -Stash[MissingChemical] / Reaction->OutputQuantity;
            if(-Stash[MissingChemical] % Reaction->OutputQuantity != 0)
            {
                ++ReactionNumber;
            }

            // Perform the reactions
            Stash[MissingChemical] += Reaction->OutputQuantity * ReactionNumber;
            for(int InputChemical = 0;
                InputChemical < ChemicalCount;
                ++InputChemical)
            {
                Stash[InputChemical] -= Reaction->Inputs[InputChemical] * ReactionNumber;
            }
        }
    }

    return -Stash[ORE];
}

static inline long long
Max(long long A, long long B)
{
    return A > B ? A : B;
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

    long long TargetCost = 1000000000000;
    long long Guess = 1;
    long double GuessVelocity = 1.0;
    for(;;)
    {
        long long Cost = GetCost(FUEL, Guess, Count, Names, ReactionCount, Reactions);
        long long CostAtGuessPlusOne = GetCost(FUEL, Guess+1, Count, Names, ReactionCount, Reactions);
        
        printf("G=%lld, C(G)=%lld, C(G+1)=%lld, V=%Lf\n", Guess, Cost, CostAtGuessPlusOne, GuessVelocity);

        if(Cost <= TargetCost && CostAtGuessPlusOne > TargetCost)
        {
            break;
        }
        else
        {
            if(Cost > TargetCost)
            {
                if(GuessVelocity > 0)
                {
                    GuessVelocity /= -2.0;
                }
                else
                {
                    GuessVelocity -= Max(1, GuessVelocity * 1.1);
                }
            }
            else
            {
                if(GuessVelocity > 0)
                {
                    GuessVelocity += Max(1, GuessVelocity * 1.1);
                }
                else
                {
                    GuessVelocity /= -2.0;
                }
            }
            
            Guess = Max(Guess + GuessVelocity, 1);
        }
    }

    printf("Result: %lld\n", Guess);
}