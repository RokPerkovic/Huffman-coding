/*
	Driver code for Huffman compression, decompression program
	Arguments: input file(-i <filename>), output file(-o <filename>), mode(-c: compress, -d: decompress)
*/

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/api/huffman.h"



void help(){
	fprintf(stderr, "Usage: huff -c <filename> [-i <filename>] [-o <filename>] | huff -d <filename> [-i <filename>] [-o <filename>]\n");
	exit(2);
}

//filename: pointer to filename to be set to default, mode (1: compress, 2: decompress)
void set_default_out_file(char *filename, char *name, int mode){
	int name_len = strlen(name);
	//char *tmp;// = malloc(name_len * sizeof(char));
	//strcpy(tmp, name); //strtok modifies original string
	//tmp = name;
	
	//currently just appends .huff to the input file name
	strcpy(filename, name /*strtok(tmp, ".")*/);
		
	if(mode == 1){
		strcat(filename, ".huff");
	}
	else{
		filename[name_len - 5] = '\0';
	}
	
	//free(tmp);
}

int main(int argc, char *args[]){

	if(argc < 2){
		help();
		exit(1);
	}
	
	char *in_file = NULL;
	char *out_file = NULL;
	char default_out_file[100];
	
	
	int compress = 0, decompress = 0; //mode option flags
	
	int opt;
	
	while((opt = getopt(argc, args, "ci:o:dhgs")) >= 0){
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
				
			case 'g':
				printf("GUI\n");
				/*	
					Start in GUI mode.
					TODO: In this case, other parameters/options are not required.
					If provided, it will be passed to GUI.
				*/
				break;
				
			case 's': 
				printf("statistics...\n");
				/*
					TODO: Print result statistics (compression ratio, time elapsed)...,
					Print progress...,
					encodings (tree, content, ...) ..., 
					Encoded tree size, encoded content size, ...
				*/
				
				break;
				
			case 'h': help(); break;
		}
	}
	
	if (optind == argc - 1) {
		in_file = args[optind];
	}
	else if(in_file == NULL){
		//no input file specified
		help();
	}
	
	
	if(compress){
		if(out_file == NULL){
			set_default_out_file(default_out_file, in_file, 1);
			out_file = default_out_file;
		}
		
		//printf("Compress... input: %s, output: %s\n", in_file, out_file);
		huff_compress(in_file, out_file);
		
		
	}
	else if(decompress){
		if(out_file == NULL){
			set_default_out_file(default_out_file, in_file, 2);
			out_file = default_out_file;
		}
		
		//printf("Decompress... input: %s, output: %s\n", in_file, out_file);
		huff_decompress(in_file, out_file);
	}
	else{
		help();
	}

	return 0;
}
