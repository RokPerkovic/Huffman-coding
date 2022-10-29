#include "pr_queue.h"
#include <stdlib.h>
#include <stdio.h>
//comment
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
	int left_child = idx * 2;
	if(left_child <= pq->last_idx){
		return left_child;
	}
	return 0;
}

//vrne indeks desnega otroka podanega indeksa ce obstaja, 0 sicer
int right(int idx, pr_queue *pq){
	int right_child = idx * 2 + 1;
	if(right_child <= pq->last_idx){
		return right_child;
	}
	return 0;
}

//dvigne node na pravo mesto v kopici -> dviguje dokler je otrok manjsi od starsa
void shift_up(pr_queue *pq){
	int idx = pq->last_idx;
	int parent_idx = parent(idx);
	while(parent_idx > 0 && pq->heap[idx]->c->weight < pq->heap[parent_idx]->c->weight){
		swap(idx, parent_idx, pq);
		idx = parent_idx;
		parent_idx = parent(parent_idx);
	}
}

//ugrezne element
void shift_down(pr_queue *pq){
	int idx = 1;
	while(idx < pq->last_idx){
		double min_value = pq->heap[idx]->c->weight;
		int min_idx = idx;
		
		int left_idx = left(idx, pq);
		if(left_idx > 0 && pq->heap[left_idx]->c->weight < min_value){
			min_value = pq->heap[left_idx]->c->weight;
			min_idx = left_idx;
		}
		
		int right_idx = right(idx, pq);
		if(right_idx > 0 && pq->heap[right_idx]->c->weight < min_value){
			min_value = pq->heap[right_idx]->c->weight;
			min_idx = right_idx;
		}
		
		if(min_idx == idx){
			break;
		}
		swap(idx,min_idx, pq);
		
		idx = min_idx;
	}
}


//operacije nad priority queue

void resize(pr_queue *pq){
	huff_node **tmp_heap = pq->heap;
	int tmp_capac = pq->capacity;
	pq->heap = malloc(tmp_capac * 2 * sizeof(huff_node *));
	pq->capacity = tmp_capac * 2;
	for(int i = 0; i < tmp_capac; i++){
		pq->heap[i] = tmp_heap[i];
		free(tmp_heap[i]);
	}	
	free(tmp_heap);
}

//vstavi nov node na zadnje mesto v kopici in ga dvigne
void enqueue(huff_node *node, pr_queue *pq){
	//queue capacity is set to number of unique characters in input so resize never occurs
	/*if(pq->last_idx + 1 > pq->capacity - 1){
		resize(pq);
	}*/
	
	pq->last_idx++;
	//printf("last idx: %d, size: %d\n", pq->last_idx, pq->capacity);
	pq->heap[pq->last_idx] = node;
	pq->size++;
	shift_up(pq);
	//printf("here %d\n", pq->size);
	
}

//zamenja prvi in zadnji element, zadnjega odstrani in prvega ugrezne
huff_node *dequeue(pr_queue *pq){
	
	if(pq->last_idx <= 0){
		return NULL;
	}
	
	huff_node *root = pq->heap[1];
	
	swap(1, pq->last_idx, pq);
	//free(pq->heap[pq->lastIdx]); ?
	
	pq->last_idx--;
	shift_down(pq);
	pq->size--;
	
	return root;
}
