/********************************************************************************/
/*                                                                              */
/*                     ����������� ��������� ��� 'ATMEGA168P'                   */
/*                         ������ 2.5 (7 ������� 2009 �)                        */
/*                                                                              */
/********************************************************************************/
#include "main.h"
#include <math.h>

#define PLATA_1 // 1,2 ��� 3 !!!
/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
struct MEASURE Buff[NSUM];                  // ������� �������
struct MEASURE one;                         // 1-� ����� -= XY =-
// ����� �����������:
unsigned char Ttick;                        // ������� �� ������� "�����"

float Teta_xy, Teta_yx;
int   iAx, iAy, iaAx, iaAy;

unsigned long s;                            // 0..0x0ffffffff
unsigned char i;                            // 0..0x0ff
unsigned char get_one;                      // ����� ������

volatile unsigned char rr_cnt;              // �������� �� ����������� 'WDT'
volatile unsigned char rr_flg;              // ���� ����������� 'WDT'
volatile unsigned char count_bod;           // ������� ���
unsigned char fl_set;                       // ������� "���������"
unsigned char var;                          // ����. ����-� ������

void Const_ini(void)                        // 1-� ��� ������������ ���������
{
  float Toffx, Toffy, X1, X2, Y1, Y2, Sx, Sy, Ax, Ay, Axy, Ayx;

#ifdef PLATA_1
  Toffx = 4095.1/8248;
  Toffy = 4375.1/8248;
  X1    = 3022.1/8248;
  X2    = 5127.1/8248;
  Y1    = 3350.1/8248;
  Y2    = 5436.1/8248;
#endif
#ifdef PLATA_2
  Toffx = 3095.1/8602;
  Toffy = 5331.1/8604;
  X1    = 3095.1/8602;
  X2    = 5331.1/8604;
  Y1    = 3144.1/8602;
  Y2    = 5253.1/8603;
#endif
#ifdef PLATA_3
  Toffx = 3836.1/8245;
  Toffy = 3952.1/8246;
  X1    = 2735.1/8245;
  X2    = 4894.1/8245;
  Y1    = 2940.1/8244;
  Y2    = 4998.1/8246;
#endif
  Sx = (X2 - X1)/2;
  Sy = (Y2 - Y1)/2;

  Ax = (one.T1X.du_i/one.T2X.du_i-Toffx)/Sx;
  Ay = (one.T1Y.du_i/one.T2Y.du_i-Toffy)/Sy;

  Axy = Ax/Ay;
  Ayx = Ay/Ax;

  Teta_xy = atan(Axy)*180/__PI;
  Teta_yx = atan(Ayx)*180/__PI;

  iAx = (int)Ax;
  iAy = (int)Ay;

  if(iAx < 0) iaAx = 65536 - iAx;
  else        iaAx = iAx;

  if(iAy < 0) iaAy = 65536 - iAy;
  else        iaAy = iAy;
}

