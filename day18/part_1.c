/**
 * Challenge:   Advent of Code 2019
 * Day:         18
 * Part:        1
 * Author:      Wojciech Bogócki
 * Take 1:      21 Dec 2019 Taipei
 * Take 2:      13 Mar 2020 Taipei
 * Take 3:      26 Dec 2020 Taipei
 * Take 4:      01 Feb 2021 Taipei
 **/

#define ARRAY_IMPL
#define TABLE_IMPL

#include "array.h"
#include "table.h"
#include <stdio.h>

int main()
{
    char a[40] = "Hello";
    char b[40] = "There";
    char c[40] = "Stream";

    Array *array = array_create(40, 0);

    array_push(array, a);
    array_push(array, b);
    array_push(array, c);

    array_set(array, 1, c);

    array_pop(array, NULL);

    for (size_t i = 0; i < array_size(array); ++i)
    {
        printf("%zu: %s\n", i, (char *)array_get(array, i));
    }

    array_destroy(array);
}
