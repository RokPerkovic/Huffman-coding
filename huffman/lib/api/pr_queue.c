#include "pr_queue.h"
#include "tree.h"
#include <stdlib.h>

int parent(int idx){
	if(idx <= 1){
		return 0;
	}
	return idx / 2;
}

//zamenja elementa na podanih indeksih v kopici
void swap(int idx1, int idx2, pr_queue *pq){
	huff_node *tmp = pq->heap[idx1];
	pq->heap[idx1] = pq->heap[idx2];
	pq->heap[idx2] = tmp;
}

//vrne indeks levega otroka podanega indeksa ce obstaja, 0 sicer
int left(int idx, pr_queue *pq){
	int leftChild = idx * 2;
	if(leftChild <= pq->lastIdx){
		return leftChild;
	}
	return 0;
}

//vrne indeks desnega otroka podanega indeksa ce obstaja, 0 sicer
int right(int idx, pr_queue *pq){
	int rightChild = idx * 2 + 1;
	if(rightChild <= pq->lastIdx){
		return rightChild;
	}
	return 0;
}

//dvigne node na pravo mesto v kopici -> dviguje dokler je otrok manjsi od starsa
void shiftUp(pr_queue *pq){
	int idx = pq->lastIdx;
	int parentIdx = parent(idx);
	while(parentIdx > 0 && pq->heap[idx]->c->weight < pq->heap[parentIdx]->c->weight){
		swap(idx, parentIdx, pq);
		idx = parentIdx;
		parentIdx = parent(parentIdx);
	}
}

//ugrezne element
void shiftDown(pr_queue *pq){
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
void enqueue(huff_node *node, pr_queue *pq){
	
	if(pq->lastIdx + 1 > pq->capacity - 1){
		//resize queue
		struct hNode **tmpHeap = pq->heap;
		int tmpCapac = pq->capacity;
		pq->heap = malloc(tmpCapac * 2 * sizeof(huff_node *));
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
