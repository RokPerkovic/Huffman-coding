#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>
#include <time.h>


//znake zlozimo v priorityQueue...prednost ima najmanjsi element... implementacija priorityQueue je najbolj ucinkovita z binary heap
//ko gradis huffmanovo drevo, rabis najdet 2 najmanjsa znaka... klices dequeue nad priorityQueue...O(logn) --> minHeap vrne najmanjsi element(extract min)
//https://www.youtube.com/watch?v=UxFDM9mbIbA
//https://wiki.lokar.fmf.uni-lj.si/r2wiki/index.php/Huffmanovo_kodiranje

#define BUFF_SIZE 512 //velikost bufferja za branje datoteke

struct character{
	int16_t c;
	char *hcode;
	double weight;
};

struct hNode{
	struct character *c;
	struct hNode *left, *right;
};

struct priorityQueue{
	int size; //the number of elements in the queue
	int capacity; //maximum number of elements the queue can hold
	struct hNode **heap; //kopica, implementirana z tabelo
	int lastIdx; //index zadnjega elementa
};

struct hNode *createhNode(struct character *c){
	struct hNode *tmp = malloc(sizeof(struct hNode));
	
	tmp->c = c;
	tmp->left = NULL;
	tmp->right = NULL;
	
	return tmp;
}


//operacije nad binary heap
//vrne indeks starsa za podani indeks v kopici
int parent(int idx){
	if(idx <= 1){
		return 0;
	}
	return idx / 2;
}

//zamenja elementa na podanih indeksih v kopici
void swap(int idx1, int idx2, struct priorityQueue *pq){
	struct hNode *tmp = pq->heap[idx1];
	pq->heap[idx1] = pq->heap[idx2];
	pq->heap[idx2] = tmp;
}

//vrne indeks levega otroka podanega indeksa ce obstaja, 0 sicer
int left(int idx, struct priorityQueue *pq){
	int leftChild = idx * 2;
	if(leftChild <= pq->lastIdx){
		return leftChild;
	}
	return 0;
}

//vrne indeks desnega otroka podanega indeksa ce obstaja, 0 sicer
int right(int idx, struct priorityQueue *pq){
	int rightChild = idx * 2 + 1;
	if(rightChild <= pq->lastIdx){
		return rightChild;
	}
	return 0;
}

//dvigne node na pravo mesto v kopici -> dviguje dokler je otrok manjsi od starsa
void shiftUp(struct priorityQueue *pq){
	int idx = pq->lastIdx;
	int parentIdx = parent(idx);
	while(parentIdx > 0 && pq->heap[idx]->c->weight < pq->heap[parentIdx]->c->weight){
		swap(idx, parentIdx, pq);
		idx = parentIdx;
		parentIdx = parent(parentIdx);
	}
}

//ugrezne element
void shiftDown(struct priorityQueue *pq){
	int idx = 1;
	while(idx < pq->lastIdx){
		double minValue = pq->heap[idx]->c->weight;
		int minIdx = idx;
		
		int leftIdx = left(idx, pq);
		if(leftIdx > 0 && pq->heap[leftIdx]->c->weight < minValue){
			minValue = pq->heap[leftIdx]->c->weight;
			minIdx = leftIdx;
		}
		
		int rightIdx = right(idx, pq);
		if(rightIdx > 0 && pq->heap[rightIdx]->c->weight < minValue){
			minValue = pq->heap[rightIdx]->c->weight;
			minIdx = rightIdx;
		}
		
		if(minIdx == idx){
			break;
		}
		
		swap(idx,minIdx, pq);
		idx = minIdx;
	}
}


//operacije nad priority queue

//vstavi nov node na zadnje mesto v kopici in ga dvigne
void enqueue(struct hNode *node, struct priorityQueue *pq){
	
	if(pq->lastIdx + 1 > pq->capacity - 1){
		//resize queue
		struct hNode **tmpHeap = pq->heap;
		int tmpCapac = pq->capacity;
		pq->heap = malloc(tmpCapac * 2 * sizeof(struct hNode *));
		pq->capacity = tmpCapac * 2;
		for(int i = 0; i < tmpCapac; i++){
			pq->heap[i] = tmpHeap[i];
			free(tmpHeap[i]);
		}	
		free(tmpHeap);
	}
	
	pq->lastIdx++;
	pq->heap[pq->lastIdx] = node;
	pq->size++;
	shiftUp(pq);
}

//zamenja prvi in zadnji element, zadnjega odstrani in prvega ugrezne
struct hNode *dequeue(struct priorityQueue *pq){
	if(pq->lastIdx <= 0){
		return NULL;
	}
	
