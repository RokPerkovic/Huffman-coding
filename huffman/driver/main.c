/*
	Driver code for Huffman compression, decompression program
	Arguments: input file(-i <filename>), output file(-o <filename>), mode(-c: compress, -d: decompress)
*/

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <unistd.h>
#include <sys/stat.h>


#include "../lib/api/huffman.h"

struct stat file_info;

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
	
	//currently just appends .husff to the input file name
	strcpy(filename, name /*strtok(tmp, ".")*/);
		
	if(mode == 1){
		strcat(filename, ".huff");
	}
	else{
		filename[name_len - 5] = '\0';
	}
	
	//free(tmp);
}


void print_stats(float time_elapsed, float saved /* compression_ratio, ... */){
	printf("\n***Statistics****\n");


	if(saved >= 100000){
		saved/=1000000;
		printf("Saved: %.2f MB\n", saved);
	}
	else if(saved >= 1000){
		saved/=1000;
		printf("Saved: %.2f kB\n", saved);
	}
	else{
		printf("Saved: %.2f B\n", saved);
	}
	
	if(time_elapsed < 1){
		printf("time elapsed: %.2f ms\n", time_elapsed * 1000);
	}
	else{
		printf("time elapsed: %.2f s\n", time_elapsed);
	}	
	
	
}

double get_compression_ratio(char *in_file, char *out_file){
	float in_file_size, out_file_size;
	float saved = 0.0;
	
	if(stat(in_file, &file_info) < 0){
		perror("Could not retrieve input file info");
	}
	
	in_file_size = file_info.st_size;
	
	if(stat(out_file, &file_info) < 0){
		perror("Could not retrieve output file info");
	}
	
	out_file_size = file_info.st_size;
	saved = in_file_size - out_file_size;
	
	return saved;
		
}

int main(int argc, char *args[]){

	if(argc < 2){
		help();
		exit(1);
	}
	
	char *in_file = NULL;
	char *out_file = NULL;
	char default_out_file[100];
	char replace_answer;
	
	
	int encode = 0, decode = 0; //mode option flags
	int stats = 0;
	
	int opt;
	
	while((opt = getopt(argc, args, "ci:o:dhgs")) >= 0){
		switch(opt){
			case 'c':
				if(decode){
					help();
				}
				encode = 1;
			 	break;
			case 'd':
				if(encode){
					help();
				}
				decode = 1;
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
					If provided, they will be passed to GUI.
				*/
				break;
				
			case 's': 
				stats = 1;
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
	
	
	if(encode){
		if(out_file == NULL){
			set_default_out_file(default_out_file, in_file, 1);
			out_file = default_out_file;
		}
		
		//check if a file with the same name as out_file already exists
		if (access(out_file, F_OK) == 0){
			//file exists
			printf("File %s already exists.\n", out_file);
			printf("Do you want to replace it? (y/n)\n");
			scanf("%c", &replace_answer);
			if(replace_answer == 'n'){
				printf("exit!\n");
				exit(2);
			}
		}
		
		float s = omp_get_wtime();
		huff_encode(in_file, out_file);
		float e = omp_get_wtime();
		
		//TODO: compression ratio, saved space, ...
		
		if(stats){
			float time_elapsed = e - s;
			float compr = get_compression_ratio(in_file, out_file);
			print_stats(time_elapsed, compr);
		}
		
		
	}
	else if(decode){
		if(out_file == NULL){
			set_default_out_file(default_out_file, in_file, 2);
			out_file = default_out_file;
		}
		
		//check if a file with the same name as out_file already exists
		if (access(out_file, F_OK) == 0){
			//file exists
			printf("File %s already exists.\n", out_file);
			printf("Do you want to replace it? (y/n)\n");
			scanf("%c", &replace_answer);
			if(replace_answer == 'n'){
				printf("exit!\n");
				exit(2);
			}
		}
		
		float s = omp_get_wtime();
		huff_decode(in_file, out_file);
		float e = omp_get_wtime();
		
		if(stats){
			float time_elapsed = e - s;
			print_stats(time_elapsed, 0);
		}
	}
	else{
		help();
	}

	return 0;
}
