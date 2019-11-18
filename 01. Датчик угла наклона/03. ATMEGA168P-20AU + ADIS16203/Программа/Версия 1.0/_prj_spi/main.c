/********************************************************************************/
/*                                                                              */
/*             ����������� ��������� ��� 'ATMEGA168P' + 'ADIS16203'             */
/*                         ������ 1.0 (10 ���� 2010 �)                          */
/*                                                                              */
/********************************************************************************/
#include "main.h"
#include <math.h>

/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
struct MEASURE one;                         // 1-� �����

unsigned char Ttick;                        // ������� �� ������� "�����"
int t0;

volatile unsigned char rr_cnt;              // �������� �� ����������� 'WDT'
volatile unsigned char rr_flg;              // ���� ����������� 'WDT'
volatile unsigned char count_bod;           // ������� ���

unsigned char fl_set;                       // ������� ��������� ������ ������

unsigned char var;                          // ����. ����-� ������

volatile unsigned int to_mc;                // 10 ���

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
  PORTD = (1<<RXD)|(1<<TXD)|(1<<STR)|(1<<SET_)|(1<<KLINE)|(1<<RST);
  DDRD  = (1<<TXD)|(1<<STR)|(1<<KLINE)|(1<<RST);
   
  PORTB = (1<<LED)|(1<<CS)|(1<<MOSI)|(1<<MISO);
  DDRB  = (1<<LED)|(1<<DIR)|(1<<CS)|(1<<MOSI)|(1<<SCK);
  
  #define PRT_RXD PIND

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

  rr_flg  = 0;                              // ������ "�������" ���������� ������
  to_mc   = TO_CEL_SET;                     // �������� ������ ����-��
  count_bod = 1;                            // ������� 1 ����������
  
  cpl(PORTD, STR);                          // 1-� "�������" c���� ����������� �������
  
  __enable_interrupt();                     // ���������� ���������

//////////// ������ � ������ -= ��������� =- /////////////////////////////////
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
    
    for (;;)
    {
      while(count_bod);                     // ����� ���
      count_bod = 1;                        // ������� 1 ����������
    
      int byte;
      
      PORTB &= ~(1<<CS);                    // ��������� ��������      
      byte = 0x0F;                          // ������ �� ������ �������� INCL_180_OUT
      SPI_WriteRead(byte);                  // �������� �������
      byte = 0x0E;                          // ������ �� ������ �������� INCL_180_OUT
      SPI_WriteRead(byte);                  // �������� �������
      PORTB |= (1<<CS);                     // ����������� ��������
           
      cpl(PORTD, STR);                      // "�������" c���� ����������� �������
    }
  }
//////////// ������ � ������ -= ������ =- ////////////////////////////////////
  else
  {
    // ������������� USART (2400)
    UCSR0B = 0x00;                          // ������ �� ����� ��������� ��������
    UCSR0A = 0x00;                          // �������� �������� USART
    UCSR0C = 0x06;                          // ������ ����� ������ 8 ���
    UBRR0L = (unsigned char)UBBR_2K4;       // ������� �������� 2400; ������ � datasheet
    UBRR0H = (unsigned char)(UBBR_2K4>>8);
    UCSR0B = 0x0C;     
   
    meas_xy();
    Meas_Cel();

    rr_cnt = RR_CONST;                      // ��������� ����������� �������
    rr_flg = 0x0f;                          // + 16*0.25=4 ���
    cpl(PORTD, STR);                        // c���� ����������� �������

    for (;;)
    {
      UCSR0B &= ~(1<<RXEN0);                // ����� �� 'RxD' 'OFF'
      count_bod = CBOD;                     // ������� ���

      do {
        if (!(PRT_RXD & (1<<RXD))) count_bod = CBOD;
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
          /*float Lind;
          int   rvrs_type;
          union u_int Ugol;*/

          cpl(PORTB, LED);                 // LED-TOGGLE ��������� ������ ��������� �� ���������������.
////////////////////////////////////////////////////////////////////////////////  
          
          //������� �������������� ���� � ����
          
////////////////////////////////////////////////////////////////////////////////  
          
          //termostat();                   
          
////////////////////////////////////////////////////////////////////////////////          
          /*rvrs_type = (int)(0.5 + Lind);
          if(rvrs_type < 0)
          {
            Ugol.du_i = 65536 - rvrs_type;
            Ugol.du_c[1] |= 0x80;
          }
          else
            Ugol.du_i = rvrs_type;
          Tx_byte(Ugol.du_c[0]);
          Tx_byte(Ugol.du_c[1]);*/
        ////////////////////////////////////////////////////////////////////////
          
        }
        meas_xy();                          // � ���. ������ ��������� �����
        Meas_Cel();
        rr_flg = 0x0f;                      // + 16*0.25=4 ���
        cpl(PORTD, STR);                    // c���� ����������� �������
      }
    }
  }
