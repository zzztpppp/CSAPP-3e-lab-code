#include <stdio.h>
#include "csapp.h"
#include "cache.h"


int cache_init(cache_t *cache, size_t n) {
    int i;
    cache->cache = Calloc(sizeof(kv_t), n);
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

int insert(cache_t *cache, char *key, char *val, size_t size) {
    
}