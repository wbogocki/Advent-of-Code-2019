/**
 * Challenge:   Advent of Code 2019
 * Day:         20
 * Part:        1
 * Author:      Wojciech Bogócki
 * Date:        13 Feb 2021 Taipei
 **/

#define ARRAY_IMPL
#define TABLE_IMPL

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "table.h"

//
// Vector
//

typedef struct
{
    size_t X;
    size_t Y;
} vec;

//
// GRID
//

#define TILES_MAX (16 * 1024)
#define PORTALS_MAX (26 * 26)

typedef struct portal portal;

struct portal
{
    char Label[3];
    vec Pos;
    portal *Other;
};

typedef struct
{
    char Tiles[TILES_MAX];
    size_t Width;
    size_t Height;
    portal Portals[PORTALS_MAX];
    size_t PortalCount;
} grid;

char GridTile(grid *Grid, size_t X, size_t Y)
{
    assert(X < Grid->Width);
    assert(Y < Grid->Height);
    return Grid->Tiles[Y * Grid->Width + X];
}

portal *GridPortal(grid *Grid, size_t X, size_t Y)
{
    for (size_t Index = 0; Index < Grid->PortalCount; ++Index)
    {
        portal *Portal = &Grid->Portals[Index];
        if (Portal->Pos.X == X && Portal->Pos.Y == Y)
        {
            return Portal;
        }
    }
    return NULL;
}

portal *GridPortalByLabel(grid *Grid, const char *Label)
{
    for (size_t Index = 0; Index < Grid->PortalCount; ++Index)
    {
        portal *Other = &Grid->Portals[Index];
        if (strcmp(Other->Label, Label) == 0)
        {
            return Other;
        }
    }
    return NULL;
}

void GridLoad(const char *Filename, grid *Grid)
{
    FILE *File = fopen(Filename, "r");
    assert(File);

    memset(Grid, 0, sizeof(Grid));

    size_t Head = 0;

    char Char = fgetc(File);
    while (Char != EOF)
    {
        if (Char == '\n')
        {
            Grid->Width = 0;
            ++Grid->Height;
        }
        else
        {
            assert(Head < TILES_MAX);
            Grid->Tiles[Head++] = Char;
            ++Grid->Width;
        }

        Char = fgetc(File);
    }
    ++Grid->Height;

    fclose(File);

    // Horizontal portal labels
    for (int Y = 0; Y < Grid->Height; ++Y)
    {
        for (int X = 0; X < Grid->Width - 1; ++X)
        {
            char Label[] = {
                GridTile(Grid, X, Y),
                GridTile(Grid, X + 1, Y),
                '\0',
            };

            if (Label[0] >= 'A' && Label[0] <= 'Z' &&
                Label[1] >= 'A' && Label[1] <= 'Z')
            {
                vec PortalPos;
                if (X + 2 < Grid->Width && GridTile(Grid, X + 2, Y) == '.')
                {
                    // Portal is on the right
                    PortalPos.X = X + 2;
                    PortalPos.Y = Y;
                }
                else
                {
                    // Portal is on the left
                    PortalPos.X = X - 1;
                    PortalPos.Y = Y;
                }

                portal *Other = GridPortalByLabel(Grid, Label);

                portal Portal = {
                    .Label = {Label[0], Label[1], '\0'},
                    .Pos = PortalPos,
                    .Other = Other,
                };

                assert(Grid->PortalCount < PORTALS_MAX);
                Grid->Portals[Grid->PortalCount++] = Portal;

                if (Other)
                {
                    Other->Other = &Grid->Portals[Grid->PortalCount - 1];
                }
            }
        }
    }

    // Vertical portal labels
    for (int Y = 0; Y < Grid->Height - 1; ++Y)
    {
        for (int X = 0; X < Grid->Width; ++X)
        {
            char Label[] = {
                GridTile(Grid, X, Y),
                GridTile(Grid, X, Y + 1),
                '\0',
            };

            if (Label[0] >= 'A' && Label[0] <= 'Z' &&
                Label[1] >= 'A' && Label[1] <= 'Z')
            {
                vec PortalPos;
                if (Y + 2 < Grid->Height && GridTile(Grid, X, Y + 2) == '.')
                {
                    // Portal is on the bottom
                    PortalPos.X = X;
                    PortalPos.Y = Y + 2;
                }
                else
                {
                    // Portal is on the top
                    PortalPos.X = X;
                    PortalPos.Y = Y - 1;
                }

                portal *Other = GridPortalByLabel(Grid, Label);

                portal Portal = {
                    .Label = {Label[0], Label[1], '\0'},
                    .Pos = PortalPos,
                    .Other = Other,
                };

                assert(Grid->PortalCount < PORTALS_MAX);
                Grid->Portals[Grid->PortalCount++] = Portal;

                if (Other)
                {
                    Other->Other = &Grid->Portals[Grid->PortalCount - 1];
                }
            }
        }
    }
}

