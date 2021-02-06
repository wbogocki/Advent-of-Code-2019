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
 * Array *array = array_create(40, 0);
 *
 * array_push(array, a);
 * array_push(array, b);
 * array_push(array, c);
 *
 * array_set(array, 1, c);
 *
 * array_pop(array, NULL);
 *
 * for (size_t i = 0; i < array_size(array); ++i)
 * {
 *     printf("%zu: %s\n", i, (char *)array_get(array, i));
 * }
 *
 * array_destroy(array);
 **/

struct Array
{
    void *data;
    size_t element_size;
    size_t size;
    size_t capacity;
};

typedef struct Array Array;

Array *array_create(size_t size, size_t element_size);
void array_destroy(Array *array);
void *array_set(Array *array, size_t index, void *element);
void *array_get(Array *array, size_t index);
void *array_push(Array *array, void *element);
void array_pop(Array *array, void *element);
size_t array_size(Array *array);
bool array_contains(Array *array, void *element);

#ifdef ARRAY_IMPL

void *array_slot(Array *array, size_t index)
{
    size_t offset = array->element_size * index;
    return (char *)array->data + offset;
}

Array *array_create(size_t element_size, size_t capacity)
{
    void *data = calloc(capacity, element_size);
    assert(data && "Array alloc failed!");

    Array *array = malloc(sizeof(Array));
    assert(data && "Array alloc failed!");

    *array = (Array){.data = data, .element_size = element_size, .size = 0, .capacity = capacity};

    return array;
}

void array_destroy(Array *array)
{
    free(array->data);
    free(array);
}

void *array_set(Array *array, size_t index, void *element)
{
    assert(index < array->size && "Out of bounds!");
    void *slot = array_slot(array, index);
    return memcpy(slot, element, array->element_size);
}

void *array_get(Array *array, size_t index)
{
    assert(index < array->size && "Out of bounds!");
    return array_slot(array, index);
}

void *array_push(Array *array, void *element)
{
    // Grow the array if needed

    if (array->size == array->capacity)
    {
        array->capacity = array->capacity > 0
                              ? array->capacity * 2
                              : 8;
        array->data = realloc(array->data, array->element_size * array->capacity);
        assert(array->data && "Array realloc failed!");
    }

    // Put the item at the end of the array

    void *slot = array_slot(array, array->size++);
    memcpy(slot, element, array->element_size);

    return slot;
}

void array_pop(Array *array, void *element)
{
    assert(array->size > 0 && "Empty array!");
    void *slot = array_slot(array, --array->size);
    if (element)
    {
        memcpy(element, slot, array->element_size);
    }
}

size_t array_size(Array *array)
{
    return array->size;
}

void array_qsort(Array *array, int (*cmp)(const void *, const void *))
{
    qsort(array->data, array->size, array->element_size, cmp);
}

bool array_contains(Array *array, void *element)
{
    for (size_t i = 0; i < array->size; ++i)
    {
        if (memcmp(array_get(array, i), element, array->element_size) == 0)
        {
            return true;
        }
    }

    return false;
}

void array_reverse(Array *array)
{
    if (array->size > 0)
    {
        size_t left = 0;
        size_t right = array->size - 1;

        void *temp = malloc(array->element_size);
        assert(temp);

        while (left < right)
        {
            memcpy(temp, array_get(array, left), array->element_size);
            memcpy(array_get(array, left++), array_get(array, right), array->element_size);
            memcpy(array_get(array, right--), temp, array->element_size);
        }

        free(temp);
    }
}

#endif

#endif