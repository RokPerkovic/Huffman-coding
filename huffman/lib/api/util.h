#include <stdint.h>

/*
	*utility function that converts *dec* value to binary string of size *size*.
*/
void dec_to_bin(char *bin, unsigned int dec, int size);

int16_t make_char(unsigned int **encoded_huff_tree, unsigned int *mask);

void read_block(int in_fd, unsigned int *content_block);

