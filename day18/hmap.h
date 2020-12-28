#ifndef __HMAP_H__
#define __HMAP_H__

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long (*hmap_hash_func)(void *key);
typedef int (*hmap_cmp_func)(void *key_a, void *key_b);
typedef void (*hmap_print_func)(void *key, void *value);

struct hmap_meta
{
    hmap_hash_func hash_func;
    hmap_cmp_func key_cmp_func;
    size_t key_size;
    size_t value_size;
};

struct hmap_bucket
{
    void *key;
    void *value;
    struct hmap_bucket *next;
};

struct hmap
{
    struct hmap_meta meta;
    struct hmap_bucket *buckets;
    size_t size;
};

struct hmap hmap_alloc(struct hmap_meta meta, size_t size);
void hmap_free(struct hmap *map);
void hmap_insert(struct hmap *map, void *key, void *value);
void hmap_remove(struct hmap *map, void *key);
void *hmap_get(struct hmap *map, void *key);
void hmap_print(struct hmap *map, hmap_print_func print_proc);

#ifdef HMAP_IMPL

struct hmap hmap_alloc(struct hmap_meta meta, size_t size)
{
    struct hmap_bucket *buckets = calloc(size, sizeof(struct hmap_bucket));
    assert(buckets);
    return (struct hmap){
        .meta = meta,
        .buckets = buckets,
        .size = size,
    };
}

void hmap_free(struct hmap *map)
{
    free(map->buckets);
    *map = (struct hmap){0};
}

void hmap_insert(struct hmap *map, void *key, void *value)
{
    // TODO: Replace if key exists

    unsigned long bucket_index = map->meta.hash_func(key) % map->size;

    struct hmap_bucket *bucket = &map->buckets[bucket_index];

    void *key_copy = malloc(map->meta.key_size);
    assert(key_copy);
    memcpy(key_copy, key, map->meta.key_size);

    void *value_copy = malloc(map->meta.value_size);
    assert(value_copy);
    memcpy(value_copy, value, map->meta.value_size);

    if (bucket->key == NULL)
    {
        // Use the first bucket in slot (quick path)

        bucket->key = key_copy;
        bucket->value = value_copy;
    }
    else
    {
        // Use a new bucket in slot (slow path)

        while (bucket->next != NULL)
        {
            bucket = bucket->next;
        }

        bucket->next = malloc(sizeof(struct hmap_bucket));
        assert(bucket->next);

        (*bucket->next) = (struct hmap_bucket){
            .key = key_copy,
            .value = value_copy,
            .next = NULL,
        };
    }
}

void hmap_remove(struct hmap *map, void *key)
{
    unsigned long bucket_index = map->meta.hash_func(key) % map->size;

    struct hmap_bucket *bucket = &map->buckets[bucket_index];

    if (map->meta.key_cmp_func(key, bucket->key) == 0)
    {
        struct hmap_bucket *next = bucket->next;
        if (next)
        {
            map->buckets[bucket_index] = *next;
            free(next);
        }
        else
        {
            *bucket = (struct hmap_bucket){0};
        }
    }
    else
    {
        while (bucket->next)
        {
            if (map->meta.key_cmp_func(key, bucket->next->key) == 0)
            {
                // Next bucket is the one we want

                struct hmap_bucket *next_next = bucket->next->next;
                free(bucket->next);
                bucket->next = next_next;
            }

            bucket = bucket->next;
        }
    }
}

void *hmap_get(struct hmap *map, void *key)
{
    unsigned long bucket_index = map->meta.hash_func(key) % map->size;

    if (map->buckets[bucket_index].key == NULL)
    {
        return false;
    }

    for (struct hmap_bucket *bucket = &map->buckets[bucket_index];
         bucket;
         bucket = bucket->next)
    {
        if (map->meta.key_cmp_func(key, bucket->key) == 0)
        {
            return true;
        }
    }

    return false;
}

void hmap_print(struct hmap *map, hmap_print_func print_proc)
{
    for (size_t i = 0; i < map->size; ++i)
    {
        printf("%zu: ", i);

        if (map->buckets[i].key == NULL)
        {
            puts("(empty)");
        }
        else
        {
            for (struct hmap_bucket *bucket = &map->buckets[i];
                 bucket;
                 bucket = bucket->next)
            {
                putchar('\"');
                print_item_func(bucket->key, bucket->value);
                putchar('\"');
                if (bucket->next)
                {
                    printf(", ");
                }
            }

            putchar('\n');
        }
    }
}

#endif // HMAP_IMPL

#endif // __HMAP_H__
