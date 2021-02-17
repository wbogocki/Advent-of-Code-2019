#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DECK_SIZE 10007
#define SHUFFLE_MAX 100

typedef enum
{
    ST_DEAL_INTO_NEW_STACK,
    ST_CUT_N_CARDS,
    ST_DEAL_WITH_INCREMENT_N,
} shuffle_technique;

const char *SHUFFLE_TECHNIQUE_NAMES[] = {
    [ST_DEAL_INTO_NEW_STACK] = "ST_DEAL_INTO_NEW_STACK",
    [ST_CUT_N_CARDS] = "ST_CUT_N_CARDS",
    [ST_DEAL_WITH_INCREMENT_N] = "ST_DEAL_WITH_INCREMENT_N",
};

typedef struct
{
    shuffle_technique Technique;
    int N;
} shuffle;

void Copy(int Destination[DECK_SIZE], int Source[DECK_SIZE])
{
    memcpy(Destination, Source, sizeof(int) * DECK_SIZE);
}

void DealIntoNewStack(int Deck[DECK_SIZE])
{
    for (int Index = 0; Index < DECK_SIZE / 2; ++Index)
    {
        int Swap = Deck[Index];
        Deck[Index] = Deck[DECK_SIZE - Index - 1];
        Deck[DECK_SIZE - Index - 1] = Swap;
    }
}

void CutNCards(int Deck[DECK_SIZE], int N)
{
    int Reference[DECK_SIZE] = {0};
    Copy(Reference, Deck);

    N %= DECK_SIZE;

    // Invert negative N
    if (N < 0)
    {
        N += DECK_SIZE;
    }

    // The first N
    for (int Index = 0; Index < N; ++Index)
    {
        Deck[DECK_SIZE - N + Index] = Reference[Index];
    }

    // The remainder
    for (int Index = N; Index < DECK_SIZE; ++Index)
    {
        Deck[Index - N] = Reference[Index];
    }
}

void DealWithIncrementN(int Deck[DECK_SIZE], int N)
{
    int Reference[DECK_SIZE] = {0};
    Copy(Reference, Deck);

    int SetIndex = 0;
    for (int GetIndex = 0; GetIndex < DECK_SIZE; ++GetIndex)
    {
        Deck[SetIndex] = Reference[GetIndex];
        SetIndex = (SetIndex + N) % DECK_SIZE;
    }
}

size_t LoadShuffleProcess(const char *Filename, shuffle Process[SHUFFLE_MAX])
{
    FILE *File = fopen("input.txt", "r");
    size_t Count = 0;

    char Line[40] = {0};
    while (fgets(Line, sizeof(Line), File))
    {
        // printf("Line: %s", Line);

        assert(Count < SHUFFLE_MAX && "Shuffle process length exceeds SHUFFLE_MAX");

        int N = 0;
        if (strncmp(Line, "deal into new stack", strlen("deal into new stack")) == 0)
        {
            Process[Count++] = (shuffle){.Technique = ST_DEAL_INTO_NEW_STACK, .N = 0};
        }
        else if (sscanf(Line, "cut %d", &N) == 1)
        {
            Process[Count++] = (shuffle){.Technique = ST_CUT_N_CARDS, .N = N};
        }
        else if (sscanf(Line, "deal with increment %d", &N) == 1)
        {
            Process[Count++] = (shuffle){.Technique = ST_DEAL_WITH_INCREMENT_N, .N = N};
        }
        else
        {
            fprintf(stderr, "Invalid shuffle technique: %s", Line);
            exit(1);
        }
    }
    // putchar('\n');

    fclose(File);
    return Count;
}

void InitDeck(int Deck[DECK_SIZE])
{
    for (int Index = 0; Index < DECK_SIZE; ++Index)
    {
        Deck[Index] = Index;
    }
}

void PrintDeck(int Deck[DECK_SIZE])
{
    for (int Index = 0; Index < DECK_SIZE; ++Index)
    {
        printf("%d", Deck[Index]);
        if (Index != DECK_SIZE - 1)
        {
            putchar(' ');
        }
    }
    putchar('\n');
}

int main(void)
{
    int Deck[DECK_SIZE] = {0};
    InitDeck(Deck);

    shuffle ShuffleProcess[SHUFFLE_MAX] = {0};
    size_t ShuffleProcessCount = LoadShuffleProcess("input.txt", ShuffleProcess);

    for (size_t Index = 0; Index < ShuffleProcessCount; ++Index)
    {
        shuffle Shuffle = ShuffleProcess[Index];

        printf("%d\t%-25s\t%d\t", Index, SHUFFLE_TECHNIQUE_NAMES[Shuffle.Technique], Shuffle.N);
        // PrintDeck(Deck);
        printf("\n");

        switch (Shuffle.Technique)
        {
        case ST_DEAL_INTO_NEW_STACK:
        {
            DealIntoNewStack(Deck);
            break;
        }
        case ST_CUT_N_CARDS:
        {
            CutNCards(Deck, Shuffle.N);
            break;
        }
        case ST_DEAL_WITH_INCREMENT_N:
        {
            DealWithIncrementN(Deck, Shuffle.N);
            break;
        }
        default:
        {
            fprintf(stderr, "Invalid shuffle technique");
            exit(1);
        }
        }
    }

    // printf("Result: ");
    // PrintDeck(Deck);

    int IndexOf2019 = 0;

    for (int Index = 0; Index < DECK_SIZE; ++Index)
    {
        if (Deck[Index] == 2019)
        {
            IndexOf2019 = Index;
            break;
        }
    }

    printf("Position of card 2019: %d", IndexOf2019);
}
