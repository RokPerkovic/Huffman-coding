#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "util.h"
char block_bin_code2[65];

void dec_to_bin(char *bin, unsigned long dec, int size){
    for (int i = size - 1; i >= 0; --i){
    	bin[(size - 1) - i] = (dec & (1UL << i)) ? '1' : '0';
    }
    bin[size + 1] = '\0';
}

int16_t make_char(unsigned long **encoded_huff_tree, unsigned long *mask){
	int16_t ch = 0;
	
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
			*mask = 1UL << 63;
		}	
	}
	return ch;
}

void read_block(int in_fd, unsigned long *content_block){
	int n;

	if((n = read(in_fd, content_block, sizeof(unsigned long))) < 0){
		perror("Error while reading encoded file...");	
	}
	/*dec_to_bin(block_bin_code2, *content_block, 64);
	printf("\n%s\n", block_bin_code2);*/
}
