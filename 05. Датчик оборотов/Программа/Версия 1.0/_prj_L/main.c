/********************************************************************************/
/*                                                                              */
/*              ����������� ��������� ��� 'ATMEGA168P' + 'AS5130'               */
/*                      ������ 1.0 (23 ������ 2011 �)                          */
/*                                                                              */
/********************************************************************************/
#include "main.h"
#include <math.h>
/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
union u_int tick;
signed char multiturn, current, previous, memory;
unsigned char angle;

volatile unsigned char count_bod;           // ������� ���
volatile unsigned char t_led_on;            // 0.2 ���

unsigned char fl_set;                       // ������� ��������� ������ ������
unsigned char const __flash Title[] = "AS5130 - convertion of Ticks to Angle and Multiturn";
/*==============================================================================*/
/*                                                                              */
/*                         ������� ������� ���������.                           */
/*                                                                              */
/*==============================================================================*/

int main(void)
{
  // ������� ��������� ������ WDT: 
  __watchdog_reset();                       // WDTCSR = WDIF WDIE WDP3 WDCE WDE WDP2 WDP1 WDP0 (XXXX XXXX)
  WDTCSR |= (1<<WDCE) | (1<<WDE);           // ���������� ���������/���������� ������ WDT      (XXX1 1XXX)
  WDTCSR  = (1<<WDE)  | (1<<WDP3);          // ��������� ����-���� = 512K ������ (~ 4.0 �)     (0010 1000)

  PORTB = (1<<LED)|(1<<MOSI)|(1<<MISO)|(1<<SCK);
  DDRB  = (1<<LED)|(1<<MOSI)|(1<<SCK);
  
  PORTD = (1<<RXD)|(1<<TXD)|(1<<RE)|(1<<SET_)|(1<<WP);
  DDRD  = (1<<TXD)|(1<<RE)|(1<<DIO)|(1<<CS)|(1<<DCLK)|(1<<WP);
     
  // ����� ������ ������ �������:
  if (PIND & (1<<SET_)) fl_set = 1; else fl_set = 0;
  
  count_bod = 1;                            // ������� 1 ����������
  t_led_on  = 0;                            // ����� ���������
  
  __watchdog_reset();                       // 1-� "�������" c���� ����������� �������

  __enable_interrupt();                     // ���������� ���������

//////////////////////// ������ � ������ -= ��������� =- ///////////////////////
  //if (fl_set)
  {
    // ������������� USART (9600):
    UCSR0B = 0x00;                          // ������ �������� USART0 �� ����� ��������� �������� ��������
    UCSR0A = 0x00;                          
    UCSR0C = 0x06;                          // ������ ����� ������ 8 ���
    UBRR0L = (unsigned char)UBBR_9K6;       // ������� �������� 9600; ������ � datasheet
    UBRR0H = (unsigned char)(UBBR_9K6>>8);
    UCSR0B = 0x08;                          // ���������� �������� (TXENn = 1)
    
    // ������������� �/�0:
    TCCR0B = 0x00;                          // ������� �/�0
    TCNT0  = RELOAD_TIM0;                   // ���. ����-�
    TCCR0A = 0x00;
    TCCR0B = START_TIM0;                    // ������ ���������
    TIMSK0 = 0x01;                          // �/�0 <- �������� ����������
    
    // ����� ������ �� ���, uart_putc(0x0a); - ������� �� ����. ������  
    uart_puts_p(Title);                                 uart_putc(0x0d); uart_putc(0x0a);                        
    uart_puts("------------------------------------");  uart_putc(0x0d); uart_putc(0x0a);  
     
    if (fl_set) memory = 0;                 // ����� �������� �� ��� ������ ������
    
    for (;;)
    {            
      while(count_bod);                     // ����� ���
      count_bod = 1;                        // ������� 1 ���������� 

      SET(CS);                              // ��������� ��������, ������������� ������ ������ 

      tick.du_i = SSI_WriteRead(RD_BOTH);   // 2 ����� ������ �� AS5130
      
      multiturn = tick.du_c[1];             // ���� ������ �� AS5130 --> multiturn <7:0>
      angle = tick.du_c[0];                 // ���� ������ �� AS5130 --> angle <7:0>
            
      previous = current;                   // ���������� �������� ���������� �������� (��) � ���
      current = multiturn;                  // ������� �������� �� � ���
      
      memory = FM24Cxx_get(FM24C512, 1);    // ���������� �������� �� �� ������
      
      if(current != previous)               // ��� ����������?
      {
        multiturn = memory + current - previous;
      } multiturn = memory;
      
      if(current != memory)                 // ������� �������� �� = ������?
      {
        memory = multiturn;
        FM24Cxx_put(FM24C512, 1, memory);   // ��������� �������� �������� �� � ������
      }
      
      uart_puts("tick/multiturn = 0x");     // ����� ������� tick/multiturn = 0x
      uart_puthex_byte(tick.du_c[1]);       // ����� ������� 8 ��� ������ �� AS5130
      uart_puts("/");
      uart_puti(multiturn);                 // ����� �������� ���������� ��������
      uart_puts("`; ");                     // ����� ������ `; 
      
      uart_putc(0x0d);                      // ������� �������
      uart_putc(0x0a);                      // ������� �� ��������� ������

      uart_puts("tick/angle = 0x");         // ����� ������� tick/angle = 0x
      uart_puthex_byte(tick.du_c[0]);       // ����� ������� 8 ��� ������ �� AS5130
      uart_puts("/");
      
      angle=(unsigned char)(1.40625*angle); // �������� ���� ����� (360/256=1.40625)
      
      uart_puti(angle);                     // ����� �������� ���� ��������
      uart_puts("`; ");                     // ����� ������ `; 
      
      uart_putc(0x0d);                      // ������� �������
      uart_putc(0x0a);                      // ������� �� ��������� ������
      
      LED_TOGGLE;                           // ��������� ������ ��������� �� ���������������
      SET(CS);                              // ����������� ��������, ���������� ������ ������

      __watchdog_reset();                   // "�������" c���� ����������� �������
      
    } // end of 'for (;;)' 
  } // end of 'if (fl_set)' 
/*  
///////////////////////// ������ � ������ -= ������ =- /////////////////////////

  else
  {
    // ������������� USART (2400):
    UCSR0B = 0x00;                          // ������ �������� USART �� ����� ��������� �������� ��������
    UCSR0A = 0x00;                          
    UCSR0C = 0x06;                          // ������ ����� ������ 8 ���
    UBRR0L = UBBR_2K4L;                     // ������� �������� 2400
    UBRR0H = UBBR_2K4H;   
    UCSR0B = 0x0C;                          // ���������� �������� (TXENn = 1); 9-Bit

    // ������������� �/�0:
    TCCR0B = 0x00;                          // ������� �/�0
    TCNT0  = RELOAD_TIM0;                   // ���. ����-�
    TCCR0A = 0x00;
    TCCR0B = START_TIM0;                    // ������ ���������
    TIMSK0 = 0x01;                          // �/�0 <- �������� ����������
       
    __enable_interrupt();                   // ���������� ���������   
////////////////////////////////////////////////////////////////////////////////  
          
                    //����������� ������ ����� ������ K-LINE                    
          
////////////////////////////////////////////////////////////////////////////////      
    for (;;)
    {
      UCSR0B &= ~(1<<RXEN0);                // ���������� ��������� USART
      count_bod = CBOD;                     // ������� ���

      do
      {
        if (!(PIND & (1<<RXD))) count_bod = CBOD;
      } while (count_bod);                  // �������� ������-����� (������ ������ ����� ��������)

      UCSR0B |= (1<<RXEN0);                 // ��������� ��������� USART

      while (!(UCSR0A & (1<<RXC0)));        // ���� ���������� ������ ����� 

      if (UCSR0B & (1<<RXB80))              // ���� 9-� ��� =1, �� ������� �������
      {
        unsigned char var;
        var  = UDR0;                        // UDRn - USART I/O Data Register n
        var &= 0x0f;                        // ����� ������� �/� �� ����� 15
        if (var == NDEV)                    // ���������� � ����� �������?
        {    
                 
          __watchdog_reset();               // c���� ����������� �������
          
        } // end of 'if (var == NDEV)' 
      } // end of 'if (UCSR0B & (1<<RXB80))' 
    } // end of 'for (;;)' 
  } // end of 'else' 
*/
} // end of 'main' 
/*==============================================================================*/
/*                                                                              */
/*                                ������������.                                 */
/*                                                                              */
/*==============================================================================*/