	struct hNode *root = pq->heap[1];
	swap(1, pq->lastIdx, pq);
	//free(pq->heap[pq->lastIdx]); ?
	
	pq->lastIdx--;
	shiftDown(pq);
	pq->size--;
	
	return root;
}


//operacije nad huffmanovim drevesom

//iz priority queue vzame 2 najmanjsa elementa. Njun stars postane sestevek njunih tez.
//Starsa vstavi nazaj v priority queue.
struct hNode *buildHuffmanTree(struct priorityQueue *pq){
	struct hNode *left, *right, *parent;
	
	while(!(pq->size == 1)){
		struct character *sum = malloc(sizeof(struct character));
		sum->c = '$';
		left = dequeue(pq);
		right = dequeue(pq);
		
		sum->weight = left->c->weight + right->c->weight;
		
		parent = createhNode(sum);
		parent->left = left;
		parent->right = right;
		
		enqueue(parent, pq);
	}
	//vrne kazalec na root kopice
	return dequeue(pq);
}


int chrArrLen = 0;

//listom drevesa dodeli kodne zamenjave
void encode(struct hNode *root, char *hcode, int parent){
	if(root->left){
		hcode[parent] = '0';
		encode(root->left, hcode, parent+1);
	}
	if(root->right){
		hcode[parent] = '1';
		encode(root->right, hcode, parent+1);
	}
	if(!(root->left) && !(root->right)){
		//listu dodeli dvojisko kodo
		hcode[parent] = '\0';
		root->c->hcode = malloc(strlen(hcode) * sizeof(char));
		strcpy(root->c->hcode, hcode);
	}
}



unsigned int bitBuffer = 0;
int bitCount = 0;
char binCode[17];
char binCode2[9];

//https://stackoverflow.com/questions/7863499/conversion-of-char-to-binary-in-c
void charToBin(int16_t c)
{
    for (int i = 15; i >= 0; --i)
    {
    	binCode[15 - i] = (c & (1 << i)) ? '1' : '0';
        //putchar( (c & (1 << i)) ? '1' : '0' );
    }
    //putchar('\n');
    binCode[16] = '\0';
}

void charToBin2(char c)
{
    for (int i = 7; i >= 0; --i)
    {
    	binCode2[7 - i] = (c & (1 << i)) ? '1' : '0';
        //putchar( (c & (1 << i)) ? '1' : '0' );
    }
    //putchar('\n');
    binCode2[8] = '\0';
}

void blockToBin(int c);
char binBlock[32];
unsigned char blockCount = 0;
//zapis zakodirane vsebine

//!! pri zadnjem znaku/ostanku lahko biti ostanejo na desni strani --> treba jih zashiftat cist do konca v levo
void writeBits(int fd, char *code, int bit){

	int codeLen;
	//printf("strlen: %ld, %s\n", strlen(code),);
	//ko pride posamezen char(encodeTree) strlen vraca napacne vrednosti(>1)
	if(bit){
		codeLen = 1;
	}
	else{
		codeLen = strlen(code);
	}
	
	//int b;
	int i;
	for(i = 0; i < codeLen; i++){
		if(code[i] == '1'){
			//b = 1;
			bitBuffer = bitBuffer << 1 | 1;
		}
		else{
			//b = 0;
			bitBuffer = bitBuffer << 1;
		}
		//bitBuffer = bitBuffer << 1 | b;
		
		bitCount++;

		if (bitCount == 32) { 
			//printf("write!\n");
			int n = write(fd, &bitBuffer, sizeof(unsigned int));
			/*blockToBin(bitBuffer);
			printf("%s\n", binBlock);*/
			blockCount++;
			if(n < 0){
				perror("error writing compressed");
			}
			bitBuffer = 0;
			bitCount = 0;
			break;	
		}
		
	}
	
	//preostanek bitov trenutnega znaka
	if(i < codeLen - 1){
		writeBits(fd, code + i + 1, 0);			
	}
}


//zapis kodnega drevesa
void encodeTree(int fd, char bit, int16_t *c){
	//char *byteCode;
	
	if(bit == '1'){
		charToBin(*c);
		writeBits(fd, &bit, 1);
		writeBits(fd, binCode, 0);
		
		return;
	}
	else{
		writeBits(fd, &bit, 1);
	}
}


void encodeNode(struct hNode *node, int fd){
	
	if(!(node->left)){
		//leaf
		//printf("%d..%c\n", node->c->c, node->c->c);
		encodeTree(fd, '1', &node->c->c);	
	}
	else{
		encodeTree(fd, '0', NULL);
	}
}


