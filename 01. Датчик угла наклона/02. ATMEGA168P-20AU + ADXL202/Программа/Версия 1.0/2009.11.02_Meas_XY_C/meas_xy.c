/********************************************************************************/
/*                                                                              */
/*                     ����������� ��������� ��� 'ATMEGA168P'                   */
/*                         ������ 1.0 (29 ������� 2009 �)                       */
/*                                                                              */
/********************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>

/*==============================================================================*/
/*                              ���������� �����������                          */
/*==============================================================================*/
#undef F_CPU
#define F_CPU 11059200                     // ������� ����������
#define BAUDRATE 9600                      // �������� ������ UART
#define UBBR_UART ((F_CPU / (16L * BAUDRATE)) - 1)

#define NSUM 25                            // ����������� ����������

// ������������� ����� D

// PIN� - ������� ������� ������, ������������ ����������� ������ ������ 
// PORTx - ������� ����� ������, ������������ ����������� ������/������
// DDRx - ������� ����������� ������, ������������ ����������� ������/������

#define RXD PIND0                          // ����� ����� ������ ������ ������          IN  (^)
#define TXD PORTD1                         // ����� ������ ������ �� �������            OUT (1)
#define TMPR_WR PORTD2                     // ����� ������� ������ � �����������        OUT (1)
#define TMPR_RD PIND2                      // ����� ����� ������ ������ � �����������   IN  (^)
#define XOUT PIND3                         // ����� ����� ������ ������ �� 'X'          IN  (^)
#define YOUT PIND4                         // ����� ����� ������ ������ �� 'Y'          IN  (^)
#define SET_ PIND5                         // ����� ����� ������������ ���������        IN  (^)
#define KLINE PORTD6                       // ����� ���������� ���������� ���� 'K-LINE' OUT (1)
                                           // "���������� ������ = 1"     
#define RS485 PORTD7                       // ����� ���������� ���������� ���� 'RS485'  OUT (1)
                                           // "���������� ������ = 0"

// ������������� ����� B
#define LED PORTB0                         // ����� ���������� ����������� �����������  OUT (1)
#define DIR PORTB1                         // ����� ���������� ������������ 'RS485'     OUT (0) 
                                           // "���������� �������� = 1"
#define STR PORTB2                         // ����� ���������� ���������� ��������      OUT (1)

/*==============================================================================*/
/*                                �������� �����                                */
/*==============================================================================*/
union u_int                                // ��� 2-� �������� ������
  {
   unsigned int  du_i;                     // data_union_int
   unsigned char du_c[2];                  // data_union_char
  };                                       // 256*[0] + [1]

struct MEASURE {                           // �������� ��������� 1-�� ������
  union u_int T1X;
  union u_int T2X;
  union u_int T1Y;
  union u_int T2Y;  };

/*==============================================================================*/
/*                                �������� ��������                             */
/*==============================================================================*/
// �������������� ����:
#ifndef cpl
#define cpl(sfr, bit)                 \
  if(bit_is_set(_SFR_BYTE(sfr), bit)) \
  {                                   \
    _SFR_BYTE(sfr) &= ~_BV(bit);      \
  }                                   \
  else                                \
  {                                   \
    _SFR_BYTE(sfr) |=  _BV(bit);      \
  }
#endif

/*==============================================================================*/
/*                                ���������                                     */
/*==============================================================================*/
void meas_xy(void);
void Tx_means(void);
void Tx_byte(char outbyte);

/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
struct MEASURE Buff[NSUM];                 // ������� �������
struct MEASURE one;                        // 1-� �����
struct MEASURE Z;                          // 1-� ����������� �����

unsigned long s1, s2, s3, s4;              // 0..0x0ffffffff
unsigned char i;                           // 0..0x0ff              
unsigned int n1, n2, n3, n4;               // ����� ������

