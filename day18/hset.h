#ifndef __HSET_H__
#define __HSET_H__

#include <stdlib.h>

typedef int (*hfunc)(void *item);
typedef int (*hcmpfunc)(void *a, void *b);

struct hset_meta
{
    hfunc hash_func;
    hcmpfunc cmp_func;
    size_t item_size;
};

struct hset_bucket
{
    void *items;
    int size;
};

struct hset
{
    struct hset_meta meta;
    struct hset_bucket *buckets;
    int size;
};

void hset_alloc(struct hset_meta meta, size_t size);
void hset_free(struct hset *set);
void hset_add(struct hset *set, void *item);
void hset_remove(struct hset *set, void *item);
bool hset_in(struct hset *set, void *item);

#ifdef HSET_IMPL

void hset_alloc(struct hset_meta meta, size_t size)
{
}

void hset_free(struct hset *set)
{
}

void hset_add(struct hset *set, void *item)
{
}

void hset_remove(struct hset *set, void *item)
{
}

bool hset_in(struct hset *set, void *item)
{
}

#endif // HSET_IMPL
#endif // __HSET_H__