void traverseHTree(struct hNode *root, int fd){
	//zapise huffmanovo drevo v stisnjeno datoteko s preorder obhodom
	if(root == NULL){
		return;
	}
	
	encodeNode(root, fd);
	
	traverseHTree(root->left, fd);
	traverseHTree(root->right, fd);
}



void blockToBin(int c)
{
    for (int i = 31; i >= 0; --i){
    	binBlock[31 - i] = (c & (1 << i)) ? '1' : '0';
    }
}



void deallocateHTree(struct hNode *root){
	if(root == NULL){
		return;
	}
        deallocateHTree(root->left);
        deallocateHTree(root->right);
        
        //printf("Deleting Node : %c\n", root->c.c);
        free(root);
        
        return;
}



int main(int argc, char *argv[]){
	//double s = omp_get_wtime();
	//printf("%s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3]);
	int fd, n; //input file descriptor, st. prebranih bytov
	char *dstFileName;
	
	struct character **arrayOfCharacters = malloc(257 * sizeof(struct character *));
	
	if (argc < 2){ 
		fprintf(stderr,"Compress usage: src_FileName [dst_FileName]\n");
		exit(1);
	}
	
	if(argv[2] != NULL){
		//podano je ime zeljeno stisnjene datoteke
		dstFileName = argv[2];
	}
	else{
		dstFileName = malloc(16 * sizeof(char));
		strcpy(dstFileName, "compressed.huff");
		dstFileName[15] = '\0';
	}
	
	fd = open(argv[1], O_RDONLY);
	//TODO: ugotovit da podan file se ni stisnjen 
	if(fd < 0){
		perror("Error opening file...\n");
		exit(1);
	}
	printf("Compressing...\n");
	
	int fileLen = 0;
	int arrayLen = 0;
	unsigned char buff[BUFF_SIZE];
	
	//pseudo znak za konec stisnjene vsebine
	struct character *pseudoEOF = malloc(sizeof(struct character));
	pseudoEOF->c = 1000;
	pseudoEOF->weight = 1.0;
	arrayOfCharacters[256] = pseudoEOF;
	
	arrayLen++;
	fileLen++;
	
	
	//v sliki so znaki 0 --> prišteva pojavitve na indeksu 0, kjer je special character
	
	/*clock_t t;
   	t = clock();*/
	while((n = read(fd, buff, BUFF_SIZE)) > 0){
		fileLen+=n;
		buff[n] = '\0';
		
		
		for(int c = 0; c < n; c++){
			if(arrayOfCharacters[(int)buff[c]] != NULL){
				arrayOfCharacters[(int)buff[c]]->weight+=1.0;
			}
			else{
				//znaka se ni v tabeli abecede
				struct character *chr = malloc(sizeof(struct character));
				chr->c = buff[c];
				chr->weight = 1.0;
				//printf("%d\n", (int)buff[c]);
				arrayOfCharacters[(int)buff[c]] = chr; //nov znak na index, ki je njegova ascii koda
				arrayLen++;	
			}
		}	
	}
	
	//int e = omp_get_wtime();
	/*t = clock() - t;
	double time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("Prvo branje vhodne datoteke: %fs.\n", time_taken);*/
	
	chrArrLen = arrayLen;
	struct priorityQueue *pq = malloc(sizeof(struct priorityQueue));
	
	pq->size = 0;
	pq->capacity = arrayLen + 1;
	pq->lastIdx = 0;
	pq->heap = malloc(pq->capacity * sizeof(struct hNode *));
	
	
	//zgradi kopico / vstavlja v priority queue
	//s = omp_get_wtime();
	//t = clock();
	for(int i = 0; i < 257; i++){
		
		if(arrayOfCharacters[i]){
			struct hNode *nd = createhNode(arrayOfCharacters[i]);
			enqueue(nd, pq);
		}
		//printf("heap size: %d\n", pq->size);	
	}
	//e = omp_get_wtime();
	/*t = clock() - t;
	time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("Gradnja kopice: %fs.\n", time_taken);*/
	
	//s = omp_get_wtime();
	//t = clock();
	struct hNode *root = buildHuffmanTree(pq);
	//e = omp_get_wtime();
	/*t = clock() - t;
	time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("Gradnja kodnega drevesa: %fs.\n", time_taken);*/
	
	char *hcode = (char *)malloc(50 * sizeof(char)); //prostor za kodno zamenjavo vsakega znaka
	//s = omp_get_wtime();
	//t = clock();
	encode(root, hcode, 0); // !!
	//e = omp_get_wtime();
	//t = clock() - t;
	/*time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("Dolocanje kodnih zamenjav: %fs.\n", time_taken);*/
	free(hcode);
	
	
	//drugo branje datoteke in zapis zakodiranih vrednosti znakov
	//printf("%s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3]);
	int fd1 = open(dstFileName, O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if(fd1 < 0){
		perror("Error opening compressed file...");
		exit(1);
	}
	
	//postavi na zacetek vhodne datoteke
	lseek(fd, 0, SEEK_SET);
	
	//rezervira prostor za unsigned char na zacetku stisnjene datoteke (st. blokov drevesa)
	lseek(fd1, 1, SEEK_SET);
	
	//izracun velikosti stisnjene datoteke --> ali se splaca stiskat, ce je output > input
	// 1 bit za notranje vozl, 1 bit za list, +16 bitov za vsak znak
	// notranjih vozlisc je za 1 manj kot listov
	/*int treeSize = 18 * arrayLen - 1;
	int contentSize = 0;
	
	for(int s = 0; s < 257; s++){
		if(arrayOfCharacters[s]){
			contentSize += arrayOfCharacters[s]->weight * strlen(arrayOfCharacters[s]->hcode);
		}
	}*/
	
	
	//zapis huffmanovega drevesa pred vsebino (file header)
	//s = omp_get_wtime();
	//t = clock();
	traverseHTree(root, fd1);
	//e = omp_get_wtime();
	/*t = clock() - t;
	time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("Zapis kodnega drevesa: %fs.\n", time_taken);*/
	
	//shift most left, kolk ostane nicel na desni
	//32 ... sizeof bitBuffer
	
	bitBuffer = bitBuffer << (32 - bitCount);
	unsigned char leftOver = 32 - bitCount;
	
	if(leftOver < 32){
		//flush
		write(fd1, &bitBuffer, sizeof(unsigned int));
		blockCount++;
		/*blockToBin(bitBuffer);
		printf("%s\n", binBlock);*/
	}
	
	//printf("input size: %d B, output size: %d B\n", fileLen, ((treeSize + contentSize) + 8 + 32) / 8);
	
	//zapis st. blokov drevesa na rezerviran byte v stisnjeni datoteki
	lseek(fd1, 0, SEEK_SET);
	write(fd1, &blockCount, sizeof(unsigned char));
	//printf("drevo: %d blocks\n", blockCount);

	
	//zapis stisnjene vsebine
	bitBuffer = 0;
	bitCount = 0;
	lseek(fd1, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	//branje po blokih?
	//s = omp_get_wtime();
	//t = clock();
	while((n = read(fd, buff, BUFF_SIZE)) > 0){
		buff[n] = '\0';
		
		for(int c = 0; c < n; c++){
			//charToBin2(buff[c]);
			//printf("znak: %c, %s, frek. pojav.: %.2f, koda: %s,\n", buff[c], binCode2, arrayOfCharacters[(int)buff[c]]->weight, arrayOfCharacters[(int)buff[c]]->hcode);
			writeBits(fd1, arrayOfCharacters[(int)buff[c]]->hcode, 0);	
		}	
	}
	//e = omp_get_wtime();
	/*t = clock() - t;
	time_taken = ((double)t)/CLOCKS_PER_SEC;
	printf("Zapis stisnjene vsebine: %fs.\n", time_taken);*/
	
	//znak za konec stisnjene vsebine
	//printf("znak %c, s frek. pojav.: %d s kodo %s.,\n", arrayOfCharacters[256]->c, arrayOfCharacters[256]->weight, arrayOfCharacters[256]->hcode);
	//charToBin(arrayOfCharacters[256]->c);
	//printf("znak %c, %s, s frek. pojav.: %.2f s kodo %s.,\n", arrayOfCharacters[256]->c, binCode, arrayOfCharacters[256]->weight, arrayOfCharacters[256]->hcode);
	writeBits(fd1, arrayOfCharacters[256]->hcode, 0);
	
	leftOver = 32 - bitCount;

	if(leftOver < 32){
		bitBuffer = bitBuffer << (32 - bitCount);
		//flush
		//shift left --> 00000010?
		write(fd1, &bitBuffer, sizeof(unsigned int));
		/*blockToBin(bitBuffer);
		printf("%s\n", binBlock);*/
	}
	
	if(close(fd1) < 0){
		perror("Error closing file...\n");
		exit(1);
	}
	
	
	if(close(fd) < 0){
		perror("Error closing file...\n");
		exit(1);
	}
	
	
	//ciscenje alociranega pomnilnika
	deallocateHTree(root);
	
	for(int c = 0; c < 257; c++){
		if(arrayOfCharacters[c]){
			free(arrayOfCharacters[c]->hcode);
		}
		free(arrayOfCharacters[c]);
	}
	
	free(pq->heap);
	free(pq);
	printf("Done!\n");
	return 0;
}