/*==============================================================================*/
/*                                                                              */
/*                         ������� ������� ���������.                           */
/*                                                                              */
/*==============================================================================*/
int main(void)
{

// ������������� USART (9600)         
  UCSR0A = 0x00;                           // ������ �� ����� ��������� ��������        
  UCSR0B = 0x00;                           // �������� �������� USART          
  UCSR0C = 0x06;                           // ������ ����� ������ 8 ���           
  UBRR0L = (unsigned char)UBBR_UART;       // ������� �������� 9600
  UBRR0H = (unsigned char)(UBBR_UART>>8);  // ��� 2400 ���
  UCSR0B = 0x08;                           // ���������� ��������

// ������������� ������������ ������ �����/������
  PORTD = 0b11111111;
  DDRD  = 0b11000010;
  PORTB = 0b00000101;
  DDRB  = 0b00000111;

// ��������� �/�1
  TCCR1B = 0x00;                           // ������� �/�1
  TCNT1H = 0x00;
  TCNT1L = 0x00;                           // ����� �/�1

  s1=0; n1=0;
  s2=0; n2=0;
  s3=0; n3=0;
  s4=0; n4=0;

  for (i=0; i<25; i++) {Buff[i].T1X.du_i=0; 
                        Buff[i].T2X.du_i=0; 
                        Buff[i].T1Y.du_i=0; 
                        Buff[i].T2Y.du_i=0;}
  for(;;)
  {
    meas_xy();                             // �������� ���� �����

//  ������ "����������� ��������":

    s1 -= Buff[n1].T1X.du_i;
    s1 += one.T1X.du_i;
    Buff[n1].T1X.du_i=one.T1X.du_i;
    n1++;
    if (n1==NSUM) n1=0;
    Z.T1X.du_i=s1/NSUM;

    s2 -= Buff[n2].T2X.du_i;
    s2 += one.T2X.du_i;
    Buff[n2].T2X.du_i=one.T2X.du_i;
    n2++;
    if (n2==NSUM) n2=0;
    Z.T2X.du_i=s2/NSUM;

    s3 -= Buff[n3].T1Y.du_i;
    s3 += one.T1Y.du_i;
    Buff[n3].T1Y.du_i=one.T1Y.du_i;
    n3++;
    if (n3==NSUM) n3=0;
    Z.T1Y.du_i=s3/NSUM;

    s4 -= Buff[n4].T2Y.du_i;
    s4 += one.T2Y.du_i;
    Buff[n4].T2Y.du_i=one.T2Y.du_i;
    n4++;
    if (n4==NSUM) n4=0;
    Z.T2Y.du_i=s4/NSUM;      

    Tx_means();                            // ������� "�������" � 'USART'

    cpl(PORTB, STR);                       // c���� ����������� �������
  }
}

void meas_xy(void)
{
// ��������� ������������ � ������� ��������� � ������ 'Xout' -------= X =--------

  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1
  loop_until_bit_is_clear(PIND,XOUT);      // ������������� ������� ������������ ��������� � ������ 'Xout'
  loop_until_bit_is_set(PIND,XOUT);        // �������� ������ 1-�� �������� � ������ 'Xout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  loop_until_bit_is_clear(PIND,XOUT);      // �������� ����� 1-�� �������� � ������ 'Xout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T1X.du_c[0] = TCNT1L;
  one.T1X.du_c[1] = TCNT1H;                // -= ������������ �������� 'X' =-    
  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1

  loop_until_bit_is_clear(PIND,XOUT);      // ������������� ������� ������� ��������� � ������ 'Xout'
  loop_until_bit_is_set(PIND,XOUT);        // �������� ������ 2-�� �������� � ������ 'Xout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  loop_until_bit_is_clear(PIND,XOUT);      // �������� ����� 2-�� �������� � ������ 'Xout'
  loop_until_bit_is_set(PIND,XOUT);        // �������� ������ 3-�� �������� � ������ 'Xout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T2X.du_c[0] = TCNT1L;
  one.T2X.du_c[1] = TCNT1H;                // -= ������ �������� 'X' =-          

// ��������� ������������ � ������� ��������� � ������ 'Yout' -------= 'Y' =------

  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1
  loop_until_bit_is_clear(PIND,YOUT);      // ������������� ������� ������������ ��������� � ������ 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // �������� ������ 1-�� �������� � ������ 'Yout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  loop_until_bit_is_clear(PIND,YOUT);      // �������� ����� 1-�� �������� � ������ 'Yout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T1Y.du_c[0] = TCNT1L;
  one.T1Y.du_c[1] = TCNT1H;                // -= ������������ �������� 'Y' =-    
  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1

  loop_until_bit_is_clear(PIND,YOUT);      // ������������� ������� ������� ��������� � ������ 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // �������� ������ 2-�� �������� � ������ 'Yout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  loop_until_bit_is_clear(PIND,YOUT);      // �������� ����� 2-�� �������� � ������ 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // �������� ������ 3-�� �������� � ������ 'Yout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T2Y.du_c[0] = TCNT1L;
  one.T2Y.du_c[1] = TCNT1H;                // -= ������ �������� 'Y' =- 
}

void Tx_means(void)
{                                          // �� ������ ��������
  unsigned char d, cc;                     // � ������� � 'USART'

  cc = 0x05a;
  Tx_byte(cc);

  d = Z.T1X.du_c[0]; cc += d; Tx_byte(d);
  d = Z.T1X.du_c[1]; cc += d; Tx_byte(d);
  d = Z.T2X.du_c[0]; cc += d; Tx_byte(d);
  d = Z.T2X.du_c[1]; cc += d; Tx_byte(d);

  d = Z.T1Y.du_c[0]; cc += d; Tx_byte(d);
  d = Z.T1Y.du_c[1]; cc += d; Tx_byte(d);
  d = Z.T2Y.du_c[0]; cc += d; Tx_byte(d);
  d = Z.T2Y.du_c[1]; cc += d; Tx_byte(d);

  Tx_byte(cc);
}

void Tx_byte(char outbyte)                 // ����� ����� � 'USART'
{

  loop_until_bit_is_set(UCSR0A,UDRE0);     // ����� ������� ������ �����������
  UDR0 = outbyte;                          // ��������� ������� ���� ������                                           // � �����, ������ ��������
}


// http://avr-libc.narod.ru/group__avr__sfr.html
