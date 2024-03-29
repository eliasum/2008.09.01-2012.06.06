/********************************************************************************/
/*                                                                              */
/*             ����������� ��������� ��� 'ATMEGA168P' + 'ADIS16209'             */
/*                        ������ 1.2 (26 ����� 2012 �)                          */
/*                                                                              */
/********************************************************************************/
#include "main.h"
#include <math.h>
/*==============================================================================*/
/*                                                                              */
/*                         ������� ������� ���������.                           */
/*                                                                              */
/*==============================================================================*/
int main(void)
{
  // ������� ��������� ������ WDT: 
  __watchdog_reset();                          // WDTCSR = WDIF WDIE WDP3 WDCE WDE WDP2 WDP1 WDP0 (XXXX XXXX)
  WDTCSR |= (1<<WDCE) | (1<<WDE);              // ���������� ���������/���������� ������ WDT      (XXX1 1XXX)
  WDTCSR  = (1<<WDE)  | (1<<WDP3);             // ��������� ����-���� = 512K ������ (~ 4.0 �)     (0010 1000)

  PORTB = (1<<LED)|(1<<CS)|(1<<MOSI)|(1<<MISO)|(1<<SCK);
  DDRB  = (1<<LED)|(1<<CS)|(1<<MOSI)|(1<<SCK);

  PORTD = (1<<RXD)|(1<<TXD)|(1<<RST)|(1<<SET_);
  DDRD  = (1<<TXD)|(1<<RST)|(1<<TST)|(1<<RST);
  
  // ����� ������ ������ �������:
  if (PIND & (1<<SET_)) fl_set = 1; else fl_set = 0;
  
  N_zamera  = 0;                               // ������ ... � "������"
  count_bod = 1;                               // ������� 1 ����������
  t_led_on  = 0;                               // ����� ���������
  
  __watchdog_reset();                          // 1-� "�������" c���� ����������� �������

  __enable_interrupt();                        // ���������� ���������

//////////////////////// ������ � ������ -= ��������� =- ///////////////////////
  if (!fl_set)
  {
    // ������������� USART (9600):
    UCSR0B = 0x00;                             // ������ �������� USART0 �� ����� ��������� �������� ��������
    UCSR0A = 0x00;                          
    UCSR0C = 0x06;                             // ������ ����� ������ 8 ���
    UBRR0L = (unsigned char)UBBR_9K6;          // ������� �������� 9600; ������ � datasheet
    UBRR0H = (unsigned char)(UBBR_9K6>>8);
    UCSR0B = 0x08;                             // ���������� �������� (TXENn = 1)
    
    // ������������� �/�0:
    TCCR0B = 0x00;                             // ������� �/�0
    TCNT0  = RELOAD_TIM0;                      // ���. ����-�
    TCCR0A = 0x00;
    TCCR0B = START_TIM0;                       // ������ ���������
    TIMSK0 = 0x01;                             // �/�0 <- �������� ����������
    
    RST_ENABLE;                            
    delay_ms(1);                               // ����� �������� ��������� ������������� 1 ��
    RST_DISABLE;   
    delay_ms(250);                             // �����, ����������� ��� ������������ ���������� �������
    
    SPI_MasterInit();                          // ������������� SPI � ������ Master
    
    // ����� ������ �� ���, uart_putc(0x0a); - ������� �� ����. ������  
    uart_puts_p(Title);                                 uart_putc(0x0d); uart_putc(0x0a);                        
    uart_puts("------------------------------------");  uart_putc(0x0d); uart_putc(0x0a);  
    
    for (;;)
    {      
      while(count_bod);                        // ����� ���
      count_bod = 1;                           // ������� 1 ���������� 
      
      unsigned char byte;
////////////////////////////////////////////////////////////////////////////////  
          
                        //����� ���� X = �90� (XINCL_OUT)
          
////////////////////////////////////////////////////////////////////////////////  
      CS_RES;                                  // ��������� ��������
      
      byte = YINCL_OUT;                        // ����� ��������, ����� ������ �� �������� � ��������� �����    
      tick_x.du_sc[1] = SPI_WriteRead(byte);   // ������� ���� ������ XINCL_OUT
      tick_x.du_sc[1] &= 0x3f;                 // ������ 14 ���
      
      byte = 0x0F;                             // ����� ������
      tick_x.du_sc[0] = SPI_WriteRead(byte);   // ������� ���� ������ XINCL_OUT 
/*
      ���� ����� �����. ����� ����� �� ��� ���� - "����������" ����� �� ������������ 
      ������� 16 ���, � ������� 16-� ��� �������� ���������� � ����� �����:
*/
      tick_x.du_si = tick_x.du_si << 2; 
/*
      �������� �� 4 ����� "���������� �������" � ��������� 14-� �������� ������� 
      � ����������� ������� �������� ���� ����� ��� ���������:
*/ 
      tick_x.du_si /= 4;   
      
      uart_puts("Tick_x/Angle_x = 0x");        // ����� ������� Tick_x/Angle_x = 0x
      uart_puthex_byte(tick_x.du_sc[1]);       // ����� �������� ����� ������ XINCL_OUT
      uart_puthex_byte(tick_x.du_sc[0]);       // ����� �������� ����� ������ XINCL_OUT
      uart_puts("/");                          // ����� ����� / 

      // �������������� ����� � �������� ����� �������:
      angle_x.du_si = (unsigned int)(0.25*tick_x.du_si);
      
      uart_puti(angle_x.du_si);                // ����� �������� ���� ������� � ���������� ��������� ��� �������
      uart_puts("`; ");                        // ����� ������ `;  
      uart_putc(0x0d);                         // ������� �������
      uart_putc(0x0a);                         // ������� �� ��������� ������ 
              
      CS_SET;                                  // ����������� ��������
      delay_us(5);
////////////////////////////////////////////////////////////////////////////////  
          
                        //����� ���� Y = �90� (YINCL_OUT)
          
////////////////////////////////////////////////////////////////////////////////       
      CS_RES;                                  // ��������� ��������
      
      byte = TEMP_OUT;                         // ����� ��������, ����� ������ �� �������� � ��������� �����
      tick_y.du_sc[1] = SPI_WriteRead(byte);   // ������� ���� ������ YINCL_OUT  
      tick_y.du_sc[1] &= 0x3F;                 // ������ 14 ���
      
      byte = 0x0B;                             // ����� ������
      tick_y.du_sc[0] = SPI_WriteRead(byte);   // ������� ���� ������ YINCL_OUT 
/*
      ���� ����� �����. ����� ����� �� ��� ���� - "����������" ����� �� ������������ 
      ������� 16 ���, � ������� 16-� ��� �������� ���������� � ����� �����:
*/
      tick_y.du_si = tick_y.du_si << 2; 
/*
      �������� �� 4 ����� "���������� �������" � ��������� 14-� �������� ������� 
      � ����������� ������� �������� ���� ����� ��� ���������:
*/ 
      tick_y.du_si /= 4; 
      
      uart_puts("Tick_y/Angle_y = 0x");        // ����� ������� Tick_y/Angle_y = 0x
      uart_puthex_byte(tick_y.du_sc[1]);       // ����� �������� ����� ������ YINCL_OUT
      uart_puthex_byte(tick_y.du_sc[0]);       // ����� �������� ����� ������ YINCL_OUT
      uart_puts("/");                          // ����� ����� / 
      
      // �������������� ����� � �������� ����� �������:
      angle_y.du_si = (signed int)(0.25*tick_y.du_si);   
            
      uart_puti(angle_y.du_si);                // ����� �������� ���� ������� � ���������� ��������� ��� �������  
      uart_puts("`; ");                        // ����� ������ `;   
      uart_putc(0x0d);                         // ������� �������
      uart_putc(0x0a);                         // ������� �� ��������� ������ 
      
      CS_SET;                                  // ����������� ��������
      delay_us(5);
////////////////////////////////////////////////////////////////////////////////  
          
                          //����� ����������� (TEMP_OUT)
          
////////////////////////////////////////////////////////////////////////////////     
      CS_RES;                                  // ��������� ��������
      
      byte = XINCL_OUT;                        // ����� ��������, ����� ������ �� �������� � ��������� �����   
      tock.du_sc[1] = SPI_WriteRead(byte);     // ������� ���� ������ TEMP_OUT
      tock.du_sc[1] &= 0x0F;                   // ������ 12 ���
 
      byte = 0x0D;                             // ����� ������
      tock.du_sc[0] = SPI_WriteRead(byte);     // ������� ���� ������ TEMP_OUT 
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
      
      uart_puts("Tock/Temperature = 0x");      // ����� ������� Tock/Temperature = 0x
      uart_puthex_byte(tock.du_sc[1]);         // ����� �������� ����� ������ TEMP_OUT
      uart_puthex_byte(tock.du_sc[0]);         // ����� �������� ����� ������ TEMP_OUT
      uart_puts("/");                          // ����� ����� / 
      
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
            
      uart_puti(temp.du_si);                   // ����� �������� �����������
      uart_puts("`C");                         // ����� ������ `C
      
      uart_putc(0x0d);                         // ������� �������
      uart_putc(0x0a);                         // ������� �� ��������� ������ 
      delay_ms(250);                           // �������� ��� ���������� ���������
           
      termostat(temp.du_si, 50);               // ����������� ������������ ~50�C
      
      LED_TOGGLE;                              // ��������� ������ ��������� �� ���������������
      
      CS_SET;                                  // ����������� ��������

      __watchdog_reset();                      // "�������" c���� ����������� �������
      
    } // end of 'for (;;)' 
  } // end of 'if (!fl_set)' 
///////////////////////// ������ � ������ -= ������ =- /////////////////////////
  else
  {
    // ������������� USART (2400):
    UCSR0B = 0x00;                             // ������ �������� USART �� ����� ��������� �������� ��������
    UCSR0A = 0x00;                             
    UCSR0C = 0x06;                             // ������ ����� ������ 8 ���
    UBRR0L = UBBR_2K4L;                        // ������� �������� 2400
    UBRR0H = UBBR_2K4H;   
    UCSR0B = 0x0C;                             // ���������� �������� (TXENn = 1); 9-Bit

    // ������������� �/�0:
    TCCR0B = 0x00;                             // ������� �/�0
    TCNT0  = RELOAD_TIM0;                      // ���. ����-�
    TCCR0A = 0x00;
    TCCR0B = START_TIM0;                       // ������ ���������
    TIMSK0 = 0x01;                             // �/�0 <- �������� ����������
    
    RST_ENABLE;                            
    delay_ms(1);                               // ����� �������� ��������� ������������� 1 ��
    RST_DISABLE;   
    delay_ms(250);                             // �����, ����������� ��� ������������ ���������� �������
    
    SPI_MasterInit();                          // ������������� SPI � ������ Master
    
    __enable_interrupt();                      // ���������� ���������
////////////////////////////////////////////////////////////////////////////////  
          
                    //����������� ������ ����� ������ K-LINE                    
          
////////////////////////////////////////////////////////////////////////////////      
    for (;;)
    {  
      UCSR0B &= ~(1<<RXEN0);                   // ���������� ��������� USART
      count_bod = CBOD;                        // ������� ���

      do
      {
        if (!(PIND & (1<<RXD))) count_bod = CBOD;
      } while (count_bod);                     // �������� ������-����� (������ ������ ����� ��������)
        
      UCSR0B |= (1<<RXEN0);                    // ��������� ��������� USART

      while (!(UCSR0A & (1<<RXC0)));           // ���� ���������� ������ ����� 

      if (UCSR0B & (1<<RXB80))                 // ���� 9-� ��� =1, �� ������� �������
      {
        unsigned char var;
        var  = UDR0;                           // UDRn - USART I/O Data Register n
        var &= 0x0f;                           // ����� ������� �/� �� ����� 15
        if (var == PITCH)                      // ���������� � ����� �������?
        {    
////////////////////////////////////////////////////////////////////////////////  
          
                        //����� ���� X = �90� (XINCL_OUT)
          
////////////////////////////////////////////////////////////////////////////////  
         unsigned char byte;
         
         t_led_on  = T_LED_ON;                 // ����� �������������
         LED_ON;                               // LED-ON
         
         CS_RES;                               // ��������� ��������
      
         byte = XINCL_OUT;                     // ����� ��������, ����� ������ �� �������� � ��������� �����    
         tick_x.du_sc[1] = SPI_WriteRead(byte);// ������� ���� ������ XINCL_OUT
         tick_x.du_sc[1] &= 0x3f;              // ������ 14 ���
      
         byte = 0x0F;                          // ����� ������
         tick_x.du_sc[0] = SPI_WriteRead(byte);// ������� ���� ������ XINCL_OUT 
/*
         ���� ����� �����. ����� ����� �� ��� ���� - "����������" ����� �� ������������ 
         ������� 16 ���, � ������� 16-� ��� �������� ���������� � ����� �����:
*/
         tick_x.du_si = tick_x.du_si << 2; 
/*
         �������� �� 4 ����� "���������� �������" � ��������� 14-� �������� ������� 
         � ����������� ������� �������� ���� ����� ��� ���������:
*/ 
         tick_x.du_si /= 4;   
      
         // �������������� ����� � �������� ����� �������:
         angle_x.du_si = (unsigned int)(0.25*tick_x.du_si);
              
         CS_SET;                               // ����������� ��������
         
         uart_putc(angle_x.du_sc[1]);          // ����� ���� �� ��� x �� ��� ��� SMON
         uart_putc(angle_x.du_sc[0]);
        }
        
         if (var == ROLL)                      // ���������� � ����� �������?
        {
////////////////////////////////////////////////////////////////////////////////  
          
                        //����� ���� Y = �90� (YINCL_OUT)
          
////////////////////////////////////////////////////////////////////////////////
         unsigned char byte;
         
         t_led_on  = T_LED_ON;                 // ����� �������������
         LED_ON;                               // LED-ON
  
         CS_RES;                               // ��������� ��������
      
         byte = YINCL_OUT;                     // ����� ��������, ����� ������ �� �������� � ��������� �����
         tick_y.du_sc[1] = SPI_WriteRead(byte);// ������� ���� ������ YINCL_OUT  
         tick_y.du_sc[1] &= 0x3F;              // ������ 14 ���
      
         byte = 0x0B;                          // ����� ������
         tick_y.du_sc[0] = SPI_WriteRead(byte);// ������� ���� ������ YINCL_OUT 
/*
         ���� ����� �����. ����� ����� �� ��� ���� - "����������" ����� �� ������������ 
         ������� 16 ���, � ������� 16-� ��� �������� ���������� � ����� �����:
*/
         tick_y.du_si = tick_y.du_si << 2; 
/*
         �������� �� 4 ����� "���������� �������" � ��������� 14-� �������� ������� 
         � ����������� ������� �������� ���� ����� ��� ���������:
*/ 
         tick_y.du_si /= 4; 
               
         // �������������� ����� � �������� ����� �������:
         angle_y.du_si = (signed int)(0.25*tick_y.du_si);     
      
         CS_SET;                               // ����������� ��������  
          
         uart_putc(angle_y.du_sc[1]);          // ����� ���� �� ��� y �� ��� ��� SMON
         uart_putc(angle_y.du_sc[0]);
        }  
         __watchdog_reset();                   // c���� ����������� �������
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
  TCNT0  = RELOAD_TIM0;                        // reload counter value

  if (count_bod)  count_bod--;

  if(t_led_on)  t_led_on--;                    // ����� ������������� --
  else          LED_OFF;                       // LED-OFF
}

// ������������ ������������� SPI � ������ Master
void SPI_MasterInit(void)
{
/* 
  - ���������� SPI � ������ �������,
  - CPOL = 1 - ������������ �������� �������� ������������� ����������,
  - CPHA = 1 - � ��������� ������ ������������ �� ������� ������ ��������� ������� SCK,
  - ��������� �������� ������ fck/128 
*/
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

// ������������ ���������� �����������������
void termostat(signed int temperatura, unsigned char t0)                       
{
  if(temperatura <= t0)		                          
  TST_ON; 
  
  else
    {  
      if(temperatura > t0)	
      TST_OFF;  
    }
}
