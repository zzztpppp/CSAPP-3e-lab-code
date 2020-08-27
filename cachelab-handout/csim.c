#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>

FILE *infile

int main(int argc, char *argv[])
{
    int num_set_bits = -1;
    int associativity = -1;
    int num_block_bits = -1
    int verbose_flag = 0;
    int help_flag = 0;
    char *input_file_name;

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
                num_block_bits = atoi(optarg)
                break;
            case 't':
                input_file_name = optarg;
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

    printSummary(0, 0, 0);
    return 0;
}

