/********************************************************************************/
/*                                                                              */
/*             ����������� ��������� ��� 'ATMEGA168P' + 'ADIS16203'             */
/*                        ������ 1.7 (25 ������ 2010 �)                         */
/*                                                                              */
/********************************************************************************/
#include "main.h"
#include <math.h>
/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
unsigned int Buff[NSUM];                    // ������� �������
unsigned char N_zamera;                     // ����� ������
unsigned long sum;                          // 0..0x0ffffffff
unsigned char i;                            // 0..0x0ff

union u_int angle, tick;
union u_sint angle_180, tick_180;
union u_sint temp, tock;

volatile unsigned char count_bod;           // ������� ���
volatile unsigned char t_led_on;            // 0.2 ���

unsigned char fl_set;                       // ������� ��������� ������ ������
unsigned char const __flash Title[] = "ADIS16203 - convertion of Ticks to Angle";
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
  
  #define NAGR_ON      PORTD |=  (1<<NAGR)
  #define NAGR_OFF     PORTD &= ~(1<<NAGR)
  #define RST_ENABLE   PORTD &= ~(1<<RST)
  #define RST_DISABLE  PORTD |=  (1<<RST)
  #define LED_ON       PORTB &= ~(1<<LED)
  #define LED_OFF      PORTB |=  (1<<LED)
  #define CS_SET       PORTB |=  (1<<CS)
  #define CS_RES       PORTB &= ~(1<<CS)
  #define LED_TOGGLE   cpl(PORTB, LED)
  
  // ����� ������ ������ �������:
  if (PIND & (1<<SET_)) fl_set = 1; else fl_set = 0;

  TCCR0B = 0x00;                            // ������� �/�0
  TCNT0  = 0xB8;                            // ���. ����-�
  TCCR0A = 0x00;
  TCCR0B = 0x04;                            // ������ ���������
  TIMSK0 = 0x01;                            // �/�0 <- �������� ����������
  
  N_zamera  = 0;                            // ������ ... � "������"
  count_bod = 1;                            // ������� 1 ����������
  t_led_on  = 0;                            // ����� ���������
  
  __watchdog_reset();                       // 1-� "�������" c���� ����������� �������

  __enable_interrupt();                     // ���������� ���������

//////////////////////// ������ � ������ -= ��������� =- ///////////////////////
  if (!fl_set)
  {
    // ������������� USART (9600):
    UCSR0B = 0x00;                          // ������ �������� USART0 �� ����� ��������� �������� ��������
    UCSR0A = 0x00;                          
    UCSR0C = 0x06;                          // ������ ����� ������ 8 ���
    UBRR0L = (unsigned char)UBBR_9K6;       // ������� �������� 9600; ������ � datasheet
    UBRR0H = (unsigned char)(UBBR_9K6>>8);
    UCSR0B = 0x08;                          // ���������� �������� (TXENn = 1)
    
    RST_ENABLE;                             // ����� �������� ��������� ������������� 100 ���
    delay_us(100);
    RST_DISABLE;   
    SPI_MasterInit();                       // ������������� SPI � ������ Master
    
    // ����� ������ �� ���, uart_putc(0x0a); - ������� �� ����. ������  
    uart_puts_p(Title);                                 uart_putc(0x0d); uart_putc(0x0a);                        
    uart_puts("------------------------------------");  uart_putc(0x0d); uart_putc(0x0a);  
     
    for (;;)
    {      
      while(count_bod);                     // ����� ���
      count_bod = 1;                        // ������� 1 ���������� 
      
      unsigned char byte;
      CS_RES;                               // ��������� ��������
////////////////////////////////////////////////////////////////////////////////  
          
                          //����� ���� 360� (INCL_OUT)
          
////////////////////////////////////////////////////////////////////////////////        
      byte = INCL_180_OUT;                  // ����� ��������, ����� �� �������� � ����������� �����    
      SPI_WriteRead(byte);                  // ������ �� ������ �������� ����� �������� INCL_180_OUT
      tick.du_c[1] = SPDR;                  // ������� ���� ������ �� INCL_OUT; SPDR - SPI Data Register  
      tick.du_c[1] &= 0x3f;                 // ������ 14 ���
      
      byte = 0x0C;                          // ����� ������
      SPI_WriteRead(byte);                  // ������ �� ������ �������� ����� �������� INCL_180_OUT
      tick.du_c[0] = SPDR;                  // ������� ���� ������ �� INCL_OUT 
       
      uart_puts("Tick/Angle = 0x");         // ����� ������� Tick/Angle = 0x
      uart_puthex_byte(tick.du_c[1]);       // ����� �������� ����� ������ �� INCL_OUT
      uart_puthex_byte(tick.du_c[0]);       // ����� �������� ����� ������ �� INCL_OUT
      uart_puts("/");                       // ����� ����� / 
      
      tick.du_i = moving_average(tick.du_i);// ������ "����������� ��������"

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
           
      termostat(temp.du_si, 50);            // ����������� ������������ ~50�C
      
      LED_TOGGLE;                           // ��������� ������ ��������� �� ���������������
      CS_SET;                               // ����������� ��������

      __watchdog_reset();                   // "�������" c���� ����������� �������
      
    } // end of 'for (;;)' 
  } // end of 'if (!fl_set)' 
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
    
    RST_ENABLE;                             // ����� �������� ��������� ������������� 100 ���
    delay_us(100);
    RST_DISABLE;   
    SPI_MasterInit();                       // ������������� SPI � ������ Master
    
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
////////////////////////////////////////////////////////////////////////////////  
          
                          //����� ���� 360� (INCL_OUT)
          
