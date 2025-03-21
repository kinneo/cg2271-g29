#include "circularQueue.h"

typedef struct {
  uint8_t A : 3;
  uint8_t B : 1;
  uint8_t C : 1;
  uint8_t D : 1;
  uint8_t OP : 2;
} TPacket;

TPacket fetchData(volatile Queue_t *q){
    TPacket data;
    uint8_t raw_data = q->buffer[q->head];

    data.OP = (raw_data >> 6) & 0x03; // Extract 2 bits
    data.D  = (raw_data >> 5) & 0x01; // Extract 1 bit
    data.C  = (raw_data >> 4) & 0x01; // Extract 1 bit
    data.B  = (raw_data >> 3) & 0x01; // Extract 1 bit
    data.A  = raw_data & 0x07;

    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count--;

    return data;
}