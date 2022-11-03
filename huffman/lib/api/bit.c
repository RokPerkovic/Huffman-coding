#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "util.h"
#include "bit.h"


char block_bin_code[33];


int read_bit(unsigned int *encoded_block, unsigned int *mask){
	if(*encoded_block & *mask){ 
		return 1;
	}
	
	return 0;
}


/*
	TODO: *optimization: try to move whole *bits* in *bit_buffer*
	*if there is no room for all the *bits* continue by shifting 1 bit a time
*/

void write_bits(/*int16_t bits*/char *bits, int size, unsigned int *bit_buffer, int *bit_count, int *block_count, int out_fd){

	int i;
	
	for(i = 0; i < size; i++){
		if(bits[i] == '1'){
			*bit_buffer = *bit_buffer << 1 | 1;
		}
		else{
			*bit_buffer = *bit_buffer << 1;
		}
		
		*bit_count = *bit_count + 1;
		
		if (*bit_count == 32) { 
			int n = write(out_fd, bit_buffer, sizeof(unsigned int));
			/*dec_to_bin(block_bin_code, *bit_buffer, 32);
			printf("%s\n", block_bin_code);*/
			
			if(n < 0){
				perror("error writing compressed");
			}
			*bit_buffer = 0;
			*bit_count = 0;
			*block_count = *block_count + 1;
			break;	
		}
	}

	if(i < size - 1){
		size = strlen(bits + i + 1);
		write_bits(bits + i + 1, size, bit_buffer, bit_count, block_count, out_fd);			
	}	
}
