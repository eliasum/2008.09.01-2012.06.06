#include "terminal.h"

/*************************************************************************
Function: uart_putc()
Purpose:  write byte to ringbuffer for transmitting via UART
Input:    byte to be transmitted
Returns:  none          
**************************************************************************/
void uart_putc(unsigned char data)          // вывод байта в 'USART'
 {  
   PORTB  |= (1<<DIR);                      // RS485 -> направление ПЕРЕДАЧА
   UCSR0A |=  (1<<TXC0);
   UCSR0B &= ~(1<<TXB80);                   // 9-й бит -> '0'      (данные)
   UDR0 = data;                             // передать байт данных
   while (!(UCSR0A & (1<<TXC0)));           // ждать конца передачи байта
   PORTB &= ~(1<<DIR);                      // RS485 -> направление ПРИЕМ
 } /* uart_putc */

/*************************************************************************
Function: uart_puts()
Purpose:  transmit string to UART
Input:    string to be transmitted
Returns:  none          
**************************************************************************/
void uart_puts(unsigned char *s )
 { while (*s) 
      uart_putc(*s++);
 } /* uart_puts */

/*************************************************************************
Function: uart_puthex_byte()
Purpose:  transmit upper and lower nibble as ASCII-hex to UART
Input:    byte value
Returns:  none
This functions has been added by Martin Thomas <eversmith@heizung-thomas.de>
Don't blame P. Fleury if it doesn't work ;-)
**************************************************************************/
void uart_puthex_byte(unsigned char  b)
 { uart_puthex_nibble(b>>4);
   uart_puthex_nibble(b);
 } /* uart_puthex_byte */

/*************************************************************************
Function: uart_puti()
Purpose:  transmit integer as ASCII to UART
Input:    integer value
Returns:  none
This functions has been added by Martin Thomas <eversmith@heizung-thomas.de>
Don't blame P. Fleury if it doesn't work ;-)
**************************************************************************/
void uart_puti( unsigned int val )
 { unsigned char buffer[8];
  
   itoa(val, buffer);
   uart_puts( buffer );
 }/* uart_puti */

/*************************************************************************
Function: uart_puthex_nibble()
Purpose:  transmit lower nibble as ASCII-hex to UART
Input:    byte value
Returns:  none
This functions has been added by Martin Thomas <eversmith@heizung-thomas.de>
Don't blame P. Fleury if it doesn't work ;-)
**************************************************************************/
void uart_puthex_nibble(unsigned char b)
 { unsigned char  c = b & 0x0f;
   if (c>9) c += 'A'-10;
   else c += '0';
   uart_putc(c);
 } /* uart_puthex_nibble */

/*************************************************************************
 itoa:  конвертируем n в символы в s
**************************************************************************/
void itoa(unsigned int n, unsigned char s[])
{
  Bin_to_Dec (s, 4, 5, n);
  s[5]=0;
}

/*----------------------------------------------------------------------------*/
/* Функция:    Преобразование BIN->DEC 's[]'                                  */
/* Принимает:  двоичное число; правую позицию вывода; число позиций (см.ниже) */
/* Возвращает: ничего (изменяет глобальные переменные)                        */
/*----------------------------------------------------------------------------*/
// 
void Bin_to_Dec (unsigned char s[],     // буфер
                 unsigned char r_poz,   // правая позиия (0...3)
                 unsigned char n_poz,   // число позиций (1...4)
                 unsigned int dnum)  {  // преобразуемое (0...32762)
  do { s[r_poz--]=dnum%10+'0'; dnum/=10; }
  while (--n_poz!=0);
}

/*************************************************************************
Function: uart_puts_p()
Purpose:  transmit string from program memory to UART
Input:    program memory string to be transmitted
Returns:  none
**************************************************************************/
void uart_puts_p(unsigned char const __flash s[] )
 { unsigned char c, i = 0; 
   while ( (c = s[i++]) ) 
      uart_putc(c);
 } /* uart_puts_p */
