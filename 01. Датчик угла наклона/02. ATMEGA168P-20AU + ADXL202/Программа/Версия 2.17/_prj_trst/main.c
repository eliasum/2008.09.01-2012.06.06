/********************************************************************************/
/*                                                                              */
/*                     ����������� ��������� ��� 'ATMEGA168P'                   */
/*                         ������ 2.17 (15 ������ 2010 �)                       */
/*                                                                              */
/********************************************************************************/
#include "main.h"
#include <math.h>

/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
struct MEASURE Buff[NSUM];                  // ������� �������
struct MEASURE one;                         // 1-� ����� -= XY =-

unsigned char Ttick;                        // ������� �� ������� "�����"

int Toffx, Toffy, X1, X2, Y1, Y2, t0;
float Sx, Sy, Lmin, Lmax;

unsigned long s;                            // 0..0x0ffffffff
unsigned char i;                            // 0..0x0ff
unsigned char get_one;                      // ����� ������

volatile unsigned char rr_cnt;              // �������� �� ����������� 'WDT'
volatile unsigned char rr_flg;              // ���� ����������� 'WDT'
volatile unsigned char count_bod;           // ������� ���

unsigned char fl_set;                       // ������� ��������� ������ ������
unsigned char set_int_ext;                  // ������� ������ ����������(flash) ��� �������(eeprom) ��������

unsigned char var;                          // ����. ����-� ������

volatile unsigned int to_mc;                // 10 ���

__no_init __eeprom __root int Yust_Dat[10] @0x0010;  
                                            // ����� ��� EEPROM
unsigned char Const_ini_ee(void)            // ��������� � EEPROM  
{
  unsigned char cc;
  union u_sig_int idat;

  cc = 0;
  idat.du_i = Yust_Dat[0]; cc =+ idat.du_c[0]; cc =+ idat.du_c[1];
   Toffy    = idat.du_i;  
  idat.du_i = Yust_Dat[1]; cc =+ idat.du_c[0]; cc =+ idat.du_c[1];
   X1       = idat.du_i;  
  idat.du_i = Yust_Dat[2]; cc =+ idat.du_c[0]; cc =+ idat.du_c[1];
   Toffx    = idat.du_i;  
  idat.du_i = Yust_Dat[3]; cc =+ idat.du_c[0]; cc =+ idat.du_c[1];
   Y2       = idat.du_i;  
  idat.du_i = Yust_Dat[4]; cc =+ idat.du_c[0]; cc =+ idat.du_c[1];
   X2       = idat.du_i;  
  idat.du_i = Yust_Dat[5]; cc =+ idat.du_c[0]; cc =+ idat.du_c[1];
   Y1       = idat.du_i;  
  idat.du_i = Yust_Dat[6]; cc =+ idat.du_c[0]; cc =+ idat.du_c[1];
   t0       = idat.du_i;  
  idat.du_i = Yust_Dat[7]; cc =+ idat.du_c[0]; cc =+ idat.du_c[1];
   Lmin     = idat.du_i/10.0;              // :10 ������� � ���������� PRJ_TO_HEX
  idat.du_i = Yust_Dat[8]; cc =+ idat.du_c[0]; cc =+ idat.du_c[1];
   Lmax     = idat.du_i/10.0;              // :10 ������� � ���������� PRJ_TO_HEX
  idat.du_i = Yust_Dat[9]; cc =+ idat.du_c[0]; cc =+ idat.du_c[1];
/*
  �� - ����������� ����� ���� CRC8. CRC8 = 0 - ��� 8-� ������ ����� ������
  ���������� � ��� ������ �����, ������� ����� � �������������� ���� ����
  8-� ������ ����� ������ ����������. ����� � �������������� ���� - ���
  ������� ��������������� ����� + 1.
*/
  if(cc) return (1);
  
  Sx = (X2 - X1)/2;
  Sy = (Y2 - Y1)/2;
    
  set_int_ext = 1;                          // "�������" (eeprom) ���������
  
  return (0);
} 

void Const_ini(void)                        // ��������� �� ������������� EEPROM
{
  Toffx = 4000;        
  Toffy = 4000;
  X1 = 3000;
  X2 = 5000;
  Y1 = 3000;
  Y2 = 5000;
  Lmin = 0;                                 // ���������� ����������������
  Lmax = 0;
  t0 = 180;                                 // ����������� ������������ ~50�C
  Sx = (X2 - X1)/2;
  Sy = (Y2 - Y1)/2;
  
  set_int_ext = 0;                          // "����������" (flash) ���������
}

