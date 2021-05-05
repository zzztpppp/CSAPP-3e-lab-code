/******************************
 * A unit test for cache.c
 ******************************/

#include <stdio.h>
#include <stdlib.h>
#include "cache.h"


void print_cache(cache_t *cache) {
    int i;
    for (i = 0; i < cache->size; i++) {
        printf("%s  ", cache->cache[i]->key);
    }

    printf("\n");

    for (i = 0; i < cache->size; i++) {
        printf("%d  ", cache->time[i]);
    }
    printf("\n\n");
    return;
}

int main(int argc, char **argv) {
    cache_t cache;
    cache_init(&cache, 10);
    char keys[] = "abcdefghij";
    char key[2] = "\0";
    
    int j, i = 0; 
    kv_t temp;

    // Fill up the cache with elements.
    for (i = 0; i < cache.size; i++) {
        key[0] = keys[i];
        insert(&cache, key, "bbb", 3);
        print_cache(&cache);
    }

    // Perform some random access to the cache.
    for (i = 0; i < cache.size; i++) {
        j = rand() % cache.size;
        key[0] = keys[j];
        printf("Finding %s\n", key);
        printf("Result is %d\n", find(&cache, key, &temp));
        print_cache(&cache);
    }

    // Test eviction by inserting new objects.
    char new_key[] = "klmnopqrstuvw";
    for (i = 0; i < cache.size; i++) {
        key[0] = new_key[i];
        insert(&cache, key, "bbb", 3);
        print_cache(&cache);
    }
    
    return 0;
}