////////////////////////////////////////////////////////////////////////////////            
          unsigned char byte;

          t_led_on  = T_LED_ON;             // ����� �������������
          LED_ON;                           // LED-ON
          CS_RES;                           // ��������� ��������
         
          byte = TEMP_OUT;                  // ����� ��������, ����� �� �������� � ����������� �����    
          SPI_WriteRead(byte);              // ������ �� ������ �������� ����� �������� TEMP_OUT
          tick.du_c[1] = SPDR;              // ������� ���� ������ �� INCL_OUT  
          tick.du_c[1] &= 0x3f;             // ������ 14 ���
      
          byte = 0x0C;                      // ����� ������
          SPI_WriteRead(byte);              // ������ �� ������ �������� ����� �������� TEMP_OUT
          tick.du_c[0] = SPDR;              // ������� ���� ������ �� INCL_OUT 
          
          // �������������� ����� � �������� ���� �������:
          angle.du_i = (unsigned int)(0.25*tick.du_i);   // 0.25 - �������� ���� ����� � ���������� ������
/*          
          2 ������� �� � 2 ������� ����� ������� "����������� ��������" 
          ���������� ��� ���������� �� ������� "��������" ���� �������� ���� 
          ������� �� 0� �� 359,9� ��� ��������� ���� �� 0� �� 359,9� � ��������,
          ������� ������� � ��������� ���������� ������� "���������� �������".
          ������ ������ "������" ��� ��������� ���� 134,9� �� 135,0� � ��������,
          �. �. ������ ������� ���� �� 0� �� 90� ��� �������������� �������.
*/
          if ((angle.du_i >= 0)&&(angle.du_i < 1350))    
            angle.du_i = angle.du_i + 2250;
          else
          if ((angle.du_i <= 3599)&&(angle.du_i >= 1350)) 
            angle.du_i = angle.du_i - 1350;

          // ������ "����������� ��������":
          angle.du_i = moving_average(angle.du_i);     
                
          if ((angle.du_i >= 2250)&&(angle.du_i < 3600))    
            angle.du_i = angle.du_i - 2250;
          else
          if ((angle.du_i <= 2249)&&(angle.du_i >= 0)) 
            angle.du_i = angle.du_i + 1350;
          
          uart_putc(angle.du_c[1]);         // ����� ���� �� ��� ��� SMON
          uart_putc(angle.du_c[0]);   
////////////////////////////////////////////////////////////////////////////////  
          
                          //����� ����������� (TEMP_OUT)
          
////////////////////////////////////////////////////////////////////////////////        
          byte = INCL_OUT;                  // ����� ��������, ����� �� �������� � ����������� �����      
          SPI_WriteRead(byte);              // ������ �� ������ �������� ����� �������� INCL_OUT
          tock.du_sc[1] = SPDR;             // ������� ���� ������ �� TEMP_OUT
          tock.du_sc[1] &= 0x0F;            // ������ 12 ���
 
          byte = 0x0A;                      // ����� ������
          SPI_WriteRead(byte);              // ������ �� ������ �������� ����� �������� INCL_OUT
          tock.du_sc[0] = SPDR;             // ������� ���� ������ �� TEMP_OUT 
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
      
          termostat(temp.du_si, 50);        // ����������� ������������ ~50�C          
          
          __watchdog_reset();               // c���� ����������� �������
          
        } // end of 'if (var == NDEV)' 
      } // end of 'if (UCSR0B & (1<<RXB80))' 
    } // end of 'for (;;)' 
  } // end of 'else' 
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
  TCNT0  = RELOAD_TIM0;                    // reload counter value

  if (count_bod)  count_bod--;

  if(t_led_on)  t_led_on--;                // ����� ������������� --
  else          LED_OFF;                   // LED-OFF
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

// ������������ ������� "����������� ��������"
unsigned int moving_average(unsigned int znachenie)
{
  Buff[N_zamera]=znachenie;                // ������� ����� ����� � "�������"
  // ������� ������� �� "�������":
  for (sum=0,i=0; i<NSUM; i++) sum += Buff[i];                           
  sum=sum>>KDEL; znachenie = (unsigned int)(0x0ffff & sum);
  N_zamera++; if (N_zamera == NSUM) N_zamera = 0;
  return znachenie;
}

// ������������ ���������� �����������������
void termostat(signed int temperatura, unsigned char t0)                       
{
   if(temperatura <= t0)		                          
   NAGR_ON;
    
   else
     {  
        if(temperatura > t0)	
        NAGR_OFF;  
     }
}
