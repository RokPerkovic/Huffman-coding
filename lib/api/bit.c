#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"
#include "bit.h"


char block_bin_code3[BIT_BUFFER_SIZE];


int read_bit(unsigned long *encoded_block, unsigned long *mask){
	if(*encoded_block & *mask){ 
		return 1;
	}
	
	return 0;
}


/*
	TODO: *optimization: try to move whole *bits* in *bit_buffer*
	*if there is no room for all the *bits* continue by shifting 1 bit a time
*/

void write_bits(unsigned long bits, int size, unsigned long *bit_buffer, int *bit_count, int *block_count, int out_fd){
	
	if(*bit_count + size > BIT_BUFFER_SIZE){
		//printf("*bits: %lu\n", bits);
		unsigned long mask = 1UL << (size - 1);
		int i;
		for(i = 0; i < size; i++){
			*bit_buffer = *bit_buffer << 1UL | read_bit(&bits, &mask);
			*bit_count = *bit_count + 1;
			mask = mask >> 1UL;
			
			if(*bit_count == BIT_BUFFER_SIZE){
				int n;
				if((n = write(out_fd, bit_buffer, sizeof(unsigned long))) < 0){
					perror("Error writing bits...");
					exit(1);
				}
				/*printf("WRITE!\n");
				dec_to_bin(block_bin_code3, *bit_buffer, BIT_BUFFER_SIZE);
				printf("%s\n", block_bin_code3);*/
				
				*bit_buffer = 0;
				*bit_count = 0;
				*block_count = *block_count + 1;
			}	
		}
		
		if(i < size){
			//bit_buffer is full but there are still bits left
			write_bits(bits, size - i - 1, bit_buffer, bit_count, block_count, out_fd);		
		}
		return;
	}
	
	
	if(size == 1 && bits == 1){
		//huff_tree geometry --> leaf
		*bit_buffer = *bit_buffer << 1 | 1UL;
		*bit_count = *bit_count + 1;
		/*printf("bits: %lu\n", bits);
		dec_to_bin(block_bin_code3, *bit_buffer, BIT_BUFFER_SIZE);
		printf("%s, %d\n", block_bin_code3, *bit_count);*/
	}
	else if(size == 1 && bits == 0){
		//huff_tree geometry --> inner node
		*bit_buffer = *bit_buffer << 1UL;
		*bit_count = *bit_count + 1;
		/*printf("bits: %lu\n", bits);
		dec_to_bin(block_bin_code3, *bit_buffer, BIT_BUFFER_SIZE);
		printf("%s, %d\n", block_bin_code3, *bit_count);*/
	}
	else{
		//huff_tree --> character
		*bit_buffer = *bit_buffer << size | bits;
		*bit_count = *bit_count + size;
		//printf("char: %c\n", bits);
		/*dec_to_bin(block_bin_code3, *bit_buffer, BIT_BUFFER_SIZE);
		printf("%s, %d\n", block_bin_code3, *bit_count);*/
	}
	
	//printf("bit count: %d\n", *bit_count);
	
	if(*bit_count == BIT_BUFFER_SIZE){
		int n;
		if((n = write(out_fd, bit_buffer, sizeof(unsigned long))) < 0){
			perror("Error writing bits...");
			exit(1);
		}
		/*printf("WRITE!\n");
		dec_to_bin(block_bin_code3, *bit_buffer, BIT_BUFFER_SIZE);
		printf("%s\n", block_bin_code3);*/
		
		*bit_buffer = 0;
		*bit_count = 0;
		*block_count = *block_count + 1;
	}	
}
