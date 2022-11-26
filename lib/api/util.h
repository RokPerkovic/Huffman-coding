#include <stdint.h>

/*
	*utility function that converts *dec* value to binary string of size *size*.
*/
void dec_to_bin(char *bin, unsigned long dec, int size);

int16_t make_char(unsigned long **encoded_huff_tree, unsigned long *mask);

void read_block(int in_fd, unsigned long *content_block);

