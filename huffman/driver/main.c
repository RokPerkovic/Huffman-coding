/*
	Driver code for Huffman compression, decompression program
	Arguments: input file(-i <filename>), output file(-o <filename>), mode(-c: compress, -d: decompress)
*/

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

void help(){
	fprintf(stderr, "Usage: huff -c <filename> [-i <filename>] [-o <filename>] | huff -d <filename> [-i <filename>] [-o <filename>]\n");
	exit(2);
}

int main(int argc, char *args[]){
	
	if(argc < 3){
		help();
		exit(1);
	}
	
	char *in_file = NULL;
	char *out_file = NULL;
	int in_fd, out_fd; //input, output file descriptors
	int compress = 0, decompress = 0; //mode option flags
	
	int opt;
	
	while((opt = getopt(argc, args, "ci::o:dh")) >= 0){
		switch(opt){
			case 'c':
				if(decompress){
					help();
				}
				compress = 1;
			 	break;
			case 'd':
				if(compress){
					help();
				}
				decompress = 1;
			 	break;
			case 'i':
				in_file = optarg;
				break;
			case 'o':
				out_file = optarg;
				break;
			case 'h': help(); break;
		}
	}
	
	if (optind == argc - 1 && in_file == NULL) {
		in_file = args[optind];
		
	}
	else{
		//no input file specified
		help();
	}
	
	if(compress){
		//printf("compress!\n");
		huffman_compress();
	}
	else if(decompress){
		printf("decompress!\n");
		huffman_decompress();
	}
	else{
		help();
	}

	return 0;
}
