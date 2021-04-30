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
} cache_t;