void GridPrint(grid *Grid)
{
    for (size_t Y = 0; Y < Grid->Height; ++Y)
    {
        for (size_t X = 0; X < Grid->Width; ++X)
        {
            putchar(GridTile(Grid, X, Y));
        }
        putchar('\n');
    }
}

//
// PATH-FINDING
//

#define TABLE_SIZE 1024

typedef struct
{
    grid *Grid; // Unowned
    vec Pos;
} node;

char *AStarTableKey(node *Node)
{
    size_t BufSize = 16;
    char *Buf = calloc(BufSize, sizeof(char));
    snprintf(Buf, BufSize, "%zu %zu", Node->Pos.X, Node->Pos.Y);
    return Buf;
}

int AStarHeuristics(node *Node, vec Goal)
{
    return 0; // Because of portals, we can't guarantee a consistent heuristics
}

// The returned key needs to be freed
char *AStarNextNode(table *OpenSet, table *FScore)
{
    int BestScore = INT_MAX;
    char *BestNode = NULL;

    for (table_entry *Entry = TableNext(OpenSet, NULL);
         Entry;
         Entry = TableNext(OpenSet, Entry))
    {
        int Score = *(int *)TableGet(FScore, Entry->Key);
        if (Score < BestScore)
        {
            BestScore = Score;
            BestNode = Entry->Key;
        }
    }

    return strdup(BestNode);
}

bool AStarIsGoal(node *Node, vec Goal)
{
    return Node->Pos.X == Goal.X && Node->Pos.Y == Goal.Y;
}

array *AStarPathReconstruct(table *CameFrom, node *Current)
{
    array *TotalPath = ArrayCreate(sizeof(node), 0);
    assert(TotalPath);

    while (Current)
    {
        ArrayPush(TotalPath, Current);

        char *Key = AStarTableKey(Current);
        Current = TableGet(CameFrom, Key);
        free(Key);
    }

    ArrayPop(TotalPath, NULL); // Pop the starting node
    ArrayReverse(TotalPath);

    return TotalPath;
}

array *AStarNeighbors(node *Node, vec Goal)
{
    size_t X = Node->Pos.X;
    size_t Y = Node->Pos.Y;

    vec NeighborTiles[] = {
        {X, Y - 1}, // Top
        {X, Y + 1}, // Bottom
        {X - 1, Y}, // Left
        {X + 1, Y}, // Right
    };

    array *Out = ArrayCreate(sizeof(node), 4);

    for (size_t Index = 0; Index < 4; ++Index)
    {
        vec Pos = NeighborTiles[Index];

        if (Pos.X >= 0 && Pos.X < Node->Grid->Width &&
            Pos.Y >= 0 && Pos.Y < Node->Grid->Height)
        {
            char Tile = GridTile(Node->Grid, Pos.X, Pos.Y);

            if (Tile == '.')
            {
                node Neighbor = {.Grid = Node->Grid, .Pos = Pos};
                ArrayPush(Out, &Neighbor);
            }
        }
    }

    portal *Portal = GridPortal(Node->Grid, Node->Pos.X, Node->Pos.Y);
    if (Portal && Portal->Other)
    {
        node Neighbor = {
            .Grid = Node->Grid,
            .Pos = Portal->Other->Pos,
        };
        ArrayPush(Out, &Neighbor);
    }

    return Out;
}

