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

#include <stdio.h>

//
// TABLES
//

typedef struct TableKey
{
    const char *string;
} TableKey;

typedef struct TableValue
{
    void *value;
} TableValue;

/**
 * A table is both a dynamic array / vector, a dictionary, and a set.
 *
 * Examples:
 *
 * Table *my_table = table_create();
 *
 * table_insert(my_table, "Value 1", some_value);
 * table_insert(my_table, 0, some_value);
 * table_insert(my_table, TABLE_END, some_value);
 *
 * table_remove(my_table, "Value 1");
 * table_remove(my_table, 0);
 * table_remove(my_table, TABLE_END);
 *
 * SomeValue *value = table_get(my_table, "Value 1", SomeValue);
 * SomeValue *value = table_get(my_table, 0, SomeValue);
 * SomeValue *value = table_get(my_table, TABLE_END, SomeValue);
 *
 * table_set(my_table, "Value 1", some_value);
 * table_set(my_table, 0, some_value);
 * table_set(my_table, TABLE_END, some_value);
 *
 * for (size_t index = 0; index < table_size(my_table); ++index)
 * {
 *     SomeValue *value = table_get(my_table, index, SomeValue);
 * }
 *
 * for (size_t index = 0; index < table_size(my_table); ++index)
 * {
 *     SomeValue *value = table_get(my_table, table_key(index), SomeValue);
 * }
 **/
typedef struct Table
{
    TableKey *keys;
    TableValue *values;
    size_t size;
    size_t capacity;
} Table;

//
// A-STAR
//

int main()
{
    const char *str = "Hello";
    printf("%s\n", str);
}
