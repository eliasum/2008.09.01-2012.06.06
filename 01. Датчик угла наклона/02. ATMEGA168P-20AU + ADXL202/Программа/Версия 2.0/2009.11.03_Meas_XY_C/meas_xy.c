/********************************************************************************/
/*                                                                              */
/*                     ����������� ��������� ��� 'ATMEGA168P'                   */
/*                          ������ 2.0 (5 ������ 2009 �)                        */
/*                                                                              */
/********************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>

#define BYTE_ONE 0                         // ������� ��
#define BYTE_TWO 1                         // �����������

#undef F_CPU
#define F_CPU 11059200                     // ������� ����������
#define BAUDRATE 9600                      // �������� ������ UART
#define UBBR_UART ((F_CPU / (16L * BAUDRATE)) - 1)

// ����� "������� ����������" ( 1, 2, 4, 8 ��� 16 )
#define NSUM 16

// ������������ ��������������� ����.�������
#if NSUM == 2
  #define KDEL 1
#else
  #if NSUM == 4
    #define KDEL 2
  #else
    #if NSUM == 8
      #define KDEL 3
    #else
      #if NSUM == 16
        #define KDEL 4
      #else
        #define KDEL 0
      #endif
    #endif
  #endif
#endif

/*==============================================================================*/
/*                              ���������� �����������                          */
/*==============================================================================*/

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

unsigned long s;                           // 0..0x0ffffffff
unsigned char i;                           // 0..0x0ff
unsigned char get_one;                     // ����� ������

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
  PORTD = (1<<RS485)|(1<<KLINE)|(1<<SET_)|(1<<YOUT)|(1<<XOUT)|(1<<TMPR_RD)|(1<<TXD)|(1<<RXD);
  DDRD  = (1<<RS485)|(1<<KLINE)|(1<<TXD);
  PORTB = (1<<STR)|(1<<LED);
  DDRB  = (1<<STR)|(1<<DIR)|(1<<LED);

// ��������� �/�1
  TCCR1B = 0x00;                           // ������� �/�1
  TCNT1H = 0x00;
  TCNT1L = 0x00;                           // ����� �/�1

  get_one = 0;

  for(;;)
  {
    meas_xy();                             // �������� ���� �����

    //  ������ "����������� ��������":
                                           // ������� ����� �����
    Buff[get_one]=one;
                                           // ������� ������� �� "�������"
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1X.du_i;
    s=s>>KDEL; one.T1X.du_i = (unsigned int)(0x0ffff & s); // T1.X'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2X.du_i;
    s=s>>KDEL; one.T2X.du_i = (unsigned int)(0x0ffff & s); // T2.X'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1Y.du_i;
    s=s>>KDEL; one.T1Y.du_i = (unsigned int)(0x0ffff & s); // T1.Y'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2Y.du_i;
    s=s>>KDEL; one.T2Y.du_i = (unsigned int)(0x0ffff & s); // T2.Y'
    get_one++; if(get_one == NSUM) get_one = 0;

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
  one.T1X.du_c[BYTE_ONE] = TCNT1L;
  one.T1X.du_c[BYTE_TWO] = TCNT1H;         // -= ������������ �������� 'X' =-    
  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1

  loop_until_bit_is_clear(PIND,XOUT);      // ������������� ������� ������� ��������� � ������ 'Xout'
  loop_until_bit_is_set(PIND,XOUT);        // �������� ������ 2-�� �������� � ������ 'Xout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  loop_until_bit_is_clear(PIND,XOUT);      // �������� ����� 2-�� �������� � ������ 'Xout'
  loop_until_bit_is_set(PIND,XOUT);        // �������� ������ 3-�� �������� � ������ 'Xout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T2X.du_c[BYTE_ONE] = TCNT1L;
  one.T2X.du_c[BYTE_TWO] = TCNT1H;         // -= ������ �������� 'X' =-          

// ��������� ������������ � ������� ��������� � ������ 'Yout' -------= 'Y' =------

  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1
  loop_until_bit_is_clear(PIND,YOUT);      // ������������� ������� ������������ ��������� � ������ 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // �������� ������ 1-�� �������� � ������ 'Yout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  loop_until_bit_is_clear(PIND,YOUT);      // �������� ����� 1-�� �������� � ������ 'Yout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T1Y.du_c[BYTE_ONE] = TCNT1L;
  one.T1Y.du_c[BYTE_TWO] = TCNT1H;         // -= ������������ �������� 'Y' =-    
  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1

  loop_until_bit_is_clear(PIND,YOUT);      // ������������� ������� ������� ��������� � ������ 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // �������� ������ 2-�� �������� � ������ 'Yout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  loop_until_bit_is_clear(PIND,YOUT);      // �������� ����� 2-�� �������� � ������ 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // �������� ������ 3-�� �������� � ������ 'Yout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T2Y.du_c[BYTE_ONE] = TCNT1L;
  one.T2Y.du_c[BYTE_TWO] = TCNT1H;         // -= ������ �������� 'Y' =-  
}

void Tx_means(void)
{                                          // �� ������ ��������
  unsigned char d, cc;                     // � ������� � 'USART'

  cc = 0x05a;
  Tx_byte(cc);

  d = one.T1X.du_c[BYTE_ONE]; cc += d; Tx_byte(d);
  d = one.T1X.du_c[BYTE_TWO]; cc += d; Tx_byte(d);
  d = one.T2X.du_c[BYTE_ONE]; cc += d; Tx_byte(d);
  d = one.T2X.du_c[BYTE_TWO]; cc += d; Tx_byte(d);

  d = one.T1Y.du_c[BYTE_ONE]; cc += d; Tx_byte(d);
  d = one.T1Y.du_c[BYTE_TWO]; cc += d; Tx_byte(d);
  d = one.T2Y.du_c[BYTE_ONE]; cc += d; Tx_byte(d);
  d = one.T2Y.du_c[BYTE_TWO]; cc += d; Tx_byte(d);

  Tx_byte(cc);
}

void Tx_byte(char outbyte)                 // ����� ����� � 'USART'
{
  loop_until_bit_is_set(UCSR0A,UDRE0);     // ����� ������� ������ �����������
  UDR0 = outbyte;                          // ��������� ������� ���� ������                                           // � �����, ������ ��������
}

