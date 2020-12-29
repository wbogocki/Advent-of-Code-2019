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
    // Replace if key exists

    void *existing_value = hmap_get(map, key);
    if (existing_value)
    {
        memcpy(existing_value, value, map->meta.value_size);
        return;
    }

    // Insert new

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
                return;
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
        return NULL;
    }

    for (struct hmap_bucket *bucket = &map->buckets[bucket_index];
         bucket;
         bucket = bucket->next)
    {
        if (map->meta.key_cmp_func(key, bucket->key) == 0)
        {
            return bucket->value;
        }
    }

    return NULL;
}

void hmap_print(struct hmap *map, hmap_print_func print_func)
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
                print_func(bucket->key, bucket->value);
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

#ifdef HMAP_TEST

struct hmap_test_key
{
    const char *str;
};

struct hmap_test_value
{
    const char *str;
};

unsigned long
hmap_test_hash(void *key)
{
    const char *str = ((struct hmap_test_key *)key)->str;

    unsigned long hash = 5381;

    for (const char *c = str; *c; ++c)
    {
        hash = ((hash << 5) + hash) + *c;
    }

    return hash;
}

int hmap_test_key_compare(void *a, void *b)
{
    const char *str_a = ((struct hmap_test_key *)a)->str;
    const char *str_b = ((struct hmap_test_key *)b)->str;
    return strcmp(str_a, str_b);
}

void hmap_test_print(void *key, void *value)
{
    const char *key_str = ((struct hmap_test_key *)key)->str;
    const char *value_str = ((struct hmap_test_value *)value)->str;
    printf("%s -> %s", key_str, value_str);
}

void hmap_test()
{
    struct hmap map = hmap_alloc((struct hmap_meta){
                                     .hash_func = hmap_test_hash,
                                     .key_cmp_func = hmap_test_key_compare,
                                     .key_size = sizeof(struct hmap_test_key),
                                     .value_size = sizeof(struct hmap_test_value),
                                 },
                                 8);

    struct hmap_test_key alice = {.str = "alice"};
    struct hmap_test_key bob = {.str = "bob"};
    struct hmap_test_key pigeon = {.str = "albatross"};

    hmap_insert(&map, &alice, &bob);
    hmap_insert(&map, &bob, &pigeon);
    hmap_insert(&map, &bob, &bob);
    hmap_insert(&map, &pigeon, &alice);

    hmap_remove(&map, &pigeon);

    puts(((struct hmap_test_value *)hmap_get(&map, &pigeon)) ? "present" : "null");

    hmap_print(&map, hmap_test_print);

    hmap_free(&map);
}

#endif // HMAP_TEST

#endif // __HMAP_H__
