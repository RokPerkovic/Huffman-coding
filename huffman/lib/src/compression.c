
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


#include "../api/tree.h"
#include "../api/util.h"

int block_count = 0;
char block_bin_code[33];
unsigned char input_buff[BUFF_SIZE + 1];


void traverseHTree(huff_node *root){
	if(root == NULL){
		return;
	}
	printf("%c, %s\n", root->c->c, root->c->h_code);
	traverseHTree(root->left);
	
	traverseHTree(root->right);
	
	//free(root);
}


void huffman_compress(char *in_file, char *out_file){
	printf("Compress... input: %s, output: %s\n", in_file, out_file);
	//input: ../lib/api/pr_queue.h, output: /lib/api/pr_queue.huff !!!

	
	int in_fd, out_fd; //input, output file descriptors
	
	in_fd = open(in_file, O_RDONLY);
	if(in_fd < 0){
		perror("Error opening input file...");
		exit(1);
	}
	
	out_fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if(out_fd < 0){
		perror("Error opening output file...");
		exit(1);
	}
	
	
	init_char_map();
	
	
	//first traversal of the input file to gather input statistics
	scan_in_file(in_fd);
	
	//build encoding tree
	huff_node *root = build_huff_tree();
	
	//append huffman code to each leaf in a tree
	char *h_code = (char *)calloc(50, sizeof(char)); //huffman code for each character
	encode_huff_chars(root, h_code, 0);
	free(h_code);
	
	
	//second traversal of the input file to write encoded characters to output file
	
	lseek(in_fd, 0, SEEK_SET); //move back to the start of input file
	lseek(in_fd, 1, SEEK_SET); //reserve 1 byte at the start for number of encoding tree blocks
	unsigned int bit_buffer = 0;
	int bit_count = 0;
	encode_huff_tree(root, &bit_buffer, &bit_count, &block_count, out_fd);
	
	bit_buffer = bit_buffer << (32 - bit_count);
	unsigned char remainder = 32 - bit_count;
	if(remainder < 32){
		write(out_fd, &bit_buffer, sizeof(unsigned int));
		dec_to_bin(block_bin_code, bit_buffer, 32);
		printf("%s\n", block_bin_code);
		
		block_count++;	
	}
	printf("remainder: %d, bit_count: %d, block_count: %d\n",remainder, bit_count, block_count);
	
	//write number of encoding tree blocks to the reserved byte in the ouput file 
	lseek(out_fd, 0, SEEK_SET);
	write(out_fd, &block_count, sizeof(unsigned char));
	
	
	encode_content();
	
	//clean up
	free_huff_tree(root);
	
	//free_huff_chars(); free_huff_tree() does that?
	
	
	if(close(in_fd) < 0){
		perror("Error closing input file...");
		exit(2);
	}
	
	if(close(out_fd) < 0){
		perror("Error closing output file...");
		exit(2);
	}
}


