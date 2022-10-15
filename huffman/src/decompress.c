#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <omp.h>

#define BUFF_SIZE 512

struct character{
	int16_t c;
	char *hcode;
	double weight;
};

struct hNode{
	struct character *c;
	struct hNode *left, *right;
};


struct hNode *createhNode(struct character *c){
	struct hNode *tmp = malloc(sizeof(struct hNode));
	
	tmp->c = c;
	tmp->left = NULL;
	tmp->right = NULL;
	
	return tmp;
}

char binBlock[32];
//pomozna funkcija za pregled binarnega niza trenutnega bloka
void blockToBin(int c){

    for (int i = 31; i >= 0; --i){
    	binBlock[31 - i] = (c & (1 << i)) ? '1' : '0';
    }
}


unsigned int mask = 1 << 31;
unsigned int block;
unsigned char treeBlocksRead = 0;
unsigned char treeBlocks;
int fd, fd1; //in, out file descriptors
int16_t ch = 0;
int flag = 0; // 1 --> nepopoln znak, ostanek; 0 --> ok
int c_global; //pozicija v trenutnem, nepopolnem znaku

int readBlock(){
	int n;
	
	if((n = read(fd, &block, sizeof(unsigned int))) < 0){
		perror("Error while reading compressed file...");	
	}
	/*blockToBin(block);
	printf("block: %s\n", binBlock);	*/
	treeBlocksRead++; //uporabljeno samo pri branju drevesa
	return n;
}


int16_t makeChar(){
	int c;

	if(flag){
		//continue char
		for(c = c_global; c >= 0; c--){
			if(block & mask){
				ch+=pow(2, c);
			}
			mask = mask >> 1;
					
			if(mask <= 0){
				//maska je prisla do najbolj levega bita v bloku
				break;
			}	
		}
	}
	else{
		ch = 0;
		for(c = 15; c >= 0; c--){	
			if(block & mask){
				ch+=pow(2, c);
			}
			mask = mask >> 1;
					
			if(mask <= 0){
				break;
			}	
		}
	}
	
	if(c <= 0){
		//sestavljen celoten znak
		//printf("ch: %d, %c,  c: %d\n", ch, ch, c);
		flag = 0;
		return ch;
	}
	else{
		//manjkajo biti v trenutnem bloku
		c_global = c - 1; //kje smo ostali z gradnjo znaka
		
		return -1;			
	}
	
}

//funkcija za branje bitov za gradnjo drevesa
int readBit(){
	//blockToBin(mask);
	//printf("maska: %s\n", binBlock);
	if(block & mask){ //preveri geometrijo drevesa (list / notranje vozlisce)
		mask = mask >> 1; //list --> postavi masko na prvi bit znaka
		
		if(mask <= 0){
			//znak se zacne v naslednjem bloku
			ch = 0;
			flag = 1;
			c_global = 15;
		}
		return 1;
	}
	mask = mask >> 1;
	return 0;
}

//funkcija za branje bitov pri dekodiranju vsebine
int readBit1(){
	//blockToBin(mask);
	//printf("maska: %s\n", binBlock);
	if(block & mask){ 
		mask = mask >> 1;
		if(mask <= 0){
			flag = 1;
		}
		return 1;
	}
	mask = mask >> 1;
	if(mask <= 0){
		flag = 1;
	}
	return 0;
}




struct hNode *ReadNode(){
    if(mask <= 0){
    	if(!(treeBlocksRead >= treeBlocks)){
    		//zadnji blok drevesa --> ne sme gradit drevesa po ostanku
    		readBlock();
    		mask = 1 << 31;
    	}
    	else{
    		//printf("Last one!\n");
    		
    		return NULL;
    	}
    }
    
    if (readBit() == 1){
    	//read(fd, 2, sizeof(char)); ...
    	//list
    	if(flag){ //znak se zacne v naslednjem bloku
    		readBlock();
    		mask = 1 << 31;
    	}
    	int16_t c;
    	if((c = makeChar()) < 0){
		flag = 1; //delno sestavljen znak
		
		//ostanek na koncu zadnjega bloka drevesa!
		readBlock();
		
		//ponastavi masko
		mask = 1 << 31;
		c = makeChar();

		struct character *chr = malloc(sizeof(struct character));
	    	chr->c = c;
		struct hNode *leaf = createhNode(chr);
		
		//printf("%d\n", c);
		
		
	    	return leaf;
					
	}
	else{
		flag = 0;
		struct character *chr = malloc(sizeof(struct character));
	    	chr->c = c;
		struct hNode *leaf = createhNode(chr);
		
		//printf("%d\n", c);
		
	    	return leaf;
	}
    	
    }
    else{
    	//notranje vozlisce
	struct hNode *leftChild = ReadNode();
	struct hNode *rightChild = ReadNode();
	struct hNode *root = malloc(sizeof(struct hNode));
	root->left = leftChild;
	root->right = rightChild;
	
	return root;
    }
}

