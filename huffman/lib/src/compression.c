
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "../api/huffman.h"
#include "../api/tree.h"
#include "../api/pr_queue.h"
//#include "../api/util.h"

#define BUFF_SIZE 512

huff_char_map huff_chars;
pr_queue pq;
unsigned char input_buff[BUFF_SIZE];

void scan_in_file(int in_fd){
	int n;
	while((n = read(in_fd, input_buff, BUFF_SIZE)) > 0){
		input_buff[n] = '\0';
		printf("%s", input_buff);
		for(int c = 0; c < n; c++){
			if(huff_chars.map[(int)input_buff[c]] != NULL){
				//character already in the map. Increment occurance counter
				huff_chars.map[(int)input_buff[c]]->weight+=1.0;
			}
			else{
				//insert new, not yet seen character in the map
				huff_char *chr = malloc(sizeof(huff_char));
				chr->c = input_buff[c];
				chr->weight = 1.0;
				huff_chars.map[(int)input_buff[c]] = chr;
				huff_chars.size++;	
			}
		}	
	}
}

void free_huff_chars(huff_char **map){
	for(int c = 0; c < 257; c++){
		if(map[c]){
			free(map[c]->h_code);
		}
		free(map[c]);
	}
}

void huffman_compress(char *in_file, char *out_file){
	//printf("Compress... input: %s, output: %s\n", in_file, out_file);
	
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
	
	
	huff_chars.map = malloc(257 * sizeof(huff_char *));
	huff_chars.capacity = 257;
	
	
	//insert a special huff_char that denotes the end of compressed content
	huff_char *pseudoEOF = malloc(sizeof(huff_char));
	pseudoEOF->c = 1000;
	pseudoEOF->weight = 1.0;
	huff_chars.map[256] = pseudoEOF;
	huff_chars.size = 1;
	
	//first traversal of the input file to gather input statistics
	scan_in_file(in_fd);
	
	pq.size = 0;
	pq.capacity = huff_chars.size;
	pq.last_idx = 0;
	pq.heap = malloc(pq.capacity * sizeof(huff_node *));
	
	
	
	
	//clean up
	//free_huff_tree();
	
	free_huff_chars(huff_chars.map);
	free(pq.heap);
	
	if(close(in_fd) < 0){
		perror("Error closing input file...");
		exit(2);
	}
	
	if(close(out_fd) < 0){
		perror("Error closing output file...");
		exit(2);
	}
}
