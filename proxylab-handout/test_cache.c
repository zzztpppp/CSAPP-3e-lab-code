/******************************
 * A unit test for cache.c
 ******************************/

#include <stdio.h>
#include <stdlib.h>
#include "cache.h"


void print_cache(cache_t *cache, int print_val) {
    int i;
    for (i = 0; i < cache->size; i++) {
        printf("key:     %s  ", cache->cache[i]->key);
    }

    printf("\n");

    if (print_val)
        for (i = 0; i < cache->size; i++) {
            printf("val_size: %ld  ", cache->cache[i]->size);
        }

    printf("\n");

    if (print_val)
        for (i = 0; i < cache->size; i++) {
            printf("val:     %s  ", cache->cache[i]->val);
        }

    printf("\n");

    for (i = 0; i < cache->size; i++) {
        printf("time:    %d  ", cache->time[i]);
    }
    printf("\n\n");
    return;
}

int main(int argc, char **argv) {
    cache_t cache;
    cache_init(&cache, 10);
    char keys[] = "abcdefghij";
    char *vals[] = {"aaa", "cc", "abcd", "cdef", "mmmnld", "laisfj", "dkllj", "dfadfas", "askljfkljas", "aklsdjfja"};
    char key[2] = "\0";
    char *val;
    
    int j, i = 0; 
    kv_t temp;

    // Test searching the empty cache.
    for (i = 0; i < cache.size; i++) {
        j = rand() % cache.size;
        key[0] = keys[j];
        printf("Finding %s\n", key);
        printf("Result is %d\n", find(&cache, key, &temp));
        print_cache(&cache, 0);
    }


    // Fill up the cache with elements.
    for (i = 0; i < cache.size; i++) {
        key[0] = keys[i];
        val = vals[i];
        insert(&cache, key, val, strlen(val));
        print_cache(&cache, 0);
    }

    // Perform some random access to the cache.
    for (i = 0; i < cache.size; i++) {
        j = rand() % cache.size;
        key[0] = keys[j];
        printf("Finding %s\n", key);
        printf("Result is %d\n", find(&cache, key, &temp));
        print_cache(&cache, 1);
    }

    // Perform searching for non-existing keys to test robustness.
    char absent_keys[] = "xyz";
    for (i = 0; i < strlen(absent_keys); i++) {
        key[0] = absent_keys[i];
        printf("Finding %s\n", key);
        printf("Result is %d\n", find(&cache, key, &temp));
        print_cache(&cache, 1);
    }

    // Test eviction by inserting new objects.
    char new_key[] = "klmnopqrstuvw";
    for (i = 0; i < cache.size; i++) {
        key[0] = new_key[i];
        val = vals[10 - i  -1];
        insert(&cache, key, val, strlen(val));
        print_cache(&cache, 1);
    }
    
    return 0;
}