#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct  CacheLine
{
    unsigned long tag_value;
    short valid;
    short age;
} cache_line;

void initialize_cache_lines(cache_line *cf, int num_lines){

    for (int i = 0; i < num_lines; i++){
        cf[i].age = 0;
        cf[i].valid = 0;
        cf[i].tag_vale = 0;
    }
}


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
unsigned long get_cache_set(char* addr, int num_set_bits, int num_block_bits) {
    unsigned long addr_unint = covert_hex_string(addr);
    return extract_bit_as_uint(addr_unint, 64-num_set_bits - num_block_bits, 
                               64 - num_block_bits);
}

unsigned long get_cache_tag(char* addr, int num_set_bits, int num_block_bits){
    unsigned long addr_uint = covert_hex_string(addr);
    return extract_bit_as_uint(addr_uint, 0, 64 - num_block_bits - num_set_bits - 1);
} 

/* Read one line from the given the memory trace
 * file stream(each line is limited to 20 characters long) */
char *read_trace_line(FILE *file_stream){
    /* The maximum length of a line is limited to 20 characters long*/
    int i = 0;
	char *buffer = malloc(20);
    char c;

    while ((!feof(file_stream)) && (i < 20)){
        c = fgetc(file_stream);
            if (c == 10){
                break;
            }
        if (feof(file_stream)){
            return NULL; 
        }

        buffer[i] = c;
        i++;
    }

    return buffer;
}


/* Helper function that extract the hex address of a memory opeartion */
char *hex_address(char *operation){
    char *address_hex;
    int seperator_index;

    address_hex = operation + 3;
    seperator_index = strchr(address_hex, ',') - address_hex;

    // Set the seperator to null to end our address hex.
    address_hex[seperator_index] = '\0';
    return address_hex;
}

/* Given instruction type, set index and tag value
 * return whether we hit or miss the cache, alter the 
 * cache contends accordingly */
int operate_cache(cache_line *cache_sim, unsigned long set_index, 
                  unsigned long tag_value, unsigned long associativity, int age){
    cache_line *set_addr;
    cache_line current_line;
    unsigned long off_set = set_index * associativity; 
    int min_age = 0;
    int min_age_index = 0;

    set_addr = cache_sim + off_set;

    // Linear search for tag a value hit
    for (int i = 0; i < associativity; i++){
        current_line = set_addr[i];
        if (current_line.valid == 1){
            if (current_line.tag_value == tag_value){ return 1;}
        }
    }

    // If its a miss, find a empty line
    for (int i = 0; i < associativity; i++){
        current_line = set_addr[i];
        if (current_line.valid == 0){
            // Found an empty line, fill the value of current instruction
            current_line.valid = 1;
            current_line.tag_value = tag_value;
            current_line.age = age;
            return 2;
        }
    }

    // No empty line? Find the oldest line to evict
    for (int i = 0; i < associativity; i++){
        current_line = set_addr[i];
        if (current_line.age <= min_age){
            min_age = current_line.age;
            min_age_index = i;
        } 
    }

    // Evict
    current_line.tag_value = tag_value;
    current_line.age = age;
    return 3;

}


/* Given the memory trace as a file, simulate the cache operations and 
 * report number of hits, misses, evictions in the form of an array */
int* simulate_cache_operation(char *trace_file, cache_line *cache_sim,
	    int num_set_bits, int associativity, int num_block_bits){

    FILE *trace;
    char *operation_line, operation_address_hex;
    int cache_behavior[3] = {0, 0, 0};
    char operation_type;
    unsigned long set_index, tag_value;

    int cache_result;
    int age = 0;

    /* valid bits to keep track if the cache block is initiallty empty
     * age bits to keep track of when was the cache block last userd */
    int valid_bits[ulong_power(2u, num_set_bits)*associativity];
    int age_bits[ulong_power(2u, num_set_bits) *associativity];


    if (!(trace = fopen(trace_file, 'r'))){
        printf("Couln't open file %s\n", trace_file);
        exit(8);
    }

    /* Read memeory trace line by line */
    while(1){
        operation_line = read_trace_line(trace);
        if (operation_line == NULL){break;} 
        
        /* Ignore intructional memeory trace */
        if (operation_line[0] == 'I') {continue;}

        // Parse the operation tye, operation address from line 
        operation_type = operation_line[1];
        operation_address_hex = hex_address(operation_line);
        set_index = get_cache_set(operation_address_hex, num_set_bits, num_block_bits);
        tag_value = get_cache_tag(operation_address_hex,num_set_bits, num_block_bits);

        cache_result = operate_cache(cache_sim, set_index, tag_value, associativity, age);
        cache_behavior[cache_result] += 1; 
    } 

    return cache_behavior;
}


int main(int argc, char *argv[])
{
    int num_set_bits = -1;
    int associativity = -1;
    int num_block_bits = -1;
    int verbose_flag = 0;
    int help_flag = 0;
    char *trace_file_name;
    int *results;

    cache_line* cache_sim;

    int c, num_lines;
    
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
    num_lines = ulong_power(2u,num_set_bits) * associativity;
    cache_sim = malloc(num_lines *sizeof(cache_line));
    initialize_cache_lines(cache_sim, num_lines);

    /* Read memory trace line by line and simulate cache operation */
    results = simulate_cache_operation(trace_file_name, cache_sim, num_set_bits,
		    associativity, num_block_bits);


    printSummary(results[0], results[1], results[2]);
    return 0;
}



