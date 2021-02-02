#ifndef __TABLE_H__
#define __TABLE_H__

#include <assert.h>
#include <stdlib.h>
#include <string.h>

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
void *table_get(Table *table, const char *key);
TableEntry *table_next(Table *table, TableEntry *previous);

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

TableEntry *table_next(Table *table, TableEntry *previous)
{
    if (previous)
    {
        if (previous->next)
        {
            return previous->next;
        }
        else
        {
            size_t previous_bin = table_hash(previous->key) % table->size;

            for (size_t i = previous_bin + 1; i < table->size; ++i)
            {
                if (table->entries[i])
                {
                    return table->entries[i];
                }
            }
        }
    }
    else
    {
        for (size_t i = 0; i < table->size; ++i)
        {
            if (table->entries[i])
            {
                return table->entries[i];
            }
        }
    }

    return NULL;
}

#endif

#endif