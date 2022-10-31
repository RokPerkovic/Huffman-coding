#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "util.h"
char block_bin_code2[33];


void dec_to_bin(char *bin, unsigned int dec, int size){
    for (int i = size - 1; i >= 0; --i){
    	bin[(size - 1) - i] = (dec & (1 << i)) ? '1' : '0';
    }
    bin[size + 1] = '\0';
}

int16_t make_char(unsigned int **encoded_huff_tree, unsigned int *mask, int *blocks_read){
	int16_t ch = 0;
	
	/*dec_to_bin(block_bin_code2, **encoded_huff_tree, 32);
	printf("%s\n", block_bin_code2);*/
	
	for(int c = 15; c >= 0; c--){	
		if(**encoded_huff_tree & *mask){
			ch+=pow(2, c);
		}
		*mask = *mask >> 1;
					
		if(*mask <= 0){
			//mask reached the end of current tree block
			//advance in array of tree blocks and continue
			
			*encoded_huff_tree = *encoded_huff_tree + 1;
			/*dec_to_bin(block_bin_code2, **encoded_huff_tree, 32);
			printf("next: %s\n", block_bin_code2);*/
			*blocks_read = *blocks_read + 1;
			
			*mask = 1 << 31;
		}	
	}
	
	return ch;
}
