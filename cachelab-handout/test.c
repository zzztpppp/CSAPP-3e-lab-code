#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
unsigned long ulong_power(unsigned long x, unsigned long y) {
	    unsigned long i = 0u;
	        unsigned long result = 1;
		    for (;i < y; i++){
			            result = result * x;
				        }

		        return result;
}
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
						                printf("Arguement %c is not a valid hex value", c);
								            exit(1);
									            }
					        }

		        return result;
}
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
     printf("addr:%lx ", address);

     if (low > high){
         printf("Arugment low must not be greater than arguement high\n");
         exit(1);
     }
     if ((low < 0) || (high) > 64){
         printf("Arguement low and high must be in between 0 and 64");
     }

     width = high - low + 1;
     bits_to_left = low;
     bits_to_right = 64 - high - 1;

     /* Craft the mask for the corresponding address segment */
     mask = ((mask << bits_to_left) >> (bits_to_left + bits_to_right)) << bits_to_right;  
	 printf("mask:%lx ", mask);

     return (mask & address) >> bits_to_right; 
 }

int main(int argc, char *argv[]) {
    
	int in;
	int low, high;
	unsigned long r;
    in = atoi(argv[1]);
	low = atoi(argv[2]);
	high = atoi(argv[3]);
	r = extract_bit_as_uint(in, low, high);
	printf("result:%lu\n", r);
    return 0;
}