int leaf = 0;
int bit;
int contentEnd;
struct hNode *tmp;
int eof = 0;
char buff[BUFF_SIZE];
int buff_pos = 0;
void traverseHTree(struct hNode *root){
	if(flag){
		root = tmp;
		flag = 0;
	}

	if(root == NULL){
		printf("null\n");
		return;
	}
	
	if(!(root->left && root->right)){
		if(root->c->c == 1000){ //pseudo eof
			eof = 1;
			leaf=1;
			//printf("%d, %c\n", root->c->c, root->c->c);
			return;
		}
		//printf("%c", root->c->c);
		
		buff[buff_pos] = root->c->c; 
		buff_pos++;
		if(buff_pos == (BUFF_SIZE - 1)){
			buff[BUFF_SIZE - 1] = '\0';
			buff_pos = 0;
			//printf("%s", buff);
			write(fd1, &buff, sizeof(buff) - 1);
		}
		
		leaf=1;
	}
	
	if(!leaf){ //vracanje rekurzije? ponovno klice readBit()
		if((bit = readBit1()) == 1){
			if(flag){
				//kodna zamenjava se nadaljuje v naslednjem bloku
				//readBit1 prebere 1 in nastavi flag
				tmp = root->right;
				//flag = 0;	
				return;
			}
			traverseHTree(root->right);
		}
		else{
			if(flag){
				//kodna zamenjava se nadaljuje v naslednjem bloku
				//readBit1 prebere 0 in nastavi flag
				tmp = root->left;
				//flag = 0;	
				return;
			}
			traverseHTree(root->left);
		}
	}
}	


void decompress(struct hNode *root){
	while(!eof){
		traverseHTree(root);
		leaf = 0;
		//blockToBin(mask);
		//printf("maska: %s\n", binBlock);
		
		if(mask <= 0){
			mask = 1 << 31;
			readBlock();
		}
	}	
}


void deallocateHTree(struct hNode *root){
	if(root == NULL){
		return;
	}
        deallocateHTree(root->left);
        deallocateHTree(root->right);
        
        free(root->c);
        free(root);
        
        return;
}

int main(int argc, char *argv[]){
	//printf("%s %s %s\n", argv[0], argv[1], argv[2]);
	//double s = omp_get_wtime();
	if (argc < 2){ 
		fprintf(stderr,"Decompress usage: src_FileName [dst_FileName]\n");
		exit(1);
	}
	char *dstFileName;
	if(argv[2] != NULL){
		//podano je zeljeno ime razpihnjene datoteke
		dstFileName = argv[2];
	}
	else{
		dstFileName = malloc(13 * sizeof(char));
		strcpy(dstFileName, "decompressed");
		dstFileName[12] = '\0';
	}
	
	fd = open(argv[1], O_RDONLY);
	if(fd < 0){
		perror("Decompress: Error opening compressed file...\n");
		exit(1);
	}
	
	fd1 = open(dstFileName, O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if(fd1 < 0){
		perror("Decompress: Error opening decompressed file...\n");
		exit(1);
	}

	int tb = read(fd, &treeBlocks, sizeof(unsigned char));
	if(tb < 0){
		perror("Decompress: Error reading tree blocks\n");
		exit(1);
	}
	//TODO: ugotovit da je podan file res zakodiran s huffmanom
	//printf("st. blokov drevesa: %d\n", treeBlocks);
	
	printf("Decompressing...\n");
	//drevo
	readBlock();
	struct hNode *root = ReadNode(); //zgradi drevo iz stisnjene vsebine in vrne kazalec na koren drevesa
	double e = omp_get_wtime();
	//printf("rekonstrukcija drevesa: %fs.\n", e-s);
	//vsebina
	flag = 0;
	mask = 1 << 31;
	readBlock();
	decompress(root); //razpihne stisnjeno vsebino
	
	//ostanek
	if(buff_pos > 0){
		buff[buff_pos] = '\0';
		int ost = write(fd1, buff, buff_pos);
		//printf("written: %d bytes\n", ost);
		if(ost < 0){
			perror("Error writing ostanek\n");
			exit(1);	
		}
	}
	
	deallocateHTree(root);
	
	if(close(fd) < 0){
		perror("Error closing compressed file...\n");
		exit(1);
	}
	
	if(close(fd1) < 0){
		perror("Error closing decompressed file...\n");
		exit(1);
	}
	printf("Done!\n");
	
	return 0;
}