////////////////////////////////////////////////////////////////////////////////
}

void meas_xy(void)
{
  unsigned char sreg = SREG;
  __disable_interrupt();                   //disable all interrupts

////////////////////////////////////////////////////////////////////////////////  
          
          //�������������� ���� � ����
          
//////////////////////////////////////////////////////////////////////////////// 
  
  SREG = sreg;                             // ��������� ���������� (���� ��� ���� �����������)
}

void Meas_Cel(void)
{
  if(to_mc > (TO_CEL_SET-1))
  {
    unsigned char sreg = SREG;
    __disable_interrupt();                 //disable all interrupts

////////////////////////////////////////////////////////////////////////////////  
          
          //��������� �������� �����������
          
////////////////////////////////////////////////////////////////////////////////     
    
    to_mc = 0;
    SREG = sreg;                           // ��������� ����������
  }
}

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
     
void Tx_means_tick(void)
{                                          // �� ������ ��������
  unsigned char d, cc;                     // � ������� � 'USART'

  cc = 0x05a;
  Tx_byte(cc);

  d = one.T1X.du_c[BYTE_ONE]; cc += d; Tx_byte(d);
  d = one.T1X.du_c[BYTE_TWO]; cc += d; Tx_byte(d);
  d = one.T1X.du_c[BYTE_ONE]; cc += d; Tx_byte(d);
  d = one.T1X.du_c[BYTE_TWO]; cc += d; Tx_byte(d);
  
  d = one.T1Y.du_c[BYTE_ONE]; cc += d; Tx_byte(d);
  d = one.T1Y.du_c[BYTE_TWO]; cc += d; Tx_byte(d);
  d = one.T1Y.du_c[BYTE_ONE]; cc += d; Tx_byte(d);
  d = one.T1Y.du_c[BYTE_TWO]; cc += d; Tx_byte(d); 

  d = Ttick;                  cc += d; Tx_byte(d);

  Tx_byte(cc);
}

void Tx_byte(char outbyte)                 // ����� ����� � 'USART'
{
  PORTB  |= (1<<DIR);                      // RS485 -> ����������� ��������
  UCSR0A |=  (1<<TXC0);
  UCSR0B &= ~(1<<TXB80);                   // 9-� ��� -> '0'      (������)
  UDR0 = outbyte;                          // �������� ���� ������
  while (!(UCSR0A & (1<<TXC0)));           // ����� ����� �������� �����
  PORTB &= ~(1<<DIR);                      // RS485 -> ����������� �����
}

#pragma vector=TIMER0_OVF_vect
__interrupt void timer0_ovf_isr(void)
{
  //TIMER0 has overflowed
  TCNT0  = CONST_RELOAD;       // reload counter value

  if (count_bod)  count_bod--;

  if (rr_flg & 0x0f)
  {
    if (rr_cnt) rr_cnt--;
    else
    {
      rr_flg--;
      rr_cnt = RR_CONST;
      cpl(PORTD, STR);
    }
  }
  if(to_mc < TO_CEL_SET) to_mc++;
}

// ������������ ������������� SPI � ������ Master
void SPI_MasterInit(void)
{
/* - ���������� SPI � ������ �������,
   - CPOL = 1 - ������������ �������� �������� ������������� ����������,
   - CPHA = 1 - � ��������� ������ ������������ �� ������� ������ ��������� ������� SCK,
   - ��������� �������� ������ fck/128 */
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPOL)|(1<<CPHA)|(1<<SPR0)|(1<<SPR1);
}

// ������������ ������/������� ����� SPI
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