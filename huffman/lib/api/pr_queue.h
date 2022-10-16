

typedef struct _priority_queue{
	int size; //the number of elements in the queue
	int capacity; //maximum number of elements the queue can hold
	huff_node **heap; //heap, implemented as array
	int last_idx; //last element in the queue
} pr_queue;

int parent(int idx);

void swap(int idx1, int idx2, pr_queue *pq);

int left(int idx, pr_queue *pq);

int right(int idx, pr_queue *pq);

void shiftUp(pr_queue *pq);

void shiftDown(pr_queue *pq);

void enqueue(huff_node *node, pr_queue *pq);

huff_node *dequeue(pr_queue *pq);
