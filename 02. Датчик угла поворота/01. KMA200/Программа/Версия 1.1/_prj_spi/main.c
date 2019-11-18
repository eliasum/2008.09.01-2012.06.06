/********************************************************************************/
/*                                                                              */
/*               ����������� ��������� ��� 'ATMEGA168P' + 'KMA200'              */
/*                       ������ 1.0 (01 �������� 2010 �)                        */
/*                                                                              */
/********************************************************************************/
#include "main.h"
#include <math.h>

/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
union u_int angle, tick;
union u_sint angle_180, tick_180;
union u_sint temp, tock;

unsigned char const __flash Title[] = "KMA200 - Programmable angle sensor";

volatile unsigned char rr_cnt;              // �������� �� ����������� 'WDT'
volatile unsigned char rr_flg;              // ���� ����������� 'WDT'
volatile unsigned char count_bod;           // ������� ���

unsigned char fl_set;                       // ������� ��������� ������ ������

/*==============================================================================*/
/*                                                                              */
/*                         ������� ������� ���������.                           */
/*                                                                              */
/*==============================================================================*/
int main(void)
{
/*  
  ������ DDxn �������� DDx ���������� ����������� �������� ������
����� ������� �����/������. ���� ���� ������ ���������� � �1�, �� n-� �����
����� �������� �������, ���� �� ������� � �0� � ������.
  ������ PORTxn �������� PORTx ��������� ������� �������. ����
����� ������������� ��� ����� (DDxn = �1�), ���� ������ ���������� ��������� 
������ �����. ���� ������ ���������� � �1�, �� ������ ��������������� 
���������� �������� ������. ���� ������ ������� � �0�, �� ������ 
��������������� ���������� ������� ������. ���� �� ����� ������������� ��� 
���� (DDxn = �0�), ������ PORTxn ���������� ��������� ����������� 
�������������� ��������� ��� ������� ������. ��� ��������� ������� PORTxn � �1�
������������� �������� ������������ ����� ������� ���������������� 
� �������� �������.
  ��������� ������ ���������������� (���������� �� ��������� ������� DDxn) 
 ����� ���� �������� ����� ������ ������� PINxn �������� PINx.
*/
  PORTD = (1<<RXD)|(1<<TXD)|(1<<STR)|(1<<SET_)|(1<<KLINE);
  DDRD  = (1<<TXD)|(1<<STR)|(1<<VDD)|(1<<VPRG)|(1<<KLINE)|(1<<RS485);

  PORTB = (1<<LED)|(1<<CS)|(1<<MOSI)|(1<<MISO);
  DDRB  = (1<<LED)|(1<<DIR)|(1<<CS)|(1<<MOSI)|(1<<SCK);
  
  #define VDD_ON       PORTD &= ~(1<<VDD)
  #define VDD_OFF      PORTD |=  (1<<VDD)
  #define VPRG_ON      PORTD |=  (1<<VPRG)
  #define VPRG_OFF     PORTD &= ~(1<<VPRG)
  #define SCK_SET      PORTB |=  (1<<SCK)
  #define SCK_RES      PORTB &= ~(1<<SCK)
  #define CS_SET       PORTB |=  (1<<CS)
  #define CS_RES       PORTB &= ~(1<<CS)
  #define MOSI_TO_IN   DDRB  &= ~(1<<MOSI)
  #define LED_TOGGLE   cpl(PORTB, LED)
  #define RWDT         cpl(PORTD, STR)
  
  // ����� ������ ������ �������:
  if (PIND & (1<<SET_)) fl_set = 1; else fl_set = 0; 

  TCCR0B = 0x00;                            // ������� �/�0
  TCNT0  = 0xB8;                            // ���. ����-�
  TCCR0A = 0x00;
  TCCR0B = 0x04;                            // ������ ���������
  TIMSK0 = 0x01;                            // �/�0 <- �������� ����������

  rr_flg  = 0;                              // ������ "�������" ���������� ������
  count_bod = 1;                            // ������� 1 ����������
  
  RWDT;                                     // 1-� "�������" c���� ����������� �������
  
  __enable_interrupt();                     // ���������� ���������

  // ������������� USART (9600):
  UCSR0B = 0x00;                            // UCSRnB � USART Control and Status Register n B
  UCSR0A = 0x00;                            // UCSRnA � USART Control and Status Register n A
  // ^=> ������ �� ����� ��������� �������� �������� �������� USART0

  UCSR0C = 0x06;                            // UCSRnC � USART Control and Status Register n C *1)
  // ^=> ������ ����� ������ 8 ���
  UBRR0L = (unsigned char)UBBR_9K6;         // ������� �������� 9600; ������ � datasheet
  UBRR0H = (unsigned char)(UBBR_9K6>>8);
  UCSR0B = 0x08;                            // ���������� �������� *2)
       
  SPI_MasterInit();                         // ������������� SPI � ������ Master
     
  // ����� ������ �� ���, uart_putc(0x0a); - ������� �� ����. ������  
  uart_puts_p(Title);                                 uart_putc(0x0d); uart_putc(0x0a);                        
  uart_puts("------------------------------------");  uart_putc(0x0d); uart_putc(0x0a);
  
  for (;;)
  {      
    while(count_bod);                       // ����� ���
    count_bod = 1;                          // ������� 1 ���������� 
      
    unsigned char byte;
      
    if (!fl_set)                           // ����� ������ ������ Command mode
    {       
      LED_TOGGLE;                          // ��������� ������ ��������� �� ���������������
      VDD_OFF;                             // ���������� ������� "��������"
         
      CS_SET;                              // ��������� ������ ������ Command mode        
      SCK_RES;
      VDD_ON;
      delay_ms(6);                         // t_init_cmd1 
      SCK_SET;
      delay_ms(5);                         // t_init_cmd2
         
      CS_RES;                              // ��������� ��������
      byte = CTRL1;                        // ����� ������������ ����� 1 � EEPROM "��������"
      SPI_WriteRead(byte);                 // ��������
         
      tick.du_i = 0x4006;                  // ����� ������ ������ ������ - SPI
      byte = tick.du_c[1];
      SPI_WriteRead(byte);                 // ������ ������� � ���
      byte = tick.du_c[0];
      SPI_WriteRead(byte);               
         
      byte = CTRL2;                        // ����� ������������ ����� 2 � EEPROM "��������"
      SPI_WriteRead(byte);                 // ��������
   
      tick.du_i = 0x504;                   // ���������� ��������������� ����������� � Normal operation mode
      byte = tick.du_c[1];
      SPI_WriteRead(byte);                 // ������ ������� � ���
      byte = tick.du_c[0];
      SPI_WriteRead(byte); 
                          
      byte = 0x30;                         // ������� ���������������� EEPROM
      SPI_WriteRead(byte);                 // ��������
      CS_SET;                              // ����������� ��������
      delay_ms(1);                         // t_cmd1
      VPRG_ON;                             // ��������� ���������� ������� ���������������� EEPROM        
      delay_ms(500);                       // t_prog
      VPRG_OFF;                            // ���������� ���������� ������� ���������������� EEPROM
      CS_RES;
      delay_ms(1);                         // t_ics
       
      LED_TOGGLE;                          // ��������� ������ ��������� �� ���������������
      CS_SET;                              // ����������� ��������
    } 
    else                                   // ����� ������ ������ Normal operation mode
    {
      MOSI_TO_IN;                          // ���������������� ������ MOSI ��� ����
      
      CS_RES;                              // ��������� ��������     
      byte = 0x01;                         // ���� ����������� ������    
      SPI_WriteRead(byte);                 // �������� ����� ����������� ������
     
      uart_puts("Tick/Angle = 0x");        // ����� ������� Tick/Angle = 0x
      
      CS_SET;                              // ����������� ��������
      LED_TOGGLE;                          // ��������� ������ ��������� �� ���������������
      delay_ms(300);
    }
    RWDT;                                  // "�������" c���� ����������� �������
  }
}

