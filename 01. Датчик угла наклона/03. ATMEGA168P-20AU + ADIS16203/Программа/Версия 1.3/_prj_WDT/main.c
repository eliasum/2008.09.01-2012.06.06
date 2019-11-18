/********************************************************************************/
/*                                                                              */
/*             ����������� ��������� ��� 'ATMEGA168P' + 'ADIS16203'             */
/*                       ������ 1.3 (22 �������� 2010 �)                        */
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

unsigned char const __flash Title[] = "ADIS16203 - convertion of Ticks to Angle";
                                            
unsigned char Ttick;                        // ������� �� ������� "�����"
signed int t0;

volatile unsigned char count_bod;           // ������� ���
unsigned char fl_set;                       // ������� ��������� ������ ������
unsigned char var;                          // ����. ����-� ������
volatile unsigned int t_led_on;             // 0.2 ���
/*==============================================================================*/
/*                                                                              */
/*                         ������� ������� ���������.                           */
/*                                                                              */
/*==============================================================================*/

int main(void)
{
  /* WDT Prescaler Change */
  __watchdog_reset();
  /* Start timed equence */
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  /* Set new prescaler(time-out) value = 512K cycles (~ 4.0 s) */
  WDTCSR  = (1<<WDE) | (1<<WDP3);
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
  PORTD = (1<<RXD)|(1<<TXD)|(1<<SET_)|(1<<KLINE)|(1<<RST);
  DDRD  = (1<<TXD)|(1<<NAGR)|(1<<KLINE)|(1<<RST);
   
  PORTB = (1<<LED)|(1<<CS)|(1<<MOSI)|(1<<MISO);
  DDRB  = (1<<LED)|(1<<CS)|(1<<MOSI)|(1<<SCK);

  #define LED_TOGGLE   cpl(PORTB, LED)
  
  if (PIND & (1<<SET_)) fl_set = 1; else fl_set = 0; // ����� ������ ������ �������

  // TIMER0 initialize - prescale:256
  // WGM:           Normal
  // desired value: 600Hz
  // actual value:  600.000Hz (0.0%)

  TCCR0B = 0x00;                            // ������� �/�0
  TCNT0  = 0xB8;                            // ���. ����-�
  TCCR0A = 0x00;
  TCCR0B = 0x04;                            // ������ ���������
  TIMSK0 = 0x01;                            // �/�0 <- �������� ����������

  t_led_on  = 0;                            // ����� ���������
  count_bod = 1;                            // ������� 1 ����������
  
  __watchdog_reset();                       // 1-� "�������" c���� ����������� �������

  __enable_interrupt();                     // ���������� ���������

//////////////////////// ������ � ������ -= ��������� =- ///////////////////////
  if (!fl_set)
  {
    // ������������� USART (9600):
    UCSR0B = 0x00;                          // UCSRnB � USART Control and Status Register n B
    UCSR0A = 0x00;                          // UCSRnA � USART Control and Status Register n A
    // ^=> ������ �� ����� ��������� �������� �������� �������� USART0

    UCSR0C = 0x06;                          // UCSRnC � USART Control and Status Register n C *1)
    // ^=> ������ ����� ������ 8 ���
    UBRR0L = (unsigned char)UBBR_9K6;       // ������� �������� 9600; ������ � datasheet
    UBRR0H = (unsigned char)(UBBR_9K6>>8);
    UCSR0B = 0x08;                          // ���������� �������� *2)
    
    PORTD &= ~(1<<RST);                     // ����� �������� ��������� ������������� 100 ���
    delay_us(100);
    PORTD |= (1<<RST);   
    SPI_MasterInit();                       // ������������� SPI � ������ Master
    
    // ����� ������ �� ���, uart_putc(0x0a); - ������� �� ����. ������  
    uart_puts_p(Title);                                 uart_putc(0x0d); uart_putc(0x0a);                        
    uart_puts("------------------------------------");  uart_putc(0x0d); uart_putc(0x0a);  
     
    for (;;)
    {      
      while(count_bod);                     // ����� ���
      count_bod = 1;                        // ������� 1 ���������� 
      
      unsigned char byte;
  
      PORTB &= ~(1<<CS);                    // ��������� ��������
////////////////////////////////////////////////////////////////////////////////  
          
                          //����� ���� 360� (INCL_OUT)
          
////////////////////////////////////////////////////////////////////////////////        
      byte = INCL_180_OUT;                  // ����� ��������, ����� �� �������� � ����������� �����    
      SPI_WriteRead(byte);                  // ������ �� ������ �������� ����� �������� INCL_180_OUT
      tick.du_c[1] = SPDR;                  // ������� ���� ������ �� INCL_OUT  
      tick.du_c[1] &= 0x3f;                 // ������ 14 ���
      
      byte = 0x0C;                          // ����� ������
      SPI_WriteRead(byte);                  // ������ �� ������ �������� ����� �������� INCL_180_OUT
      tick.du_c[0] = SPDR;                  // ������� ���� ������ �� INCL_OUT 
       
      uart_puts("Tick/Angle = 0x");         // ����� ������� Tick/Angle = 0x
      uart_puthex_byte(tick.du_c[1]);       // ����� �������� ����� ������ �� INCL_OUT
      uart_puthex_byte(tick.du_c[0]);       // ����� �������� ����� ������ �� INCL_OUT
      uart_puts("/");                       // ����� ����� / 
      
      // �������������� ����� � �������� ����� �������:
      angle.du_i = (unsigned int)(0.25*tick.du_i);
      
      uart_puti(angle.du_i);                // ����� �������� ���� ������� � ���������� ��������� ��� �������
      uart_puts("`; ");                     // ����� ������ `;  
      uart_putc(0x0d);                      // ������� �������
      uart_putc(0x0a);                      // ������� �� ��������� ������ 
                     
////////////////////////////////////////////////////////////////////////////////  
          
                        //����� ���� �180� (INCL_180_OUT)
          
////////////////////////////////////////////////////////////////////////////////        
      byte = TEMP_OUT;                      // ����� ��������, ����� �� �������� � ����������� �����    
      SPI_WriteRead(byte);                  // ������ �� ������ �������� ����� �������� TEMP_OUT
      tick_180.du_sc[1] = SPDR;             // ������� ���� ������ �� INCL_180_OUT  
      tick_180.du_sc[1] &= 0x3F;            // ������ 14 ���
      
      byte = 0x0E;                          // ����� ������
      SPI_WriteRead(byte);                  // ������ �� ������ �������� ����� �������� TEMP_OUT
      tick_180.du_sc[0] = SPDR;             // ������� ���� ������ �� INCL_180_OUT 
/*
      ���� ����� �����. ����� ����� �� ��� ���� - "����������" ����� �� ������������ 
      ������� 16 ���, � ������� 16-� ��� �������� ���������� � ����� �����:
*/
      tick_180.du_si = tick_180.du_si << 2; 
/*
      �������� �� 4 ����� "���������� �������" � ��������� 14-� �������� ������� 
      � ����������� ������� �������� ���� ����� ��� ���������:
*/ 
      tick_180.du_si /= 4; 
      
      uart_puts("Tick_180/Angle_180 = 0x"); // ����� ������� Tick_180/Angle_180 = 0x
      uart_puthex_byte(tick_180.du_sc[1]);  // ����� �������� ����� ������ �� INCL_180_OUT
      uart_puthex_byte(tick_180.du_sc[0]);  // ����� �������� ����� ������ �� INCL_180_OUT
      uart_puts("/");                       // ����� ����� / 
      
      // �������������� ����� � �������� ����� �������:
      angle_180.du_si = (signed int)(0.025*tick_180.du_si);     
      
      uart_puti(angle_180.du_si);           // ����� �������� ���� �������  
      uart_puts("`; ");                     // ����� ������ `;   
      uart_putc(0x0d);                      // ������� �������
      uart_putc(0x0a);                      // ������� �� ��������� ������ 
////////////////////////////////////////////////////////////////////////////////  
          
                          //����� ����������� (TEMP_OUT)
          
////////////////////////////////////////////////////////////////////////////////        
      byte = INCL_OUT;                      // ����� ��������, ����� �� �������� � ����������� �����      
      SPI_WriteRead(byte);                  // ������ �� ������ �������� ����� �������� INCL_OUT
      tock.du_sc[1] = SPDR;                 // ������� ���� ������ �� TEMP_OUT
      tock.du_sc[1] &= 0x0F;                // ������ 12 ���
 
      byte = 0x0A;                          // ����� ������
      SPI_WriteRead(byte);                  // ������ �� ������ �������� ����� �������� INCL_OUT
      tock.du_sc[0] = SPDR;                 // ������� ���� ������ �� TEMP_OUT 
/*
      ���� ����� �����. ����� ����� �� 4 ���� - "����������" ����� �� ������������ 
      ������� 16 ���, � ������� 16-� ��� �������� ���������� � ����� �����:
*/
      tock.du_si = tock.du_si << 4; 
/*
      �������� �� 16 ����� "���������� �������" � ��������� 12-� �������� ������� 
      � ����������� ������� �������� ���� ����� ��� ���������:
*/
      tock.du_si /= 16;
      
      uart_puts("Tock/Temperature = 0x");   // ����� ������� Tock/Temperature = 0x
      uart_puthex_byte(tock.du_sc[1]);      // ����� �������� ����� ������ �� TEMP_OUT
      uart_puthex_byte(tock.du_sc[0]);      // ����� �������� ����� ������ �� TEMP_OUT
      uart_puts("/");                       // ����� ����� / 
      
      // �������������� ����� � �������� �����������:
      float delta;
      if (tock.du_si >= 0x04FE)
      {
        delta = tock.du_si - 0x04FE;
        temp.du_si = (signed int)(25 - 0.47*delta);
      }  
      else
      {
        delta = 0x04FE - tock.du_si;
        temp.du_si = (signed int)(25 + 0.47*delta);
      }         
            
      uart_puti(temp.du_si);                // ����� �������� �����������
      uart_puts("`C");                      // ����� ������ `C
      
      uart_putc(0x0d);                      // ������� �������
      uart_putc(0x0a);                      // ������� �� ��������� ������ 
      delay_ms(250);                        // �������� ��� ���������� ���������
           
      LED_TOGGLE;
      PORTB |= (1<<CS);                     // ����������� ��������

      __watchdog_reset();                   // "�������" c���� ����������� �������
    }
  }
///////////////////////// ������ � ������ -= ������ =- /////////////////////////
  else
  {
    // ������������� USART (2400):
    UCSR0B = 0x00;                          // ������ �� ����� ��������� ��������
    UCSR0A = 0x00;                          // �������� �������� USART
    UCSR0C = 0x06;                          // ������ ����� ������ 8 ���
    UBRR0L = (unsigned char)UBBR_2K4;       // ������� �������� 2400; ������ � datasheet
    UBRR0H = (unsigned char)(UBBR_2K4>>8);
    UCSR0B = 0x0C;     

    for (;;)
    {
      UCSR0B &= ~(1<<RXEN0);                // ����� �� 'RxD' 'OFF'
      count_bod = CBOD;                     // ������� ���

      do
      {
        if (!(PIND & (1<<RXD))) count_bod = CBOD;
      } while (count_bod);                  // �������� ������-�����

      UCSR0B |= (1<<RXEN0);                 // ����� �� 'RxD' 'ON'
      while (UCSR0A & (1<<RXC0))
        var = UDR0;                         // "������" �������� �����

      while (!(UCSR0A & (1<<RXC0)));        // "����" ����

      if (UCSR0B & (1<<RXB80))
      {
        var  = UDR0;
        var &= 0x0f;
        if (var == NDEV)                    // ���������� � ����� �������?
        {
          unsigned char byte;

          t_led_on  = T_LED_ON;             // LED-ON
          PORTB &= ~(1<<CS);                // ��������� ��������
          byte = INCL_180_OUT;              // ����� ��������, ����� �� �������� � ����������� �����    
          SPI_WriteRead(byte);              // ������ �� ������ �������� ����� �������� INCL_180_OUT
          tick.du_c[1] = SPDR;              // ������� ���� ������ �� INCL_OUT  
          tick.du_c[1] &= 0x3f;             // ������ 14 ���
      
          byte = 0x0C;                      // ����� ������
          SPI_WriteRead(byte);              // ������ �� ������ �������� ����� �������� INCL_180_OUT
          tick.du_c[0] = SPDR;              // ������� ���� ������ �� INCL_OUT 
       
          // �������������� ����� � ���������� �������:
          angle.du_i = (unsigned int)(0.25*tick.du_i);
          // ����� ��������
          uart_putc(angle.du_c[0]);
          uart_putc(angle.du_c[1]);
          __watchdog_reset();               // c���� ����������� �������
        }
      }
    }
  }  
////////////////////////////////////////////////////////////////////////////////  
} /* end of 'main' */
/*==============================================================================*/
/*                                                                              */
/*                                ������������.                                 */
/*                                                                              */
/*==============================================================================*/

/*void termostat(void)                       // �������� �����������������
{
   if(Ttick <= t0)		                          
   PORTD |= (1<<NAGR);
    
   else
     {  
        if(Ttick > t0)	
        PORTD &= ~(1<<NAGR);  
     }
}*/

#pragma vector=TIMER0_OVF_vect
__interrupt void timer0_ovf_isr(void)
{
  //TIMER0 has overflowed
  TCNT0  = RELOAD;                          // reload counter value

  if (count_bod)  count_bod--;

  if(t_led_on)                              // LED-ON
  {
    PORTB &= ~(1<<LED); 
    t_led_on--;
  } 
  else                                      // LED-OFF
    PORTB |=  (1<<LED);              
}

// ������������ ������������� SPI � ������ Master
void SPI_MasterInit(void)
{
/* - ���������� SPI � ������ �������,
   - CPOL = 1 - ������������ �������� �������� ������������� ����������,
   - CPHA = 1 - � ��������� ������ ������������ �� ������� ������ ��������� ������� SCK,
   - ��������� �������� ������ fck/128 */
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

   *FUSEs:

WDTON               :   '0' [ programmed     ]
BODLEVEL 2-1-0 Fuses: '100' [ 4.3v (4.1-4.5) ]
*/