/********************************************************************************/
/*                                                                              */
/*                     ����������� ��������� ��� 'ATMEGA168P'                   */
/*                         ������ 2.1 (9 ������ 2009 �)                         */
/*                                                                              */
/********************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>

#define BYTE_ONE 0                         // ������� ��
#define BYTE_TWO 1                         // �����������

#undef F_CPU
#define F_CPU 11059200                     // ������� ����������
#define BR9K6 9600                         // �������� ������ 9600
#define BR2K4 2400                         // �������� ������ 2400
#define UBBR_9K6 ((F_CPU / (16L * BR9K6)) - 1)
#define UBBR_2K4 ((F_CPU / (16L * BR2K4)) - 1)

// ������� ���������� ���� = 600 ��:
#define CONST_RELOAD 0xB8                  // ��������� ������������ ������� '0'
#define CBOD 4                             // ��������� ���
#define SYSTEM_TICK 600                    // ��� ���������� ������� � ��
#define NDEV 5                             // ������� ����� ����������
#define RR_CONST 150                       // 0.25 ��� �� ����������� 'WDT'

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
#define RS485 PORTD7                       // ����� ���������� ���������� ���� 'RS485'  OUT (0)
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
#define cpl(sfr, bit) \
  if(sfr & (1<<bit))  \
  {                   \
   sfr &= ~(1<<bit);  \
  }                   \
  else                \
  {                   \
    sfr |= (1<<bit);  \
  }
#endif

/*==============================================================================*/
/*                                ���������                                     */
/*==============================================================================*/
void meas_xy(void);
void Tx_means_tick(void);
void Tx_byte(char outbyte);
void Get_Buff(void);
/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
struct MEASURE Buff[NSUM];                 // ������� �������
struct MEASURE one;                        // 1-� �����

unsigned long s;                           // 0..0x0ffffffff
unsigned char i;                           // 0..0x0ff
unsigned char get_one;                     // ����� ������

volatile unsigned char rr_cnt;             // �������� �� ����������� 'WDT'
volatile unsigned char rr_flg;             // ���� ����������� 'WDT'
volatile unsigned char count_bod;          // ������� ���
unsigned char fl_set;                      // ������� "���������"
unsigned char var;                         // ����. ����-� ������

/*==============================================================================*/
/*                                                                              */
/*                         ������� ������� ���������.                           */
/*                                                                              */
/*==============================================================================*/
int main(void)
{

// ������������� ������������ ������ �����/������
  PORTD = (1<<KLINE)|(1<<SET_)|(1<<YOUT)|(1<<XOUT)|(1<<TMPR_RD)|(1<<TXD)|(1<<RXD);
  DDRD  = (1<<RS485)|(1<<KLINE)|(1<<TXD);
  PORTB = (1<<STR)|(1<<LED);
  DDRB  = (1<<STR)|(1<<DIR)|(1<<LED);

  if(PIND & (1<<SET_)) fl_set = 1; else fl_set = 0;

  get_one = 0;                             // ������ ... � "������"

//////////// ������ � ������ -= ��������� =- /////////////////////////////////
  if(fl_set)
  {
    // ������������� USART (9600)         
    UCSR0A = 0x00;                         // ������ �� ����� ��������� ��������        
    UCSR0B = 0x00;                         // �������� �������� USART          
    UCSR0C = 0x06;                         // ������ ����� ������ 8 ���           
    UBRR0L = (unsigned char)UBBR_9K6;      // ������� �������� 9600
    UBRR0H = (unsigned char)(UBBR_9K6>>8);
    UCSR0B = 0x08;                         // ���������� ��������

    // ��������� �/�1
    TCCR1B = 0x00;                         // ������� �/�1
    TCNT1H = 0x00;
    TCNT1L = 0x00;                         // ����� �/�1

    for(;;)
    {
      Get_Buff();
      Tx_means_tick();                     // ������� "�������" ���� � 'USART'
      cpl(PORTB, STR);                     // c���� ����������� �������
    }
  }
//////////// ������ � ������ -= ������ =- ////////////////////////////////////
  else
  {
    // ������������� USART (2400)         
    UCSR0A = 0x00;                          // ������ �� ����� ��������� ��������        
    UCSR0B = 0x00;                          // �������� �������� USART          
    UCSR0C = 0x06;                          // ������ ����� ������ 8 ���           
    UBRR0L = (unsigned char)UBBR_2K4;       // ������� �������� 2400
    UBRR0H = (unsigned char)(UBBR_2K4>>8);
    UCSR0B = 0x08;                          // ���������� ��������

    // TIMER0 initialize - prescale:256
    // WGM:           Normal
    // desired value: 600Hz
    // actual value:  600.000Hz (0.0%)
    TCCR0B = 0x00;                          // ������� �/�0
    TCNT0  = 0xB8;                          // ���. ����-�
    TCCR0A = 0x00; 
    TCCR0B = 0x04;                          // ������ ���������

    TIMSK0 = 0x01;                          // �/�0 <- �������� ����������

    rr_cnt = RR_CONST;
    rr_flg = 0x0f;                          // + 16*0.25=4 ���
    cpl(PORTB, STR);                        // c���� ����������� �������

    SMCR |= (1<<SE);                        // SLEEP enable
                                                                          
    sei();                                  // ���������� ���������

    for(;;)
    {
      UCSR0B &= ~(1<<RXEN0);                // ����� �� 'RxD' 'OFF'
      count_bod = CBOD;                     // ������� ���
      do {
           if(!(PIND & (1<<RXD))) count_bod = CBOD;
      } while(count_bod);                   // �������� ������-�����

      UCSR0B |= (1<<RXEN0);                 // ����� �� 'RxD' 'ON'
      while(UCSR0A & (1<<RXC0))
            var = UDR0;                     // "������" �������� �����

      while(!(UCSR0A & (1<<RXC0)));         // "����" ����

      if(UCSR0B & (1<<RXB80))
      {
        var  = UDR0;
        var &= 0x0f;
        if(var == NDEV)                     // ���������� � ����� �������?
        {
          PORTB &= ~(1<<LED);               // LED-ON
        //////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////
          PORTB |=  (1<<LED);               // LED-OFF
        }
        Get_Buff();                         // � ���. ������ ��������� �����
        cpl(PORTB, STR);                    // c���� ����������� �������
        rr_flg = 0x0f;                      // + 16*0.25=4 ���
      }
    }
  }
//////////////////////////////////////////////////////////////////////////////
}

