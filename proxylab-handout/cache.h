/**************************************
 * A concurrent cach implemented for proxy
 *************************************/

typedef struct {
    char *key;
    char *val;
} kv_t;


typedef struct  
{
    kv_t *cache;
    int *time;
    int size;
    sem_t mutex, w;
    int readcnt;
} cache_t;

