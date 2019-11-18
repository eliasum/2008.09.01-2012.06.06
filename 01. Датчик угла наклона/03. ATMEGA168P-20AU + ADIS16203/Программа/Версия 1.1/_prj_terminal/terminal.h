#ifndef _terminal_h_
#define _terminal_h_

#include "main.h"

void uart_putc(unsigned char data);
void uart_puts(unsigned char *s );
void uart_puthex_byte(unsigned char  b);
void uart_puti( unsigned int val );
void uart_puthex_nibble(unsigned char b);
void itoa(unsigned int n, unsigned char s[]);
void Bin_to_Dec (unsigned char s[], unsigned char r_poz, unsigned char n_poz, unsigned int dnum);   
void uart_puts_p(unsigned char const __flash s[] );

#endif