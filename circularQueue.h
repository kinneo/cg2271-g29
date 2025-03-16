#define QUEUE_SIZE 16  // Maximum packets stored in queue

typedef struct {
    uint8_t buffer[QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} Queue_t;

// Queue Initialization
void Q_Init(volatile Queue_t *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

// Check if queue is full
bool Q_Full(volatile Queue_t *q) {
    return (q->count == QUEUE_SIZE);
}

// Check if queue is empty
bool Q_Empty(volatile Queue_t *q) {
    return (q->count == 0);
}

// Enqueue (add element to queue)
bool Q_Enqueue(volatile Queue_t *q, uint8_t data) {
    if (Q_Full(q)) {
        return false;  // Queue is full, cannot enqueue
    }
    q->buffer[q->tail] = data;
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    q->count++;
    return true;
}

// Dequeue (remove element from queue)
bool Q_Dequeue(volatile Queue_t *q, uint8_t *data) {
    if (Q_Empty(q)) {
        return false;  // Queue is empty, cannot dequeue
    }
    *data = q->buffer[q->head];
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count--;
    return true;
}