array *AStar(node Start, vec Goal)
{
    char *Key = AStarTableKey(&Start);

    // Init open set
    table *OpenSet = TableCreate(TABLE_SIZE);
    TableSet(OpenSet, Key, &Start, sizeof(node));

    // Init come-from set
    table *CameFrom = TableCreate(TABLE_SIZE);

    // Init g-score set
    table *GScore = TableCreate(TABLE_SIZE);
    int Score = 0;
    TableSet(GScore, Key, &Score, sizeof(int));

    // Init f-score set
    table *FScore = TableCreate(TABLE_SIZE);
    int H = AStarHeuristics(&Start, Goal);
    TableSet(FScore, Key, &H, sizeof(int));

    free(Key);

    // Find the path
    array *Out = NULL;
    while (!TableEmpty(OpenSet))
    {
        char *CurrentKey = AStarNextNode(OpenSet, FScore);

        node Current = *(node *)TableGet(OpenSet, CurrentKey);

        if (AStarIsGoal(&Current, Goal))
        {
            free(CurrentKey);

            Out = AStarPathReconstruct(CameFrom, &Current);

            // Double check
            node *LastNode = ArrayGet(Out, ArraySize(Out) - 1);
            assert(LastNode->Pos.X == Goal.X && LastNode->Pos.Y == Goal.Y &&
                   "The path ends on the desired tile!");

            goto cleanup;
        }

        TableUnset(OpenSet, CurrentKey);

        array *Neighbors = AStarNeighbors(&Current, Goal);
        for (size_t i = 0; i < ArraySize(Neighbors); ++i)
        {
            node *Neighbor = ArrayGet(Neighbors, i);
            char *NeighborKey = AStarTableKey(Neighbor);

            int Infinity = INT_MAX / 2;
            int GScoreCurrent = *(int *)TableGetDefault(GScore, CurrentKey, &Infinity);
            int GScoreNeighbor = *(int *)TableGetDefault(GScore, NeighborKey, &Infinity);

            int Distance = 1;

            int TentativeGScore = GScoreCurrent + Distance;

            if (TentativeGScore < GScoreNeighbor)
            {
                int FScoreNeighbor = TentativeGScore + AStarHeuristics(Neighbor, Goal);

                TableSet(CameFrom, NeighborKey, &Current, sizeof(node));
                TableSet(GScore, NeighborKey, &TentativeGScore, sizeof(int));
                TableSet(FScore, NeighborKey, &FScoreNeighbor, sizeof(int));

                if (TableGet(OpenSet, NeighborKey) == NULL)
                {
                    TableSet(OpenSet, NeighborKey, Neighbor, sizeof(node));
                }
            }

            free(NeighborKey);
        }
        ArrayDestroy(Neighbors);

        free(CurrentKey);
    }

cleanup:
    TableDestroy(FScore);
    TableDestroy(GScore);
    TableDestroy(CameFrom);
    TableDestroy(OpenSet);

    return Out;
}

int main()
{
    grid *Grid = malloc(sizeof(grid));

    GridLoad("input.txt", Grid);
    GridPrint(Grid);

    node Start = {
        .Grid = Grid,
        .Pos = GridPortalByLabel(Grid, "AA")->Pos,
    };

    vec Goal = GridPortalByLabel(Grid, "ZZ")->Pos;

    array *Path = AStar(Start, Goal);

    printf("%zu,%zu", Start.Pos.X, Start.Pos.Y);
    for (int Index = 0; Index < ArraySize(Path); ++Index)
    {
        node *Node = ArrayGet(Path, Index);
        printf("-> %zu,%zu", Node->Pos.X, Node->Pos.Y);
    }
    printf("\n");

    printf("Path length: %zu\n", ArraySize(Path));
}
