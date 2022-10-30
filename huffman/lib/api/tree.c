#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "bit.h"
#include "tree.h"
#include "util.h"

#include "pr_queue.h"

pr_queue pq;
huff_char_map huff_chars;
unsigned char input_buff[BUFF_SIZE + 1];
char char_bin_code[17];
char block_bin_code1[33];

unsigned int tree_block;
unsigned int mask = 2147483648; // 1 << 31


void init_char_map(){
	huff_chars.map = calloc(257, sizeof(huff_char *));
	huff_chars.capacity = 257;
	//insert a special huff_char that denotes the end of compressed content
	huff_char *pseudoEOF = calloc(1, sizeof(huff_char));
	pseudoEOF->c = 1000;
	pseudoEOF->weight = 1.0;
	huff_chars.map[256] = pseudoEOF;
	huff_chars.size = 1;
}

void scan_in_file(int in_fd){
	
	int n;
	while((n = read(in_fd, input_buff, BUFF_SIZE)) > 0){
		if(n < 0){
			perror("Error reading input file...");
			exit(2);
		}
		input_buff[n] = '\0';
		for(int c = 0; c < n; c++){
			if(huff_chars.map[(int)input_buff[c]]){
				//character already in the map. Increment occurance counter
				huff_chars.map[(int)input_buff[c]]->weight+=1.0;
			}
			else{
				//insert new, not yet seen character in the map
				huff_char *chr = calloc(1, sizeof(huff_char));
				chr->c = input_buff[c];
				chr->weight = 1.0;
				
				huff_chars.map[(int)input_buff[c]] = chr;
				huff_chars.size++;
			}
		}	
	}
}

void build_heap(){
	for(int i = 0; i < 257; i++){
		if(huff_chars.map[i]){
			huff_node *node = create_huff_node(huff_chars.map[i]);
			enqueue(node, &pq);
		}
	}
}

huff_node *create_huff_node(huff_char *c){

	huff_node *tmp = calloc(1, sizeof(huff_node));
	
	tmp->c = c;
	tmp->left = NULL;
	tmp->right = NULL;
	
	return tmp;
}


huff_node *build_huff_tree(){
	//build a heap of unique characters found in input
	pq.size = 0;
	pq.capacity = huff_chars.size + 1;
	pq.last_idx = 0;
	pq.heap = calloc(pq.capacity, sizeof(huff_node *));
	build_heap();
	

	huff_node *left, *right, *parent;
	
	while(!(pq.size == 1)){
	
		huff_char *sum = calloc(1, sizeof(huff_char));
		sum->c = '$';
		
		left = dequeue(&pq);
		right = dequeue(&pq);
		
		sum->weight = left->c->weight + right->c->weight;
		
		parent = create_huff_node(sum);
		parent->left = left;
		parent->right = right;
		
		enqueue(parent, &pq);
	}
	return dequeue(&pq);
}


huff_node *rebuild_huff_tree(unsigned int *encoded_huff_tree, int block_count){
	if(mask <= 0){
		//16-bit char starts in new 32-bit block
		//advance in array of tree blocks and continue
		encoded_huff_tree++;
	}
	

	if(read_bit(encoded_huff_tree, &mask) == 1){
		//make_char();
	}
	else{
		huff_node *left = rebuild_huff_tree(encoded_huff_tree, block_count);
		huff_node *right = rebuild_huff_tree(encoded_huff_tree, block_count);
		huff_node *root = malloc(sizeof(huff_node));
		
		root->left = left;
		root->right = right;
		
		return root;
	}
	
	return NULL;
}


void encode_huff_chars(huff_node *root, char *h_code, int parent){
	if(root->left){
		h_code[parent] = '0';
		encode_huff_chars(root->left, h_code, parent+1);
	}
	if(root->right){
		h_code[parent] = '1';
		encode_huff_chars(root->right, h_code, parent+1);
	}
	if(!(root->left) && !(root->right)){
		h_code[parent] = '\0';
		root->c->h_code = calloc((parent + 1) /*(strlen(h_code) + 1)*/, sizeof(char));
		strcpy(root->c->h_code, h_code);
		root->c->h_code_len = parent;
	}
}


void encode_huff_tree(huff_node *root, unsigned int *bit_buffer, int *bit_count, int *block_count, int out_fd){
	if(root == NULL){
		return;
	}
	
	if(!(root->left) && !(root->right)){
		//leaf
		char bit = '1';
		dec_to_bin(char_bin_code, root->c->c, 16);

		write_bits(&bit, 1, bit_buffer, bit_count, block_count, out_fd);
		write_bits(char_bin_code, 16, bit_buffer, bit_count, block_count, out_fd);
	}
	else{
		//non-leaf node
		char bit = '0';
		write_bits(&bit, 1, bit_buffer, bit_count, block_count, out_fd);
	}
	
	encode_huff_tree(root->left, bit_buffer, bit_count, block_count, out_fd);
	encode_huff_tree(root->right, bit_buffer, bit_count, block_count, out_fd);
}

void encode_content(unsigned int *bit_buffer, int *bit_count, int *block_count, int in_fd, int out_fd){
	int n;
	while((n = read(in_fd, input_buff, BUFF_SIZE)) > 0){
		if(n < 0){
			perror("Error reading input file...");
			exit(2);
		}
	
		input_buff[n] = '\0';
		
		for(int c = 0; c < n; c++){
			huff_char *hc = huff_chars.map[input_buff[c]];
			write_bits(hc->h_code, hc->h_code_len, bit_buffer, bit_count, block_count, out_fd);	
		}	
	}
	
	huff_char *p_eof = huff_chars.map[256];
	write_bits(p_eof->h_code, p_eof->h_code_len, bit_buffer, bit_count, block_count, out_fd);
	
	int remainder = 32 - *bit_count;
	
	if(remainder < 32){
		*bit_buffer = *bit_buffer << (32 - *bit_count);
		write(out_fd, bit_buffer, sizeof(unsigned int));
	
		/*dec_to_bin(block_bin_code1, *bit_buffer, 32);
		printf("%s\n", block_bin_code1);*/
	}
}


void free_huff_chars(){
	for(int c = 0; c < 257; c++){
		if(huff_chars.map[c]){
			free(huff_chars.map[c]->h_code);
		}
		free(huff_chars.map[c]);
	}
}


void free_huff_tree(huff_node *root){
	if(root == NULL){
		return;
	}
        free_huff_tree(root->left);
        free_huff_tree(root->right);
        
        if(root->c->h_code){
        	free(root->c->h_code);
        }
        free(root->c);
        free(root);
        return;
}
