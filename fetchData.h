#include "circularQueue.h"

typedef struct {
    uint8_t OP : 2;
    uint8_t A : 1;
    uint8_t B : 1;
    uint8_t C : 1;
    uint8_t D : 1;
    uint8_t E : 1;
    uint8_t F : 1;
} TPacket;

TPacket fetchData(volatile Queue_t *q){
    TPacket data;
    uint8_t raw_data = q->buffer[q->head];

    data.OP = (raw_data >> 6) & 0b11; // Extract 2 bits
    data.A  = (raw_data >> 5) & 0b01;
    data.B  = (raw_data >> 4) & 0b01;
    data.C  = (raw_data >> 3) & 0b01;
    data.D  = (raw_data >> 2) & 0b01;
    data.E  = (raw_data >> 1) & 0b01;
    data.F  = (raw_data) & 0b00;

    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count--;

    return data;
}