/*==============================================================================*/
/*                                                                              */
/*                         ������� ������� ���������.                           */
/*                                                                              */
/*==============================================================================*/
int main(void)
{

// ������������� ������������ ������ �����/������
  PORTD = (1<<KLINE)|(1<<SET_)|(1<<YOUT)|(1<<XOUT)|(1<<TXD)|(1<<RXD);
  DDRD  = (1<<RS485)|(1<<KLINE)|(1<<TXD);
  PORTB = (1<<STR)|(1<<LED);
  DDRB  = (1<<STR)|(1<<DIR)|(1<<LED);

  if (PIND & (1<<SET_)) fl_set = 1; else fl_set = 0;

  get_one = 0;                              // ������ ... � "������"

//////////// ������ � ������ -= ��������� =- /////////////////////////////////
  if (fl_set)
  {
    // ������������� USART (9600):
    UCSR0A = 0x00;                          // ������ �� ����� ��������� ��������
    UCSR0B = 0x00;                          // �������� �������� USART
    UCSR0C = 0x06;                          // ������ ����� ������ 8 ���
    UBRR0L = (unsigned char)UBBR_9K6;       // ������� �������� 9600
    UBRR0H = (unsigned char)(UBBR_9K6>>8);
    UCSR0B = 0x08;                          // ���������� ��������

    // ������������� OpenWire:
    ow_set_bus(&ONEWIRE_IN,&ONEWIRE_OUT,&ONEWIRE_DDR,ONEWIRE_PIN);
    ow_reset();

    // ��������� �/�1:
    TCCR1B = 0x00;                          // ������� �/�1
    TCNT1H = 0x00;
    TCNT1L = 0x00;                          // ����� �/�1

    for (;;)
    {
      Get_Buff();                           // ������ -= XY =-
      Meas_Cel();
      Tx_means_tick();                      // ������� "�������" ���� � 'USART'
      cpl(PORTB, STR);                      // c���� ����������� �������
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

    Const_ini();                            // ��������� �������������
    for(i=0; i<NSUM; i++)                   // ��������� �������
    {
      meas_xy();
      Buff[i]=one;
    }
    cpl(PORTB, STR);                        // c���� ����������� �������
       
    __enable_interrupt();                   // ���������� ���������

    for (;;)
    {
      UCSR0B &= ~(1<<RXEN0);                // ����� �� 'RxD' 'OFF'
      count_bod = CBOD;                     // ������� ���
      do {
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
          float Lind;
          int rvrs_type;
          union u_int Ugol;

          PORTB &= ~(1<<LED);               // LED-ON
        //////////////////////////////////////////////////////////////////////

        //   IF ABS(Ay) >  ABS(Ax), Ay >  0,         Z1: v =        atan(Ax/Ay)
        //   IF ABS(Ay) <= ABS(Ax), Ax <  0,         Z2: v = -90  - atan(Ay/Ax)
        //   IF ABS(Ay) <= ABS(Ax), Ax >  0,         Z3: v =  90  - atan(Ay/Ax)
        //   IF ABS(Ay) >  ABS(Ax), Ax <  0, Ay < 0, Z4: v = -180 + atan(Ax/Ay)
        //   IF ABS(Ay) >  ABS(Ax), Ax >= 0, Ay < 0, Z5: v =  180 + atan(Ax/Ay)

          if(iaAy > iaAx)
          {
            if(iAy > 0)   Lind = Teta_xy;        // 'Z1'
            else
            {
              if(iAy < 0)
              {
                if(iAx >= 0)
                          Lind =  180 + Teta_xy; // 'Z5'
                else
                          Lind = -180 + Teta_xy; // 'Z4'
              }
            }
          }
          else     // 'Z2' ��� 'Z3'?
          {
            if(iAx > 0)   Lind =  90 -  Teta_yx; // 'Z3'
            else
            {
            	if(iAx < 0) Lind = -90 -  Teta_yx; // 'Z2'
            }
          }
          rvrs_type = (int)Lind;
          if(rvrs_type < 0)
          {
            Ugol.du_i = 65536 - rvrs_type;
            Ugol.du_c[1] |= 0x80;
          }
          else
            Ugol.du_i = rvrs_type;
          Tx_byte(Ugol.du_c[0]);
          Tx_byte(Ugol.du_c[1]);
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

  // ������ "����������� ��������":
  // ������� ����� �����
  Buff[get_one]=one;
  // ������� ������� �� "�������"
  s=0; for (i=0; i<NSUM; i++) s += Buff[i].T1X.du_i;
  s=s>>KDEL; one.T1X.du_i = (unsigned int)(0x0ffff & s); // T1.X'
  s=0; for (i=0; i<NSUM; i++) s += Buff[i].T2X.du_i;
  s=s>>KDEL; one.T2X.du_i = (unsigned int)(0x0ffff & s); // T2.X'
  s=0; for (i=0; i<NSUM; i++) s += Buff[i].T1Y.du_i;
  s=s>>KDEL; one.T1Y.du_i = (unsigned int)(0x0ffff & s); // T1.Y'
  s=0; for (i=0; i<NSUM; i++) s += Buff[i].T2Y.du_i;
  s=s>>KDEL; one.T2Y.du_i = (unsigned int)(0x0ffff & s); // T2.Y'
  get_one++; if (get_one == NSUM) get_one = 0;

  __enable_interrupt();                    // ���������� ���������
}

void meas_xy(void)
{
// ��������� ������������ � ������� ��������� � ������ 'Xout' -------= X =--------

  __disable_interrupt();                   //disable all interrupts

  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1
  while (PIND & (1<<XOUT));                // ������������� ������� ������������ ��������� � ������ 'Xout'
  while (!(PIND & (1<<XOUT)));             // �������� ������ 1-�� �������� � ������ 'Xout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  while (PIND & (1<<XOUT));                // �������� ����� 1-�� �������� � ������ 'Xout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T1X.du_c[BYTE_ONE] = TCNT1L;
  one.T1X.du_c[BYTE_TWO] = TCNT1H;         // -= ������������ �������� 'X' =-
  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1

  while (PIND & (1<<XOUT));                // ������������� ������� ������� ��������� � ������ 'Xout'
  while (!(PIND & (1<<XOUT)));             // �������� ������ 2-�� �������� � ������ 'Xout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  while (PIND & (1<<XOUT));                // �������� ����� 2-�� �������� � ������ 'Xout'
  while (!(PIND & (1<<XOUT)));             // �������� ������ 3-�� �������� � ������ 'Xout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T2X.du_c[BYTE_ONE] = TCNT1L;
  one.T2X.du_c[BYTE_TWO] = TCNT1H;         // -= ������ �������� 'X' =-

// ��������� ������������ � ������� ��������� � ������ 'Yout' -------= 'Y' =------

  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1
  while (PIND & (1<<YOUT));                // ������������� ������� ������������ ��������� � ������ 'Yout'
  while (!(PIND & (1<<YOUT)));             // �������� ������ 1-�� �������� � ������ 'Yout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  while (PIND & (1<<YOUT));                // �������� ����� 1-�� �������� � ������ 'Yout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T1Y.du_c[BYTE_ONE] = TCNT1L;
  one.T1Y.du_c[BYTE_TWO] = TCNT1H;         // -= ������������ �������� 'Y' =-
  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1

  while (PIND & (1<<YOUT));                // ������������� ������� ������� ��������� � ������ 'Yout'
  while (!(PIND & (1<<YOUT)));             // �������� ������ 2-�� �������� � ������ 'Yout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  while (PIND & (1<<YOUT));                // �������� ����� 2-�� �������� � ������ 'Yout'
  while (!(PIND & (1<<YOUT)));             // �������� ������ 3-�� �������� � ������ 'Yout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T2Y.du_c[BYTE_ONE] = TCNT1L;
  one.T2Y.du_c[BYTE_TWO] = TCNT1H;         // -= ������ �������� 'Y' =-
}

void Meas_Cel(void)
{
  DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );
  delay_ms(DS18B20_TCONV_10BIT);
  DS18X20_read_meas_in_tick(&Ttick);
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

  d = Ttick;                  cc += d; Tx_byte(d);

  Tx_byte(cc);
}

void Tx_byte(char outbyte)                 // ����� ����� � 'USART'
{
  PORTB  |= (1<<DIR);                      // RS485 -> ����������� ��������
  UCSR0B &= ~(1<<RXEN0);                   // ���������� ���       ON
  UCSR0A |=  (1<<TXC0);
  UDR0 = outbyte;                          // �������� ���� ������
  while (!(UCSR0A & (1<<TXC0)));           // ����� ����� �������� �����
  PORTB &= ~(1<<DIR);                      // RS485 -> ����������� �����
  UCSR0B |=  (1<<RXEN0);                   // ���������� ���       OFF
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
      cpl(PORTB, STR);
    }
  }
}

