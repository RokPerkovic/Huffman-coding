
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "../api/huffman.h"
#include "../api/tree.h"
#include "../api/util.h"

int block_count = 0;
unsigned int bit_buffer = 0;
int bit_count = 0;
char block_bin_code[33];
unsigned char input_buff[BUFF_SIZE + 1];

unsigned char huff_tree_blocks;


void huff_encode(char *in_file, char *out_file){
	printf("Encoding...\n input: %s\n", in_file);
	
	int in_fd, out_fd; //input, output file descriptors
	char replace_answer;
	
	in_fd = open(in_file, O_RDONLY);
	if(in_fd < 0){
		perror("Error opening input file...");
		exit(1);
	}
	
	//check if a file with the same name as out_file already exists
	if (access(out_file, F_OK) == 0){
		//file exists
		printf("File %s already exists.\n", out_file);
		printf("Do you want to replace it? (y/n)\n");
		scanf("%c", &replace_answer);
		if(replace_answer == 'n'){
			printf("exit!\n");
			exit(2);
		}
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
	lseek(out_fd, 1, SEEK_SET); //reserve 1 byte at the start for number of encoding tree blocks
	
	encode_huff_tree(root, &bit_buffer, &bit_count, &block_count, out_fd);
	
	bit_buffer = bit_buffer << (32 - bit_count);
	unsigned char remainder = 32 - bit_count;
	if(remainder < 32){
		write(out_fd, &bit_buffer, sizeof(unsigned int));
		/*dec_to_bin(block_bin_code, bit_buffer, 32);
		printf("%s\n", block_bin_code);*/
		block_count++;	
	}

	
	//write the number of encoding tree blocks to the reserved byte in the ouput file 
	lseek(out_fd, 0, SEEK_SET);
	write(out_fd, &block_count, sizeof(unsigned char));
	//printf("num of huff_tree blocks: %d\n", block_count);
	
	bit_count = 0;
	bit_buffer = 0;
	
	lseek(out_fd, 0, SEEK_END);
	lseek(in_fd, 0, SEEK_SET);
	
	encode_content(&bit_buffer, &bit_count, &block_count, in_fd, out_fd);
	
	//traverse_huff_tree(root);
	
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
	
	printf("Done!\n output: %s\n", out_file);
}


void huff_decode(char *in_file, char *out_file){
	printf("Decoding...\n input: %s\n", in_file);
	char replace_answer;
	
	int in_fd, out_fd; //input, output file descriptors
	
	in_fd = open(in_file, O_RDONLY);
	if(in_fd < 0){
		perror("Error opening input file...");
		exit(1);
	}
	
	//check if a file with the same name as out_file already exists
	if (access(out_file, F_OK) == 0){
		//file exists
		printf("File %s already exists.\n", out_file);
		printf("Do you want to replace it? (y/n)\n");
		scanf("%c", &replace_answer);
		if(replace_answer == 'n'){
			printf("exit!\n");
			exit(2);
		}
	}
	
	out_fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if(out_fd < 0){
		perror("Error opening output file...");
		exit(1);
	}
	
	//Read in the first byte of the encoded file which represents the number of 32-bit encoding tree blocks
	int rb;
	if((rb = read(in_fd, &huff_tree_blocks, sizeof(unsigned char))) < 0){
		perror("Error reading huff tree blocks\n");
		exit(1);
	}
	
	//printf("num of huff_tree blocks: %d\n", huff_tree_blocks);
	
	
	//allocate huff_tree_blocks * sizeof(unsigned int) array and read the whole encoded tree into array. 
	
	unsigned int *encoded_huff_tree = malloc(huff_tree_blocks * sizeof(unsigned int));
	unsigned int *tmp = encoded_huff_tree;
	if((rb = read(in_fd, encoded_huff_tree, huff_tree_blocks * sizeof(unsigned int))) < 0){
		perror("Error reading encoded tree...");
		exit(2);
	}
	
	
	huff_node *root = rebuild_huff_tree(&encoded_huff_tree);	
	//traverse_huff_tree(root);

	//TODO: where to reset mask after rebuilding encoding tree to make it reusable...
	unsigned int content_mask = 0;
	
	char output_buff[BUFF_SIZE];
	int buff_pos = 0;

	decode_content(root, &content_mask, in_fd, output_buff, &buff_pos, out_fd);
	
	if(buff_pos > 0){
		//left over
		output_buff[buff_pos] = '\0';
		write(out_fd, output_buff, buff_pos);
		//printf("%s", output_buff);
	}

	free(tmp);
	
	free_huff_tree(root);
	
	if(close(in_fd) < 0){
		perror("Error closing input file...");
		exit(2);
	}
	
	if(close(out_fd) < 0){
		perror("Error closing output file...");
		exit(2);
	}
	
	printf("Done!\n output: %s\n", out_file);
}



