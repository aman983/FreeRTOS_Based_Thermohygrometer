
#ifndef UART_H
#define UART_H

#define Tx  PD1
#define Rx  PD0
#define BAUD 9600
#define MYUBRR (F_CPU / 16 / BAUD - 1)

void UART_init(void);
char UART_read_char(void);
#endif
