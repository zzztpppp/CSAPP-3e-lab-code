#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

/* Helper function to calculated unsigned 
 * integer power
 */
unsigned long ulong_power(unsigned long x, unsigned long y) {
    unsigned long i = 0u;
    unsigned long result = 1;
    for (;i < y; i++){
        result = result * x;
    }

    return result;
}

/* Helper function to convert 4-bit hex to unsigned int. */
unsigned long hex_to_ulong(char c) {
    unsigned long result;

    if ((c >= 48) && (c <= 57)){
        result = c - 48;
    }

    else{
        if ((c >= 65) && (c <= 70)){
            result = c-55;
        }
        else {
            printf("Arguement %c is not a valid hex value\n", c);
            exit(1);
        }
    }

    return result;
}

/* Given a string representation of 8byte heximal 
 * to unsigned long integer */
 unsigned long convert_hex_string(char * hex_string) {
     int length = strlen(hex_string);
     unsigned long result = 0u;

     for(unsigned long i = 0u; i < length; i++){
        result = ulong_power(16u, i) * hex_to_ulong(hex_string[i]) + result;
     }

     return result;
 }


 /* Mask and extract bit values as unsigned int on a 64-bit address.
  * given the bit posistion. Which is used for extracting set address
  * and tag values of a 64-bit address */
 unsigned long extract_bit_as_uint(unsigned long address, int low, int high){
     
     int width, bits_to_left, bits_to_right;
     unsigned long mask = 0xFFFFFFFF;

     if (low > high){
         printf("Arugment low must not be greater than arguement high\n");
         exit(1);
     }
     if ((low < 0) || (high) > 64){
         printf("Arguement low and high must be in between 0 and 64");
     }

     width = high - low + 1;
     bits_to_left = low;
     bits_to_right = 64 - high;

     /* Craft the mask for the corresponding address segment */
     mask = ((mask << bits_to_left) >> (bits_to_left + bits_to_right)) << bits_to_right;  

     return (mask & address) >> bits_to_right; 
 }



/* Given the memory address, 
 * return the corresponding index of the cache */
int * get_cache_location(char* addr, int num_set_bits, int associativity) {
    unsigned long addr_unint = covert_hex_string(addr);
}


/* Given the memory trace as a file, simulate the cache operations and 
 * report number of hits, misses, evictions in the form of an array */
int* simulate_cache_operation(char *trace_file, unsigned long *cache_sim,
		int num_set_bits, int associativity, int num_block_bits){
	/* Go! */
}


int main(int argc, char *argv[])
{
    int num_set_bits = -1;
    int associativity = -1;
    int num_block_bits = -1;
    int verbose_flag = 0;
    int help_flag = 0;
    char *trace_file_name;
    int results[3];

    unsigned long* cache_sim;

    int c;
    
    /* Parse command line arguements*/
    while ((c = getopt(argc, argv, "hvs:E:b:t:")) != -1){
        switch(c){
            case 'h':
                help_flag = 1;
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case 's':
                num_set_bits = atoi(optarg);
                break;
            case 'E':
                associativity = atoi(optarg);
                break;
            case 'b':
                num_block_bits = atoi(optarg);
                break;
            case 't':
                trace_file_name = optarg;
                break;
            case '?':
            default:
                printf("./csim failed to parse its arguements.\n");
        }
    }

    /* Assert the necessary arguements are fed */
    if (num_set_bits == -1){
        printf("Number of set bits is not fed through -s option\n");
        exit(1);
    }
    if (associativity == -1){
        printf("Associativity is not fed through -E option\n");
        exit(1);
    }
    if (num_block_bits == -1){
        printf("Number of block bits is not fed through -E option\n");
        exit(1);
    }

    /* The simulated cache */
    cache_sim = malloc(ulong_power(2u, num_set_bits) * associativity);

    /* Read memory trace line by line and simulate cache operation */
    resluts = simulate_cache_operation(trace_file_name, cache_sim, num_set_bits,
		    associativity, num_block_bits);


    printSummary(results[0], results[1], results[2]);
    return 0;
}