/*==============================================================================*/
/*                                                                              */
/*                         ������� ������� ���������.                           */
/*                                                                              */
/*==============================================================================*/
int main(void)
{
/*
  �����������:
  - IN  (^) - ����� �����(������), ^ - ����������� ��������;
  - OUT (0) - ����� ������(������) ��������� ��������� 0, �������� ��������� 1;
  - OUT (1) - ����� ������(������) ��������� ��������� 1, �������� ��������� 0;
  ��������� � �������� ��������� ������� OUT ������������ �� �������. �����.
*/  
// ������������� ������������ ������ �����/������
#ifndef NDEBUG // m128-debug
#warning FOR DEBUG MODE ONLY!
  PORTE = (1<<KLINE)|(1<<SET_)|(1<<YOUT)|(1<<XOUT)|(1<<TXD)|(1<<RXD);
  DDRE  = (1<<RS485)|(1<<KLINE)|(1<<TXD);
  #define PRT_RXD PINE
#else // m168p-release
  PORTD = (1<<KLINE)|(1<<SET_)|(1<<YOUT)|(1<<XOUT)|(1<<TXD)|(1<<RXD);
  DDRD  = (1<<RS485)|(1<<KLINE)|(1<<TXD);
  #define PRT_RXD PIND
#endif //debug/release
  PORTB = (1<<STR)|(1<<LED);
  DDRB  = (1<<STR)|(1<<DIR)|(1<<LED);

  PORTC &= ~(1<<TRMS);                      // ���. �������� 0
  DDRC   =  (1<<TRMS);                      // ����������� ������ �� �����

  if (PIND & (1<<SET_)) fl_set = 1; else fl_set = 0; // ����� ������ ������ �������

  // ������������� OneWire (include 'ow_reset()'):
  ow_set_bus(&ONEWIRE_IN,&ONEWIRE_OUT,&ONEWIRE_DDR,ONEWIRE_PIN);

  // TIMER0 initialize - prescale:256
  // WGM:           Normal
  // desired value: 600Hz
  // actual value:  600.000Hz (0.0%)

#ifndef NDEBUG // m128-debug
  TCCR0 = 0x00;                             // ������� �/�0
  TCNT0 = 0xB8;                             // ���. ����-�
  TCCR0 = 0x06;                             // ������ ���������
  TIMSK = 0x01;                             // �/�0 <- �������� ����������
#else // m168p-release
  TCCR0B = 0x00;                            // ������� �/�0
  TCNT0  = 0xB8;                            // ���. ����-�
  TCCR0A = 0x00;
  TCCR0B = 0x04;                            // ������ ���������
  TIMSK0 = 0x01;                            // �/�0 <- �������� ����������
#endif //debug/release

  rr_flg  = 0;                              // ������ "�������" ���������� ������
  to_mc   = TO_CEL_SET;                     // �������� ������ ����-��
  count_bod = 1;                            // ������� 1 ����������
  get_one = 0;                              // ������ ... � "������"

  cpl(PORTB, STR);                          // 1-� "�������" c���� ����������� �������

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

    // ��������� �/�1 � ����� ������� ��� ��������� ����-�� ���������
    TCCR1B = 0x00;                          // Timer/Counter1 Control Register B *3)
    // ^=> ������� �/�1
    TCNT1H = 0x00;                          // TCNT1H and TCNT1L � Timer/Counter1
    TCNT1L = 0x00;                          // ����� �/�1

    for (;;)
    {
      while(count_bod);                     // ����� ���
      count_bod = 1;                        // ������� 1 ����������
      Get_Buff();                           // ������ -= XY =-
      Meas_Cel();
      // �������� ������������ ����������� ������������ ~50�C
      t0 = 180;                             // ����������� ������������ ~50�C
      termostat();
      
      Tx_means_tick();                      // ������� "�������" ���� � 'USART'
      cpl(PORTB, STR);                      // "�������" c���� ����������� �������
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
#ifndef NDEBUG     // m128-debug
    UCSR0B = 0x0C; //(8)                    ?
#else              // m168p-release
    UCSR0B = 0x0C; //(9)                    ? (����������� '0x08')
#endif

    if(Const_ini_ee()) Const_ini();         // ��������� �������������
    else Const_ini_ee();                            
/*    
  ���� ������� Const_ini_ee() ���������� 1, ����� ����� ������� 
  Const_ini(), ����� ����� ������� Const_ini_ee().
*/ 
    
    for(i=0; i<NSUM; i++)                   // ��������� �������
    {
      meas_xy();
      Buff[i]=one;
    }
    Meas_Cel();

    rr_cnt = RR_CONST;                      // ��������� ����������� �������
    rr_flg = 0x0f;                          // + 16*0.25=4 ���
    cpl(PORTB, STR);                        // c���� ����������� �������

    for (;;)
    {
      UCSR0B &= ~(1<<RXEN0);                // ����� �� 'RxD' 'OFF'
      count_bod = CBOD;                     // ������� ���
#ifdef NDEBUG     // m168p-release
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
#else            // m128-debug
      {
        var = NDEV;       
#endif
        var &= 0x0f;
        if (var == NDEV)                    // ���������� � ����� �������?
        {
          float Ax, Ay, Axy, Ayx;
          float Teta_xy, Teta_yx, Lind;
          int   rvrs_type;
          union u_int Ugol;

          if(set_int_ext) PORTB &= ~(1<<LED); // LED-ON     [eeprom]
          else            cpl(PORTB, LED);    // LED-TOGGLE [flash]
/*
  ���� ��������� "�������"(eeprom), �.�. set_int_ext == 1, ����� ��������� ����������.
  ���� ��������� "����������"(flash), �.�. set_int_ext == 0, ����� ��������� ������
  ��������� �� ���������������.
*/    
          // ���������� � �������� ����:

          Ax = ((float)one.T1X.du_i-Toffx)/Sx;
          Ay = ((float)one.T1Y.du_i-Toffy)/Sy;

          Axy = Ax/Ay;
          Ayx = Ay/Ax;

          Teta_xy = atan(Axy)*180/__PI;
          Teta_yx = atan(Ayx)*180/__PI;

        ///////////////////////////////////////////////////////////////////////////
        //   ����:                                                               //
        //   IF ABS(Ay) >  ABS(Ax), Ay >  0,         Z1: v =        atan(Ax/Ay)  //
        //   IF ABS(Ay) <= ABS(Ax), Ax <  0,         Z2: v = -90  - atan(Ay/Ax)  //
        //   IF ABS(Ay) <= ABS(Ax), Ax >  0,         Z3: v =  90  - atan(Ay/Ax)  //
        //   IF ABS(Ay) >  ABS(Ax), Ax <  0, Ay < 0, Z4: v = -180 + atan(Ax/Ay)  //
        //   IF ABS(Ay) >  ABS(Ax), Ax >= 0, Ay < 0, Z5: v =  180 + atan(Ax/Ay)  //
        //                                                                       //
        ///////////////////////////////////////////////////////////////////////////

          if(fabs(Ay) > fabs(Ax))
          {
            if(Ay >= 0)     Lind = Teta_xy;        // 'Z1'
            else
            {
              if(Ay < 0)
              {
                if(Ax >= 0)
                            Lind =  180 + Teta_xy; // 'Z5'
                else
                            Lind = -180 + Teta_xy; // 'Z4'
              }
            }
          }
          else     // 'Z2' ��� 'Z3'?
          {
            if(Ax >= 0)     Lind =  90 -  Teta_yx; // 'Z3'
            else
            {
            	if(Ax < 0)    Lind = -90 -  Teta_yx; // 'Z2'
            }
          }

          if(Lind > -180 && Lind <= 90)   Lind = Lind + 90;  // �������� ��� �� 90�  
          else Lind = Lind - 270;
          
////////////////////////////////////////////////////////////////////////////////  
          
          termostat();                   
          
////////////////////////////////////////////////////////////////////////////////          
          rvrs_type = (int)(0.5 + Lind);
          if(rvrs_type < 0)
          {
            Ugol.du_i = 65536 - rvrs_type;
            Ugol.du_c[1] |= 0x80;
          }
          else
            Ugol.du_i = rvrs_type;
          Tx_byte(Ugol.du_c[0]);
          Tx_byte(Ugol.du_c[1]);
        ////////////////////////////////////////////////////////////////////////
          if(set_int_ext) PORTB |=  (1<<LED); // LED-OFF    [eeprom]
/*
  ���� ��������� "�������"(eeprom), �.�. set_int_ext == 1, ����� ��������� ������.
  ���� ��������� "����������"(flash), �.�. set_int_ext == 0, ����� ��������� �� 
  ������ ���� ���������.
*/ 
        }
        Get_Buff();                         // � ���. ������ ��������� �����
        Meas_Cel();
        rr_flg = 0x0f;                      // + 16*0.25=4 ���
        cpl(PORTB, STR);                    // c���� ����������� �������
      }
    }
  }
////////////////////////////////////////////////////////////////////////////////
}

