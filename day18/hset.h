#ifndef __HSET_H__
#define __HSET_H__

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long (*hfunc)(void *item);
typedef int (*hcmpfunc)(void *a, void *b);
typedef void (*hprint)(void *item);

struct hset_meta
{
    hfunc hash_func;
    hcmpfunc cmp_func;
    size_t item_size;
};

struct hset_bucket
{
    void *item;
    struct hset_bucket *next;
};

struct hset
{
    struct hset_meta meta;
    struct hset_bucket *buckets;
    size_t size;
};

struct hset hset_alloc(struct hset_meta meta, size_t size);
void hset_free(struct hset *set);
void hset_add(struct hset *set, void *item);
void hset_remove(struct hset *set, void *item);
bool hset_in(struct hset *set, void *item);
void hset_print(struct hset *set, hprint print_proc);

#ifdef HSET_IMPL

struct hset hset_alloc(struct hset_meta meta, size_t size)
{
    struct hset_bucket *buckets = calloc(size, sizeof(struct hset_bucket));
    assert(buckets);
    return (struct hset){
        .meta = meta,
        .buckets = buckets,
        .size = size,
    };
}

void hset_free(struct hset *set)
{
    free(set->buckets);
    *set = (struct hset){0};
}

void hset_add(struct hset *set, void *item)
{
    if (hset_in(set, item))
    {
        return; // item already present
    }

    unsigned long bucket_index = set->meta.hash_func(item) % set->size;

    void *item_copy = malloc(set->meta.item_size);
    assert(item_copy);
    memcpy(item_copy, item, set->meta.item_size);

    struct hset_bucket *bucket = &set->buckets[bucket_index];
    if (bucket->item == NULL)
    {
        // Use the first bucket in slot (quick path)

        bucket->item = item_copy;
    }
    else
    {
        // Use a new bucket in slot (slow path)

        while (bucket->next != NULL)
        {
            bucket = bucket->next;
        }

        bucket->next = malloc(sizeof(struct hset_bucket));
        assert(bucket->next);

        (*bucket->next) = (struct hset_bucket){
            .item = item_copy,
            .next = NULL,
        };
    }
}

void hset_remove(struct hset *set, void *item)
{
    unsigned long bucket_index = set->meta.hash_func(item) % set->size;

    struct hset_bucket *bucket = &set->buckets[bucket_index];

    if (set->meta.cmp_func(item, bucket->item) == 0)
    {
        struct hset_bucket *next = bucket->next;
        if (next)
        {
            set->buckets[bucket_index] = *next;
            free(next); // free the original node memory
        }
        else
        {
            *bucket = (struct hset_bucket){0};
        }
    }
    else
    {
        while (bucket->next)
        {
            if (set->meta.cmp_func(item, bucket->next->item) == 0)
            {
                // Next bucket is the one we want

                struct hset_bucket *next_next = bucket->next->next;
                free(bucket->next);
                bucket->next = next_next;
            }

            bucket = bucket->next;
        };
    }
}

bool hset_in(struct hset *set, void *item)
{
    unsigned long bucket_index = set->meta.hash_func(item) % set->size;

    if (set->buckets[bucket_index].item == NULL)
    {
        return false;
    }

    for (struct hset_bucket *bucket = &set->buckets[bucket_index];
         bucket;
         bucket = bucket->next)
    {
        if (set->meta.cmp_func(item, bucket->item) == 0)
        {
            return true;
        }
    }

    return false;
}

void hset_print(struct hset *set, hprint print_item_func)
{
    for (size_t i = 0; i < set->size; ++i)
    {
        printf("%zu: ", i);

        if (set->buckets[i].item == NULL)
        {
            puts("(empty)");
        }
        else
        {
            for (struct hset_bucket *bucket = &set->buckets[i];
                 bucket;
                 bucket = bucket->next)
            {
                putchar('\"');
                print_item_func(bucket->item);
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

#endif // HSET_IMPL

#ifdef HSET_TEST

struct hset_test_item
{
    const char *str;
};

unsigned long
hset_test_hash(void *item)
{
    const char *str = ((struct hset_test_item *)item)->str;

    unsigned long hash = 5381;

    for (const char *c = str; *c; ++c)
    {
        hash = ((hash << 5) + hash) + *c;
    }

    return hash;
}

int hset_test_compare(void *a, void *b)
{
    const char *str_a = ((struct hset_test_item *)a)->str;
    const char *str_b = ((struct hset_test_item *)b)->str;
    return strcmp(str_a, str_b);
}

void hset_test_print(void *item)
{
    const char *str = ((struct hset_test_item *)item)->str;
    printf("%s", str);
}

void hset_test()
{
    struct hset set = hset_alloc((struct hset_meta){
                                     .hash_func = hset_test_hash,
                                     .cmp_func = hset_test_compare,
                                     .item_size = sizeof(struct hset_test_item),
                                 },
                                 8);

    struct hset_test_item hello = {"Hello"};
    struct hset_test_item alice = {"Alice"};
    struct hset_test_item bob = {"Bob"};
    struct hset_test_item albatross = {"Albatross"};

    hset_add(&set, &hello);
    hset_add(&set, &alice);
    hset_add(&set, &bob);

    hset_print(&set, hset_test_print);

    printf("Hello in set: %s\n", hset_in(&set, &hello) ? "true" : "false");
    printf("Albatross in set: %s\n", hset_in(&set, &albatross) ? "true" : "false");

    printf("---\n");

    hset_add(&set, &albatross);

    hset_print(&set, hset_test_print);

    printf("Hello in set: %s\n", hset_in(&set, &hello) ? "true" : "false");
    printf("Albatross in set: %s\n", hset_in(&set, &albatross) ? "true" : "false");

    printf("---\n");

    hset_remove(&set, &hello);
    hset_remove(&set, &bob);

    hset_print(&set, hset_test_print);

    printf("Hello in set: %s\n", hset_in(&set, &hello) ? "true" : "false");
    printf("Bob in set: %s\n", hset_in(&set, &bob) ? "true" : "false");
    printf("Albatross in set: %s\n", hset_in(&set, &albatross) ? "true" : "false");
}

#endif // HSET_TEST

#endif // __HSET_H__