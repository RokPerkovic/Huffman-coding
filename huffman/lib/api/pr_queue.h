struct priorityQueue;

int parent(int idx);

void swap(int idx1, int idx2, struct priorityQueue *pq);

int left(int idx, struct priorityQueue *pq);

int right(int idx, struct priorityQueue *pq);

void shiftUp(struct priorityQueue *pq);

void shiftDown(struct priorityQueue *pq);

void enqueue(struct hNode *node, struct priorityQueue *pq);

struct hNode *dequeue(struct priorityQueue *pq);
