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


/*
	*allocates memory for a map of unique characters that appear in the input file
	*inserts a special EOF huff_char that denotes end of encoded content in the encoded file
*/
void init_char_map();

/*
	*reads the input file
	*fills the huff_char_map with unique characters that appear in the input file
*/

void scan_in_file(int in_fd);

/*
	*allocates memory for a new encoding tree node
	*stores a character c in the node	
	
*/

huff_node *create_huff_node(huff_char *c);

/*
	*builds heap of unique characters that appear in the input file
	*uses dequeue operation on the heap to get left and right child of the root
*/

huff_node *build_huff_tree();


/*
	*traverses the encoding tree from root to leaves
	*builds a string of 1s and 0s representing code for the leaf node (h_code)	
	*parent denotes the encoding of the previous node
	
*/
void encode_huff_chars(huff_node *root, char *h_code, int parent);

/*
	*writes out the encoded coding tree with preorder traversal
	 at the beggining of the output file
	*non-leaf nodes are encoded as 0-bit
	*leaf nodes are encoded as 1-bit followed by 16-bit representation of the character they store
*/

void encode_huff_tree(huff_node *root, unsigned int *bit_buffer, int *bit_count, int *block_count, int in_fd);

/*
	allocated memory clean up
*/

void free_huff_chars();

void free_huff_tree(huff_node *root);

#endif
