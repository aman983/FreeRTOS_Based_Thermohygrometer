#include <avr/io.h>
#include <stdarg.h>
#include <stdlib.h>
#include "UART.h"
#include "avr/interrupt.h"
#include "Ring_Buffer.h"
#include <stdio.h>

ring_buffer_t uart_buffer;

static int UART_putchar(char c, FILE *stream) {
    if (c == '\n') {
        UART_putchar('\r', stream);  // Add carriage return for new line
    }
    while (!(UCSR0A & (1 << UDRE0)));  // Wait until buffer is empty
    UDR0 = c;                          // Send character
    return 0;
}
// Create FILE stream object for stdout
FILE uart_output = FDEV_SETUP_STREAM(UART_putchar, NULL, _FDEV_SETUP_WRITE);

void UART_init(void) {
    DDRD |= (1 << Tx);
    DDRD &= ~(1 << Rx);

    // Set baud rate
    UBRR0H = (uint8_t)(MYUBRR >> 8);
    UBRR0L = (uint8_t)(MYUBRR);

    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // Set frame: 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
  
    RB_init(&uart_buffer);
    stdout = &uart_output;
  
}



char UART_read_char(void) {
    // Wait until data is received
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0; // Read and return received data
}
/*
ISR(USART_RX_vect){
    RB_Write(&uart_buffer, UDR0);
}
*/
