#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "Ring_Buffer.h" // Assuming this is your header file
#include <avr/interrupt.h>

// --- ATOMICITY MACROS FIX ---
// The custom ATOMIC_START/END split macros cause 'sreg_tmp' scoping issues.
// We revert these to simple cli()/sei(), and handle the saving/restoring
// of SREG manually within each function block for safety.
#define ENTER_CRITICAL() cli()
#define EXIT_CRITICAL() sei()


void RB_init(ring_buffer_t *buffer)
{
    buffer->head = 0;
    buffer->tail = 0;
    buffer->max_size = RING_BUFFER_SIZE;
    buffer->data_len = 0;
    memset(buffer->data, 0, RING_BUFFER_SIZE);
}

// Status checks are now protected against concurrent access
bool RB_is_full(const ring_buffer_t *buffer)
{
    bool full;
    uint8_t sreg_tmp = SREG;
    ENTER_CRITICAL();
    full = (buffer->data_len == buffer->max_size);
    SREG = sreg_tmp; // Restore SREG
    return full;
}

bool RB_is_empty(const ring_buffer_t *buffer)
{
    bool empty;
    uint8_t sreg_tmp = SREG;
    ENTER_CRITICAL();
    empty = (buffer->data_len == 0);
    SREG = sreg_tmp; // Restore SREG
    return empty;
}

void RB_Write(ring_buffer_t *buffer, char data)
{
    // Write data before atomic block (minor optimization)
    buffer->data[buffer->head] = data;
    
    // Manual SREG Save/Restore for true atomicity
    uint8_t sreg_tmp = SREG;
    ENTER_CRITICAL(); // Disable interrupts (cli())
    
    size_t next_head = (buffer->head + 1) % buffer->max_size;
    
    // Check if the buffer is already full BEFORE updating the head.
    if (buffer->data_len == buffer->max_size)
    {
        // Overwrite: Move tail forward to discard oldest data
        buffer->tail = (buffer->tail + 1) % buffer->max_size;
        // data_len remains at max_size
    }
    else
    {
        // Not full: Simply increment length
        buffer->data_len++;
    }

    buffer->head = next_head;

    SREG = sreg_tmp; // Restore interrupts based on saved state
}

bool RB_Read(ring_buffer_t *buffer, char *data_out)
{
    // Quick check outside of critical section (minor speed up)
    if(RB_is_empty(buffer))
    {
        return false;
    }
    
    // Manual SREG Save/Restore for true atomicity
    uint8_t sreg_tmp = SREG;
    ENTER_CRITICAL(); // Disable interrupts (cli())

    // Re-check emptiness inside critical section for full safety
    if (buffer->data_len == 0)
    {
        SREG = sreg_tmp; // Restore SREG before early exit
        return false;
    }

    *data_out = buffer->data[buffer->tail]; 
    
    buffer->data_len--;
    buffer->tail = (buffer->tail + 1) % buffer->max_size;
    
    SREG = sreg_tmp; // Restore interrupts based on saved state
    return true;
}

// FIX: Changed return type to uint8_t to match the declaration in Ring_Buffer.h
uint8_t RB_Data_len(ring_buffer_t *buffer)
{
    uint8_t len;
    
    // Manual SREG Save/Restore for true atomicity
    uint8_t sreg_tmp = SREG;
    ENTER_CRITICAL(); // Disable interrupts (cli())
    
    // Read the volatile data_len and cast to match return type
    len = (uint8_t)buffer->data_len;
    
    SREG = sreg_tmp; // Restore interrupts based on saved state
    
    return len;
}