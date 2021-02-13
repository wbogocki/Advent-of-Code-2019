#ifndef __TABLE_H__
#define __TABLE_H__

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * Usage:
 *
 * table *Table = TableCreate(8);
 *
 * TableSet(Table, "a1", "1", 2);
 * TableSet(Table, "b2", "2", 2);
 * TableSet(Table, "c3", "3", 2);
 * TableSet(Table, "d4", "4", 2);
 *
 * printf("a1 -> %s\n", (char *)TableGet(Table, "a1"));
 * printf("b2 -> %s\n", (char *)TableGet(Table, "b2"));
 * printf("c3 -> %s\n", (char *)TableGet(Table, "c3"));
 * printf("d4 -> %s\n", (char *)TableGet(Table, "d4"));
 *
 * for (table_entry *Entry = TableNext(Table, NULL);
 *      Entry;
 *      Entry = TableNext(Table, Entry))
 * {
 *     printf("%s -> %s\n", Entry->Key, (char *)Entry->Value);
 * }
 *
 * TableDestroy(table);
 **/

typedef struct table_entry table_entry;
typedef struct table table;

struct table_entry
{
    char *Key;
    void *Value;
    table_entry *Next;
};

struct table
{
    table_entry **Entries;
    size_t Size;
};

table *TableCreate(size_t Size);
void TableDestroy(table *Table);
void *TableSet(table *Table, const char *Key, void *Value, size_t ValueSize);
void TableUnset(table *Table, const char *Key);
void *TableGet(table *Table, const char *Key);
void *TableGetDefault(table *Table, const char *Key, void *Default);
table_entry *TableNext(table *Table, table_entry *Previous);
bool TableEmpty(table *Table);

#ifdef TABLE_IMPL
#undef TABLE_IMPL

table_entry *TableEntryCreate(const char *Key, void *Value, size_t ValueSize, table_entry *Next)
{
    // Allocate and set the key

    char *EntryKey = strdup(Key);
    assert(EntryKey);

    // Allocate and set the value

    char *EntryValue = malloc(ValueSize);
    assert(EntryValue);

    memcpy(EntryValue, Value, ValueSize);

    // Allocate and init the entry

    table_entry *Entry = malloc(sizeof(table_entry));
    assert(Entry);

    *Entry = (table_entry){.Key = EntryKey, .Value = EntryValue, .Next = Next};

    return Entry;
}

table_entry *TableEntryDestroy(table_entry *Entry)
{
    free(Entry->Key);
    free(Entry->Value);

    Entry->Key = NULL;
    Entry->Value = NULL;

    return Entry->Next;
}

void TableEntryReplace(table_entry *Entry, void *Value, size_t ValueSize)
{
    Entry->Value = realloc(Entry->Value, ValueSize);
    assert(Entry->Value);

    memcpy(Entry->Value, Value, ValueSize);
}

size_t TableHash(const char *Key)
{
    // http://www.cse.yorku.ca/~oz/hash.html

    size_t Hash = 5381;
    int Char = *Key++;

    while (Char)
    {
        Hash = ((Hash << 5) + Hash) + Char; /* hash * 33 + c */
        Char = *Key++;
    }

    return Hash;
}

table *TableCreate(size_t Size)
{
    // Allocate and zero entries

    table_entry **Entries = calloc(sizeof(table_entry *), Size);
    assert(Entries && "Table alloc failed!");

    memset(Entries, 0, sizeof(table_entry *) * Size);

    // Allocate and init the table

    table *Table = malloc(sizeof(table));
    assert(Table && "Table alloc failed!");

    *Table = (table){.Entries = Entries, .Size = Size};

    return Table;
}

void TableDestroy(table *Table)
{
    if (Table)
    {
        for (size_t i = 0; i < Table->Size; ++i)
        {
            while (Table->Entries[i])
            {
                Table->Entries[i] = TableEntryDestroy(Table->Entries[i]);
            }
        }

        free(Table->Entries);
        free(Table);
    }
}

void *TableSet(table *Table, const char *Key, void *Value, size_t ValueSize)
{
    size_t Bin = TableHash(Key) % Table->Size;

    // Look for an existing entry

    table_entry *CurrentEntry = NULL;

    for (table_entry *Entry = Table->Entries[Bin]; Entry; Entry = Entry->Next)
    {
        if (strcmp(Entry->Key, Key) == 0)
        {
            CurrentEntry = Entry;
            break;
        }
    }

    // Replace the existing entry or create a new one

    if (CurrentEntry)
    {
        TableEntryReplace(CurrentEntry, Value, ValueSize);
        return CurrentEntry->Value;
    }
    else
    {
        Table->Entries[Bin] = TableEntryCreate(Key, Value, ValueSize, Table->Entries[Bin]);
        return Table->Entries[Bin]->Value;
    }
}

void TableUnset(table *Table, const char *Key)
{
    size_t Bin = TableHash(Key) % Table->Size;

    for (table_entry **Next = &Table->Entries[Bin]; *Next; Next = &(*Next)->Next)
    {
        if (strcmp((*Next)->Key, Key) == 0)
        {
            *Next = TableEntryDestroy(*Next);
            break;
        }
    }
}

void *TableGet(table *Table, const char *Key)
{
    size_t Bin = TableHash(Key) % Table->Size;

    for (table_entry *Entry = Table->Entries[Bin]; Entry; Entry = Entry->Next)
    {
        if (strcmp(Entry->Key, Key) == 0)
        {
            return Entry->Value;
        }
    }

    return NULL;
}

void *TableGetDefault(table *Table, const char *Key, void *Default)
{
    void *Value = TableGet(Table, Key);
    return Value ? Value : Default;
}

// NOTE: This is very slow for large tables because it has to check every bin one by one,
// a better approach would be to store a list of bins that contain values.
table_entry *TableNext(table *Table, table_entry *Previous)
{
    if (!Previous)
    {
        // Grab the first entry in the first bin

        for (size_t i = 0; i < Table->Size; ++i)
        {
            if (Table->Entries[i])
            {
                return Table->Entries[i];
            }
        }
    }
    else
    {
        // Grab the next entry in the same bin if it exists; else grab the first entry in the next bin

        if (Previous->Next)
        {
            return Previous->Next;
        }
        else
        {
            size_t Bin = TableHash(Previous->Key) % Table->Size;

            for (size_t i = Bin + 1; i < Table->Size; ++i)
            {
                if (Table->Entries[i])
                {
                    return Table->Entries[i];
                }
            }
        }
    }

    return NULL;
}

bool TableEmpty(table *Table)
{
    return TableNext(Table, NULL) == NULL;
}

#endif

#endif