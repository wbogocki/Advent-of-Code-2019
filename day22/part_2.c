#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
    int64_t N;
} shuffle;

size_t LoadShuffleProcess(const char *Filename, shuffle Process[SHUFFLE_MAX])
{
    FILE *File = fopen("input.txt", "r");
    size_t Count = 0;

    char Line[40] = {0};
    while (fgets(Line, sizeof(Line), File))
    {
        // printf("Line: %s", Line);

        assert(Count < SHUFFLE_MAX && "Shuffle process length exceeds SHUFFLE_MAX");

        int64_t N = 0;
        if (strncmp(Line, "deal into new stack", strlen("deal into new stack")) == 0)
        {
            Process[Count++] = (shuffle){.Technique = ST_DEAL_INTO_NEW_STACK, .N = 0};
        }
        else if (sscanf(Line, "cut %" PRId64, &N) == 1)
        {
            Process[Count++] = (shuffle){.Technique = ST_CUT_N_CARDS, .N = N};
        }
        else if (sscanf(Line, "deal with increment %" PRId64, &N) == 1)
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

// Take the output index of a card and return the index it started at before the shuffle process
int64_t Unshuffle(int64_t Card, int64_t DeckSize, shuffle Process[SHUFFLE_MAX], size_t ProcessCount)
{
    int64_t Out = Card;
    for (int64_t Index = ProcessCount - 1; Index >= 0; --Index)
    {
        shuffle Shuffle = Process[Index];

        // printf("%zu\t%-25s\t%" PRId64 "\n", Index, SHUFFLE_TECHNIQUE_NAMES[Shuffle.Technique], Shuffle.N);

        switch (Shuffle.Technique)
        {
        case ST_DEAL_INTO_NEW_STACK:
        {
            Out = DeckSize - 1 - Card; // Mirror the card to the other side of the deck
            break;
        }
        case ST_CUT_N_CARDS:
        {
            int64_t N = Shuffle.N;
            if (N < 0)
            {
                N += DeckSize;
            }
            Out = (Out + N) % DeckSize;
            break;
        }
        case ST_DEAL_WITH_INCREMENT_N:
        {
            /*
                0 1 2 3 4 5 6 7 8 9
                0 . . . . . . 1 . .
                0 . . . 2 . . 1 . .
                0 3 . . 2 . . 1 . .
                0 3 . . 2 . . 1 4 .
                0 3 . . 2 5 . 1 4 .
                0 3 6 . 2 5 . 1 4 7
                0 3 6 . 2 5 8 1 4 7
                0 3 6 9 2 5 8 1 4 7

                Rounds:
                0 3 5 7 2 4 6 1 3 5
            */

            int64_t Round = Out % (DeckSize % Shuffle.N);

            printf("%llu ", Round);

            // Offset to the first element in the round in the source deck
            int64_t Offset = Round * Shuffle.N;

            // Index of the element in the round after offset
            int64_t Index = (Out + Round) / Shuffle.N;

            Out = Offset + Index;

            /*
            // One round lasts until we wrap around the source deck
            int Round = (DeckSize - 1 - Out) % Shuffle.N; // <--------------- BROKEN

            // Offset to the first element in the round in the source deck
            int Offset = Round * Shuffle.N;

            // Index of the element in the round after offset
            int Index = (Out + Round) / Shuffle.N;

            Out = Offset + Index;
            */
            break;
        }
        default:
        {
            fprintf(stderr, "Invalid shuffle technique");
            exit(1);
        }
        }
    }
    return Out;
}

int main(void)
{
    // int64_t Card = 2020;
    int64_t DeckSize = 10;

    shuffle ShuffleProcess[SHUFFLE_MAX] = {0};
    size_t ShuffleProcessCount = LoadShuffleProcess("input.txt", ShuffleProcess);

    int64_t Deck[] = {0, 3, 6, 9, 2, 5, 8, 1, 4, 7};

    for (int64_t Card = 0; Card < DeckSize; ++Card)
    {
        int64_t Result = Unshuffle(Card, DeckSize, ShuffleProcess, ShuffleProcessCount);

        // printf("%" PRId64 " ", Result);
    }
    putchar('\n');
}