// �������� 1-�� ��������� � ����� � ������ ��������
void Get_Buff(void)
{
  meas_xy();                               // �������� ���� �����

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
}

void meas_xy(void)
{
// ��������� ������������ � ������� ��������� � ������ 'Xout' -------= X =--------

  cli();                                   //disable all interrupts

  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1
  while(PIND & (1<<XOUT));                 // ������������� ������� ������������ ��������� � ������ 'Xout'
  while(!(PIND & (1<<XOUT)));              // �������� ������ 1-�� �������� � ������ 'Xout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  while(PIND & (1<<XOUT));                 // �������� ����� 1-�� �������� � ������ 'Xout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T1X.du_c[BYTE_ONE] = TCNT1L;
  one.T1X.du_c[BYTE_TWO] = TCNT1H;         // -= ������������ �������� 'X' =-    
  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1

  while(PIND & (1<<XOUT));                 // ������������� ������� ������� ��������� � ������ 'Xout'
  while(!(PIND & (1<<XOUT)));              // �������� ������ 2-�� �������� � ������ 'Xout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  while(PIND & (1<<XOUT));                 // �������� ����� 2-�� �������� � ������ 'Xout'
  while(!(PIND & (1<<XOUT)));              // �������� ������ 3-�� �������� � ������ 'Xout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T2X.du_c[BYTE_ONE] = TCNT1L;
  one.T2X.du_c[BYTE_TWO] = TCNT1H;         // -= ������ �������� 'X' =-          

// ��������� ������������ � ������� ��������� � ������ 'Yout' -------= 'Y' =------

  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1
  while(PIND & (1<<YOUT));                 // ������������� ������� ������������ ��������� � ������ 'Yout'
  while(!(PIND & (1<<YOUT)));              // �������� ������ 1-�� �������� � ������ 'Yout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  while(PIND & (1<<YOUT));                 // �������� ����� 1-�� �������� � ������ 'Yout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T1Y.du_c[BYTE_ONE] = TCNT1L;
  one.T1Y.du_c[BYTE_TWO] = TCNT1H;         // -= ������������ �������� 'Y' =-    
  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1

  while(PIND & (1<<YOUT));                 // ������������� ������� ������� ��������� � ������ 'Yout'
  while(!(PIND & (1<<YOUT)));              // �������� ������ 2-�� �������� � ������ 'Yout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  while(PIND & (1<<YOUT));                 // �������� ����� 2-�� �������� � ������ 'Yout'
  while(!(PIND & (1<<YOUT)));              // �������� ������ 3-�� �������� � ������ 'Yout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T2Y.du_c[BYTE_ONE] = TCNT1L;
  one.T2Y.du_c[BYTE_TWO] = TCNT1H;         // -= ������ �������� 'Y' =-  
}

void Tx_means_tick(void)
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
  PORTB  |= (1<<DIR);                      // RS485 -> ����������� ��������
  UCSR0B &= ~(1<<RXEN0);                   // ���������� ���       ON
  UCSR0A |=  (1<<TXC0);
  UDR0 = outbyte;                          // �������� ���� ������
  while(!(UCSR0A & (1<<TXC0)));            // ����� ����� �������� �����
  PORTB &= ~(1<<DIR);                      // RS485 -> ����������� �����
  UCSR0B |=  (1<<RXEN0);                   // ���������� ���       OFF
}

// Timer 0 overflow interrupt service routine
ISR(TIMER0_OVF_vect)
{
  //TIMER0 has overflowed
  TCNT0  = CONST_RELOAD;       // reload counter value

  if(count_bod)  count_bod--;

  if(rr_flg & 0x0f)
  {
    if(rr_cnt) rr_cnt--;
    else
    {
      rr_flg--;
      rr_cnt = RR_CONST;
      cpl(PORTB, STR);
    }
  }
}

