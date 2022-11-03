
//Main header file for Huffman compression and decompression algorithms


/*
	*Encodes the input file *in_file* with static Huffman coding algorithm
	*First byte in the encoded file *out_file* represents number (n) of 32-bit (unsigned int) encoding tree blocks
	*Then follows the n 32-bit blocks of encoding tree 
	*Finally, the encoded content is written and enclosed with pseudo EOF character.
*/
void huff_encode(char *in_file, char *out_file);

void huff_decode(char *in_file, char *out_file);
