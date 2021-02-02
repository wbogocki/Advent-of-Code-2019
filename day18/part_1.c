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

#define TABLE_IMPL

#include "table.h"
#include <stdio.h>

int main()
{
    Table *table = table_create(64);

    table_set(table, "a1", "1", 2);
    table_set(table, "b2", "2", 2);
    table_set(table, "c3", "3", 2);
    table_set(table, "d4", "4", 2);
    table_set(table, "e5", "5", 2);
    table_set(table, "f6", "6", 2);
    table_set(table, "g6", "7", 2);
    table_set(table, "h7", "8", 2);
    table_set(table, "i8", "9", 2);

    for (size_t i = 0; i < table->size; ++i)
    {
        TableEntry *entry = table->entries[i];
        while (entry)
        {
            printf("%zu: %s -> %s\n", i, entry->key, (char *)entry->value);
            entry = entry->next;
        }
    }

    printf("-----\n");

    for (TableEntry *entry = table_next(table, NULL);
         entry;
         entry = table_next(table, entry))
    {
        printf("%s: %s\n", entry->key, (char *)entry->value);
    }
}
