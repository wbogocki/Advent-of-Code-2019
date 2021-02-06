#ifndef __TABLE_H__
#define __TABLE_H__

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * Usage:
 *
 * Table *table = table_create(8);
 *
 * table_set(table, "a1", "1", 2);
 * table_set(table, "b2", "2", 2);
 * table_set(table, "c3", "3", 2);
 * table_set(table, "d4", "4", 2);
 *
 * printf("a1 -> %s\n", (char *)table_get(table, "a1"));
 * printf("b2 -> %s\n", (char *)table_get(table, "b2"));
 * printf("c3 -> %s\n", (char *)table_get(table, "c3"));
 * printf("d4 -> %s\n", (char *)table_get(table, "d4"));
 *
 * for (TableEntry *entry = table_next(table, NULL);
 *      entry;
 *      entry = table_next(table, entry))
 * {
 *     printf("%s -> %s\n", entry->key, (char *)entry->value);
 * }
 *
 * table_destroy(table);
 **/

struct TableEntry;
struct Table;

typedef struct TableEntry TableEntry;
typedef struct Table Table;

struct TableEntry
{
    char *key;
    void *value;
    TableEntry *next;
};

struct Table
{
    TableEntry **entries;
    size_t size;
};

Table *table_create(size_t size);
void table_destroy(Table *table);
void *table_set(Table *table, const char *key, void *value, size_t value_size);
void table_unset(Table *table, const char *key);
void *table_get(Table *table, const char *key);
void *table_get_default(Table *table, const char *key, void *default);
TableEntry *table_next(Table *table, TableEntry *previous);
bool table_empty(Table *table);

#ifdef TABLE_IMPL

TableEntry *table_entry_create(const char *key, void *value, size_t value_size, TableEntry *next)
{
    // Allocate and set the key

    char *entry_key = strdup(key);
    assert(entry_key);

    // Allocate and set the value

    char *entry_value = malloc(value_size);
    assert(entry_value);

    memcpy(entry_value, value, value_size);

    // Allocate and init the entry

    TableEntry *entry = malloc(sizeof(TableEntry));
    assert(entry);

    *entry = (TableEntry){.key = entry_key, .value = entry_value, next = next};

    return entry;
}

TableEntry *table_entry_destroy(TableEntry *entry)
{
    free(entry->key);
    free(entry->value);

    entry->key = NULL;
    entry->value = NULL;

    return entry->next;
}

void table_entry_replace(TableEntry *entry, void *value, size_t value_size)
{
    entry->value = realloc(entry->value, value_size);
    assert(entry->value);

    memcpy(entry->value, value, value_size);
}

size_t table_hash(const char *key)
{
    // http://www.cse.yorku.ca/~oz/hash.html

    size_t hash = 5381;
    int c;

    while (c = *key++)
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

Table *table_create(size_t size)
{
    // Allocate and zero entries

    TableEntry **entries = calloc(sizeof(TableEntry *), size);
    assert(entries && "Table alloc failed!");

    memset(entries, 0, sizeof(TableEntry *) * size);

    // Allocate and init the table

    Table *table = malloc(sizeof(Table));
    assert(table && "Table alloc failed!");

    *table = (Table){.entries = entries, .size = size};

    return table;
}

void table_destroy(Table *table)
{
    if (table)
    {
        for (size_t i = 0; i < table->size; ++i)
        {
            while (table->entries[i])
            {
                table->entries[i] = table_entry_destroy(table->entries[i]);
            }
        }

        free(table->entries);
        free(table);
    }
}

void *table_set(Table *table, const char *key, void *value, size_t value_size)
{
    size_t bin = table_hash(key) % table->size;

    // Look for an existing entry

    TableEntry *curr_entry = NULL;

    for (TableEntry *entry = table->entries[bin]; entry; entry = entry->next)
    {
        if (strcmp(entry->key, key) == 0)
        {
            curr_entry = entry;
            break;
        }
    }

    // Replace the existing entry or create a new one

    if (curr_entry)
    {
        table_entry_replace(curr_entry, value, value_size);
        return curr_entry->value;
    }
    else
    {
        table->entries[bin] = table_entry_create(key, value, value_size, table->entries[bin]);
        return table->entries[bin]->value;
    }
}

void table_unset(Table *table, const char *key)
{
    size_t bin = table_hash(key) % table->size;

    for (TableEntry **next = &table->entries[bin]; *next; next = &(*next)->next)
    {
        if (strcmp((*next)->key, key) == 0)
        {
            *next = table_entry_destroy(*next);
            break;
        }
    }
}

void *table_get(Table *table, const char *key)
{
    size_t bin = table_hash(key) % table->size;

    for (TableEntry *entry = table->entries[bin]; entry; entry = entry->next)
    {
        if (strcmp(entry->key, key) == 0)
        {
            return entry->value;
        }
    }

    return NULL;
}

void *table_get_default(Table *table, const char *key, void *default)
{
    void *value = table_get(table, key);
    return value ? value : default;
}

TableEntry *table_next(Table *table, TableEntry *previous)
{
    if (!previous)
    {
        // Grab the first entry in the first bin

        for (size_t i = 0; i < table->size; ++i)
        {
            if (table->entries[i])
            {
                return table->entries[i];
            }
        }
    }
    else
    {
        // Grab the next entry in the same bin if it exists; else grab the first entry in the next bin

        if (previous->next)
        {
            return previous->next;
        }
        else
        {
            size_t bin = table_hash(previous->key) % table->size;

            for (size_t i = bin + 1; i < table->size; ++i)
            {
                if (table->entries[i])
                {
                    return table->entries[i];
                }
            }
        }
    }

    return NULL;
}

bool table_empty(Table *table)
{
    return table_next(table, NULL) == NULL;
}

#endif

#endif