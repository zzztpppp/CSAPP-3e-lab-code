#include <stdio.h>
#include "csapp.h"
#include "cache.h"


int cache_init(cache_t *cache, size_t n) {
    int i;
    cache->cache = Calloc(sizeof(kv_t *), n);
    cache->size = n;
    cache->time = Calloc(sizeof(int), n);
    Sem_init(&cache->mutex, 0, 1);
    Sem_init(&cache->w, 0, 1);
    cache->readcnt = 0;
    return 0; 
}


int cache_deinit(cache_t *cache) {
    Free(cache->cache);
    Free(cache->time);
    return 0;
}


/* Lcok the caceh and perform insertion
 */ 
int insert(cache_t *cache, char *key, char *val, size_t size) {
    P(&cache->w);
    kv_t *obj;
    int to_insert = 0;
    int least_time = cache->time[0];

    // First find for an empty slot, if no found, evict 
    // the slot that is least recently used.
    for (int i = 1; i < cache->size; i++) {
        if (cache->time[i] < least_time) {
            to_insert = i;
            least_time = cache->time[i];

            /* Evict the memory use of old object*/
            Free(cache->cache[to_insert]);
        }
    }

    cache->time[to_insert]++;
    obj = Malloc(sizeof(kv_t));
    strncpy(obj->key, key, 50);
    memcpy(obj->val, val, size);
    cache->cache[to_insert] = obj;
    V(&cache->w);
    return 0;
}


/*
 * find - Find the value of given key
 *     in the given cache.
 */
int find(cache_t *cache, char *key, kv_t *kv) {
    P(&cache->readcnt);
    cache->readcnt++;
    if (cache->readcnt == 1) 
        P(&cache->w);
    V(&cache->mutex);

    /* Linearly search the entire cache */
    int i, found = 0;
    kv_t *obj;
    for (i = 0; i < cache->size; i++) {
        obj = cache->cache[i];
        if (!strcmp(key, obj->key)) {
            memcpy(obj, kv, sizeof(kv_t));
            found = 1;
        }
    }

    P(&cache->mutex);
    cache->readcnt--;
    if (cache->readcnt == 0)
        V(&cache->w);
    V(&cache->mutex);

    return found;
}