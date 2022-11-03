#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

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
unsigned int content_block;
unsigned int mask = 2147483648; // 1 << 31
int blocks_read = 0;


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
				//printf("%d\n", input_buff[c]);
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


void traverse_huff_tree(huff_node *root){
	if(root == NULL){
		return;
	}
	
	if(!root->left){
		printf("%d, %s, %d\n", root->c->c, root->c->h_code, root->c->h_code_len);
	}
	
	traverse_huff_tree(root->left);
	traverse_huff_tree(root->right);
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


huff_node *rebuild_huff_tree(unsigned int **encoded_huff_tree){
	//printf("call...\n");
	
	if(read_bit(*encoded_huff_tree, &mask) == 1){
		mask = mask >> 1; // place the mask bit on the first bit of the following character
		if(mask == 0){
			*encoded_huff_tree = *encoded_huff_tree + 1;
			mask = 1 << 31;
			//blocks_read++;		
		}
		
		int16_t chr = make_char(encoded_huff_tree, &mask); //read next 16 bits and construct a character
		
		//printf("%c, %d\n", chr, chr);
		huff_char *character = malloc(sizeof(huff_char));
		character->c = chr;
		huff_node *leaf = create_huff_node(character);
		
		return leaf;
	}
	else{
		
		mask = mask >> 1;
		if(mask == 0){
			*encoded_huff_tree = *encoded_huff_tree + 1;
			mask = 1 << 31;	
		}
		
		huff_node *left = rebuild_huff_tree(encoded_huff_tree);
		huff_node *right = rebuild_huff_tree(encoded_huff_tree);
		huff_node *root = malloc(sizeof(huff_node));
		
		
		root->left = left;
		root->right = right;
		
		return root;
	}
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
		root->c->h_code = calloc((parent + 1), sizeof(char));
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



int16_t decode_char(huff_node *root, unsigned int *mask, int in_fd){
	
	if(*mask == 0){
		read_block(in_fd, &content_block);
		*mask = 1 << 31;
	}
	
	if(!root->left && !root->right){
		//leaf
		return root->c->c;
	}
	
	
	if(read_bit(&content_block, mask) == 1){
		//move to the right
		*mask = *mask >> 1;
		return decode_char(root->right, mask, in_fd);
	}
	else{
		//move to the left
		*mask = *mask >> 1;
		return decode_char(root->left, mask, in_fd);
	}
}


void decode_content(huff_node *root, unsigned int *mask, int in_fd, char *output_buff, int *buff_pos, int out_fd){
	
	int16_t c;
	
	while((c = decode_char(root, mask, in_fd)) != 1000){
		output_buff[*buff_pos] = c;
	
		if(*buff_pos == (BUFF_SIZE - 1)){
			output_buff[*buff_pos] = c;
			output_buff[BUFF_SIZE] = '\0';
			write(out_fd, output_buff, BUFF_SIZE);
			*buff_pos = 0;
		}
		else{
			*buff_pos = *buff_pos + 1;
		}	
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
        
        if(root->c && root->c->h_code){
        	free(root->c->h_code);
        }
       
        free(root->c);
        free(root);
        return;
}