/*==============================================================================*/
/*                                                                              */
/*                                ������������.                                 */
/*                                                                              */
/*==============================================================================*/
#pragma vector=TIMER0_OVF_vect
__interrupt void timer0_ovf_isr(void)
{
  //TIMER0 has overflowed
  TCNT0 = RELOAD;                          // reload counter value

  if (count_bod)  count_bod--;

  if (rr_flg & 0x0f)
  {
    if (rr_cnt) rr_cnt--;
    else
    {
      rr_flg--;
      rr_cnt = RR_CONST;
      RWDT;                                // "�������" c���� ����������� �������
    }
  }
}

// ������������ ������������� SPI � ������ Master
void SPI_MasterInit(void)
{
/* - ���������� SPI � ������ �������;
   - CPOL = 1 - ������������ �������� �������� ������������� ����������;
   - CPHA = 1 - ��������� ������ ������������ �� ������� ������ ��������� ������� SCK;
   - ��������� �������� ������ fck/128. */
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPOL)|(1<<CPHA)|(1<<SPR1)|(1<<SPR0);
}

// ������������ ������/������ ����� SPI
unsigned char SPI_WriteRead(unsigned char dataout)
{
  // ������ ����� � ������ ������ �������� (������������� ��������)
  SPDR = dataout;
  // �������� ���������� �������� (���� ��� SPIF �� ����������)
  while(!(SPSR & (1<<SPIF)));
  // ������ �������� ������ � ����� �� ���������
  return SPDR;
}
/* *1)
Bit    7         6        5       4       3       2        1        0
    UMSELn1 | UMSELn0 | UPMn1 | UPMn0 | USBSn | UCSZn1 | UCSZn0 | UCPOLn | UCSRnC
       0    |    0    |       |       |       |        |        |        | Asynchronous USART
            |         |   0   |   0   |       |        |        |        | Parity Mode disabled
            |         |       |       |   0   |        |        |        | Stop Bit(s) - 1-bit
            |         |       |       |       |   1    |   1    |        | Character Size - 8-bit
            |         |       |       |       |        |        |   0    | zero when asynchronous mode is used
������ ����� ������ 8 ��� => 0b00000110 = 0x06
   *2)
Bit    7         6        5       4       3       2        1       0
     RXCIEn | TXCIEn | UDRIEn | RXENn | TXENn | UCSZn2 | RXB8n | TXB8n | UCSRnB
       0    |        |        |       |       |        |       |       | RX Complete Interrupt Enable n
            |    0   |        |       |       |        |       |       | TX Complete Interrupt Enable n
            |        |    0   |       |       |        |       |       | USART Data Register Empty Interrupt Enable n
            |        |        |   0   |       |        |       |       | Receiver Enable n
            |        |        |       |   1   |        |       |       | Transmitter Enable n
            |        |        |       |       |   0    |       |       | Character Size - 8-bit
            |        |        |       |       |        |   0   |       | Receive Data Bit 8 n
            |        |        |       |       |        |       |   0   | Transmit Data Bit 8 n
���������� �������� => 0b00001000 = 0x08
*/