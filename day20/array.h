#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * Usage:
 *
 * char a[40] = "Hello";
 * char b[40] = "There";
 * char c[40] = "Stream";
 *
 * array *Array = ArrayCreate(40, 0);
 *
 * ArrayPush(Array, a);
 * ArrayPush(Array, b);
 * ArrayPush(Array, c);
 *
 * ArraySet(Array, 1, c);
 *
 * ArrayPop(Array, NULL);
 *
 * for (size_t i = 0; i < ArraySize(Array); ++i)
 * {
 *     printf("%zu: %s\n", i, (char *)ArrayGet(Array, i));
 * }
 *
 * ArrayDestroy(Array);
 **/

struct array
{
    void *Data;
    size_t ElementSize;
    size_t Size;
    size_t Capacity;
};

typedef struct array array;

array *ArrayCreate(size_t Size, size_t ElementSize);
void ArrayDestroy(array *Array);
void *ArraySet(array *Array, size_t Index, void *Element);
void *ArrayGet(array *Array, size_t Index);
void *ArrayPush(array *Array, void *Element);
void ArrayPop(array *Array, void *Element);
size_t ArraySize(array *Array);
void ArrayQSort(array *Array, int (*Compare)(const void *, const void *));
bool ArrayContains(array *Array, void *Element);
void ArrayReverse(array *Array);

#ifdef ARRAY_IMPL
#undef ARRAY_IMPL

void *ArraySlot(array *Array, size_t Index)
{
    size_t Offset = Array->ElementSize * Index;
    return (char *)Array->Data + Offset;
}

array *ArrayCreate(size_t ElementSize, size_t Capacity)
{
    void *Data = calloc(Capacity, ElementSize);
    assert(Data && "Array alloc failed!");

    array *Array = malloc(sizeof(array));
    assert(Data && "Array alloc failed!");

    *Array = (array){.Data = Data, .ElementSize = ElementSize, .Size = 0, .Capacity = Capacity};

    return Array;
}

void ArrayDestroy(array *Array)
{
    free(Array->Data);
    free(Array);
}

void *ArraySet(array *Array, size_t Index, void *Element)
{
    assert(Index < Array->Size && "Out of bounds!");
    void *Slot = ArraySlot(Array, Index);
    return memcpy(Slot, Element, Array->ElementSize);
}

void *ArrayGet(array *Array, size_t Index)
{
    assert(Index < Array->Size && "Out of bounds!");
    return ArraySlot(Array, Index);
}

void *ArrayPush(array *Array, void *Element)
{
    // Grow the array if needed

    if (Array->Size == Array->Capacity)
    {
        Array->Capacity = Array->Capacity > 0
                              ? Array->Capacity * 2
                              : 8;
        Array->Data = realloc(Array->Data, Array->ElementSize * Array->Capacity);
        assert(Array->Data && "Array realloc failed!");
    }

    // Put the item at the end of the array

    void *Slot = ArraySlot(Array, Array->Size++);
    memcpy(Slot, Element, Array->ElementSize);

    return Slot;
}

void ArrayPop(array *Array, void *Element)
{
    assert(Array->Size > 0 && "Empty array!");
    void *Slot = ArraySlot(Array, --Array->Size);
    if (Element)
    {
        memcpy(Element, Slot, Array->ElementSize);
    }
}

size_t ArraySize(array *Array)
{
    return Array->Size;
}

void ArrayQSort(array *Array, int (*Compare)(const void *, const void *))
{
    qsort(Array->Data, Array->Size, Array->ElementSize, Compare);
}

bool ArrayContains(array *Array, void *Element)
{
    for (size_t i = 0; i < Array->Size; ++i)
    {
        if (memcmp(ArrayGet(Array, i), Element, Array->ElementSize) == 0)
        {
            return true;
        }
    }

    return false;
}

void ArrayReverse(array *Array)
{
    if (Array->Size > 0)
    {
        size_t Left = 0;
        size_t Right = Array->Size - 1;

        void *Temp = malloc(Array->ElementSize);
        assert(Temp);

        while (Left < Right)
        {
            memcpy(Temp, ArrayGet(Array, Left), Array->ElementSize);
            memcpy(ArrayGet(Array, Left++), ArrayGet(Array, Right), Array->ElementSize);
            memcpy(ArrayGet(Array, Right--), Temp, Array->ElementSize);
        }

        free(Temp);
    }
}

#endif

#endif