#ifndef BIT

#define BIT

#include <stdint.h>

int read_bit(unsigned int *encoded_block, unsigned int *mask);

/*
	*receives binary string *bits* and writes it to output file *out_fd*.
	*stores number of bits *bit_count* in the *bit_buffer*.
	*stores number of encoded tree blocks in *block_count*.
	*when writing encoded content, *block_count* is not used.
*/

void write_bits(char* bits, int size, unsigned int *bit_buffer, int *bit_count, int *block_count, int out_fd);

#endif