// �������� 1-�� ��������� � ����� � ������ ��������
void Get_Buff(void)
{
  unsigned char sreg = SREG;
  __disable_interrupt();

  meas_xy();                               // �������� ���� �����

  // ������ "����������� ��������":
  // ������� ����� �����
  Buff[get_one]=one;
  // ������� ������� �� "�������"
  s=0; for (i=0; i<NSUM; i++) s += Buff[i].T1X.du_i;
  s=s>>KDEL; one.T1X.du_i = (unsigned int)(0x0ffff & s); // T1.X'
  s=0; for (i=0; i<NSUM; i++) s += Buff[i].T1Y.du_i;
  s=s>>KDEL; one.T1Y.du_i = (unsigned int)(0x0ffff & s); // T1.Y'
  get_one++; if (get_one == NSUM) get_one = 0;

  SREG = sreg;                             // ���������� ���������
}

void meas_xy(void)
{
  unsigned char sreg = SREG;
  __disable_interrupt();                   //disable all interrupts

// ��������� ������������ ��������� � ������ 'Xout' -------= X =--------

#ifndef NDEBUG // m128-debug
  one.T1X.du_i = 3066;                     // -= AEEOAEUIINOU EIIOEUNA 'X' =-
  one.T1Y.du_i = 4561;                     // -= AEEOAEUIINOU EIIOEUNA 'Y' =-
#else // m168p-release
 
  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1
  while (PIND & (1<<XOUT));                // ������������� ������� ������������ ��������� � ������ 'Xout'
  while (!(PIND & (1<<XOUT)));             // �������� ������ 1-�� �������� � ������ 'Xout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  while (PIND & (1<<XOUT));                // �������� ����� 1-�� �������� � ������ 'Xout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T1X.du_c[BYTE_ONE] = TCNT1L;
  one.T1X.du_c[BYTE_TWO] = TCNT1H;         // -= ������������ �������� 'X' =-
 
// ��������� ������������ ��������� � ������ 'Yout' -------= 'Y' =------

  TCNT1H = 0; TCNT1L = 0;                  // ����� �/�1
  while (PIND & (1<<YOUT));                // ������������� ������� ������������ ��������� � ������ 'Yout'
  while (!(PIND & (1<<YOUT)));             // �������� ������ 1-�� �������� � ������ 'Yout'
  TCCR1B = 0x02;                           // ������ �/�1 � �������� �������� CLK_io/8

  while (PIND & (1<<YOUT));                // �������� ����� 1-�� �������� � ������ 'Yout'
  TCCR1B = 0x00;                           // ������� �/�1
  one.T1Y.du_c[BYTE_ONE] = TCNT1L;
  one.T1Y.du_c[BYTE_TWO] = TCNT1H;         // -= ������������ �������� 'Y' =-
 
#endif //debug/release

  SREG = sreg;                             // ��������� ���������� (���� ��� ���� �����������)
}

