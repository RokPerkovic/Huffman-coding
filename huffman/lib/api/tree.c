#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "tree.h"
#include "pr_queue.h"

pr_queue pq;
huff_char_map huff_chars;
unsigned char input_buff[BUFF_SIZE + 1];

void init_char_map(){
	huff_chars.map = calloc(257, sizeof(huff_char *));
	huff_chars.capacity = 257;
	//insert a special huff_char that denotes the end of compressed content
	huff_char *pseudoEOF = malloc(sizeof(huff_char));
	pseudoEOF->c = 1000;
	pseudoEOF->weight = 1.0;
	huff_chars.map[256] = pseudoEOF;
	huff_chars.size = 1;
}

void scan_in_file(int in_fd){
	
	int n;
	while((n = read(in_fd, input_buff, BUFF_SIZE)) > 0){
		input_buff[n] = '\0';
		//printf("read: %d\n", n);
		//printf("%s", input_buff);
		for(int c = 0; c < n; c++){
			//printf("%c", input_buff[c]);
			if(huff_chars.map[(int)input_buff[c]]){
				//character already in the map. Increment occurance counter
				huff_chars.map[(int)input_buff[c]]->weight+=1.0;
			}
			else{
				//insert new, not yet seen character in the map
				huff_char *chr = malloc(sizeof(huff_char));
				chr->c = input_buff[c];
				chr->weight = 1.0;
				//printf("%d\n", (int)input_buff[c]);
				huff_chars.map[(int)input_buff[c]] = chr;
				//printf("%p\n", huff_chars.map[(int)input_buff[c]]);
				huff_chars.size++;
				//printf("%c\n", input_buff[c]);	
			}
		}	
	}
	//printf("\n\n");
	//printf("size: %d\n", huff_chars.size);
	
	/*for(int i = 0; i < 257; i++){
		if(huff_chars.map[i]){
			printf("%d\n ", huff_chars.map[i]->c);
		}	
	}*/
}

void build_heap(){
	for(int i = 0; i < 257; i++){
		if(huff_chars.map[i]){
			huff_node *node = create_huff_node(huff_chars.map[i]);
			
			enqueue(node, &pq);
			//printf("enqueue: %c\n", huff_chars.map[i]);
		}
	}
}

huff_node *create_huff_node(huff_char *c){

	huff_node *tmp = malloc(sizeof(huff_node));
	
	tmp->c = c;
	tmp->left = NULL;
	tmp->right = NULL;
	//printf("allocated: %c, %p\n", tmp->c->c, tmp);
	return tmp;
}


huff_node *build_huff_tree(){
	//build a heap of unique characters found in input
	pq.size = 0;
	pq.capacity = huff_chars.size + 1;
	pq.last_idx = 0;
	pq.heap = malloc(pq.capacity * sizeof(huff_node *));
	//printf("here %d\n", pq.capacity);
	build_heap();
	

	huff_node *left, *right, *parent;
	
	while(!(pq.size == 1)){
	
		huff_char *sum = malloc(sizeof(huff_char));
		sum->c = '$';
		
		left = dequeue(&pq);
		right = dequeue(&pq);
		//printf("%c %c\n", left->c->c, right->c->c);
		
		sum->weight = left->c->weight + right->c->weight;
		
		parent = create_huff_node(sum);
		parent->left = left;
		parent->right = right;
		
		enqueue(parent, &pq);
	}
	//vrne kazalec na root kopice
	return dequeue(&pq);
}


void encode(huff_node *root, char *h_code, int parent){
	if(root->left){
		h_code[parent] = '0';
		encode(root->left, h_code, parent+1);
	}
	if(root->right){
		h_code[parent] = '1';
		encode(root->right, h_code, parent+1);
	}
	if(!(root->left) && !(root->right)){
		//listu dodeli dvojisko kodo
		h_code[parent] = '\0';
		root->c->h_code = malloc((parent + 1) /*(strlen(h_code) + 1)*/ * sizeof(char));
		strcpy(root->c->h_code, h_code);
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
        
        //printf("Deleting Node : %c\n", root->c->c);
        free(root->c->h_code);
        free(root->c);
        free(root);
        return;
}
