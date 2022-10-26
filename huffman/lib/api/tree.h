#ifndef TREE
#define TREE 

#include <stdint.h>
//#include "pr_queue.h"

#define BUFF_SIZE 512



typedef struct _character{
	int16_t c;
	char *h_code;
	double weight;
} huff_char;


typedef struct _h_node{
	huff_char *c;
	struct _h_node *left, *right;
} huff_node;


typedef struct _huff_char{
	huff_char **map; /*
	Map of characters that occur in the input file.
	Characters are placed on the indexes corresponding to their decimal value ('a'->97).
	*/
	int size; //the number of elements currently in the map
	int capacity; //maximum number of elements the map can hold
} huff_char_map;


void init_char_map();

void scan_in_file(int in_fd);

huff_node *create_huff_node(huff_char *c);

huff_node *build_huff_tree();

void encode(huff_node *root, char *h_code, int parent);

void free_huff_chars();

void free_huff_tree(huff_node *root);

#endif