#pragma vector=TIMER0_OVF_vect
__interrupt void timer0_ovf_isr(void)
{
  //TIMER0 has overflowed
  TCNT0  = RELOAD_TIM0;                     // reload counter value

  if (count_bod)  count_bod--;

  if(t_led_on)  t_led_on--;                 // ����� ������������� --
  else          LED_OFF;                    // LED-OFF
}

unsigned int SSI_WriteRead(unsigned int dataout) 
{
  unsigned char bit;
  unsigned int datain = 0;                  // ����������� ������
    
  for(bit=0; bit<8; bit++)                  // ������ �������
  {
    SET(DCLK);                              // �������� ����� �������������� �������� �������������
    
    if(dataout&0x80) SET(DIO);              // ���� ������� ��� ������� = 1 --> �� ������ DIO �������
    else             RES(DIO);              // ����� --> �� ������ DIO ����

    dataout<<=1;                            // ����� ������������ ������� �� 1 ���
    RES(DCLK);                              // ������ ����� �������������� �������� �������������
  }  
  
  for(bit=0; bit<16; bit++)                 // ������ ������ (2-� ������� �����)
  {
    SET(DCLK);                              // �������� ����� �������������� �������� �������������
    datain<<=1;                             // ����� ������������ ����� �� 1 ���
    
    if(PIND&(1<<DIO))                       // ���� �� ������ DIO �������  --> ������� ��� ���������� ����� = 1 
      {
        datain+=0x01;
      }                                     // ����� --> ������� ��� ���������� ����� = 0                               
    RES(DCLK);                              // ������ ����� �������������� �������� �������������
  } 
  return datain;
}