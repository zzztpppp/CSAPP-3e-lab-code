#include <stdio.h>
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

int main(int argc, char *argv[]) {
    unsigned long r; 
    printf("The original argument is %s\n ", argv[1]);
    r = convert_hex_string(argv[1]);

    printf("%lu\n", r);
}