void Meas_Cel(void)
{
#ifdef NDEBUG // m168p-release
  if(to_mc > (TO_CEL_SET-1))
  {
    unsigned char sreg = SREG;
    __disable_interrupt();                 //disable all interrupts
    DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );
    delay_ms(DS18B20_TCONV_10BIT);
    DS18X20_read_meas_in_tick(&Ttick);
    to_mc = 0;
    SREG = sreg;                           // ��������� ����������
  }
#else        // m128-debug        
  Ttick = 161;
#endif
}

void termostat(void)                       // �������� �����������������
{
   if(Ttick <= t0)		                          
   PORTC |= (1<<TRMS);
    
   else
     {  
        if(Ttick > t0)	
        PORTC &= ~(1<<TRMS);  
     }
}
     
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
      cpl(PORTB, STR);
    }
  }
  if(to_mc < TO_CEL_SET) to_mc++;
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
   *3)
Bit    7       6     5     4       3       2      1      0
     ICNC1 | ICES1 | � | WGM13 | WGM12 | CS12 | CS11 | CS10 | TCCR1B
       0   |       |   |       |       |      |      |      | Input Capture Noise Canceler
           |   0   |   |       |       |      |      |      | Input Capture Edge Select
           |       | 0 |       |       |      |      |      | Reserved Bit
           |       |   |   0   |   0   |      |      |      | Waveform Generation Mode
           |       |   |       |       |   0  |   0  |   0  | No clock source (Timer/Counter stopped).
������� �/�1
*/