/**************************************
 * A concurrent cach implemented for proxy
 *************************************/
#include "csapp.h"

typedef struct {
    char key[50];
    char val[102400];
    size_t size;
} kv_t;


typedef struct  
{
    kv_t **cache;
    int *time;
    int size;
    int clock;
    sem_t mutex, w;
    int readcnt;
} cache_t;


int insert(cache_t *cache, char *key, char *val, size_t size);
int find(cache_t *cache, char *key, kv_t *kv);
int cache_deinit(cache_t *cache);
int cache_init(cache_t *cache, size_t n);