#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <stdbool.h>
// Define the maximum size of the buffer
#define RING_BUFFER_SIZE 50

// Define the structure for the ring buffer
typedef struct {
    char data[RING_BUFFER_SIZE]; // Array to hold the data
    uint8_t head;                 // Index of the next write location (empty slot)
    uint8_t tail;                 // Index of the next read location (first filled slot)
    uint8_t max_size;             // The maximum capacity (RING_BUFFER_SIZE)
    uint8_t data_len;
} ring_buffer_t;

void RB_init(ring_buffer_t *buffer);
bool RB_is_full(const ring_buffer_t *buffer);
bool RB_is_empty(const ring_buffer_t *buffer);
void RB_Write(ring_buffer_t *buffer, char data);
bool RB_Read(ring_buffer_t *buffer, char *data_out);
uint8_t RB_Data_len(ring_buffer_t *buffer);
#endif