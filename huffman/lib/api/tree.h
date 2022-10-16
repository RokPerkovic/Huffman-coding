#include <stdint.h>

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
	int size; //the number of elements in the map
	int capacity; //maximum number of elements the map can hold
} huff_char_map;


huff_node *create_huff_node(huff_char *c);

void free_huff_tree(huff_node *root);
