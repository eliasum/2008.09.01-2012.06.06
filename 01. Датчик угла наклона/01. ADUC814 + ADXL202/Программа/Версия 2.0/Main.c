/********************************************************************************/
/*                                                                              */
/*                       ����������� ��������� ��� 'ADXL202'                    */
/*                          ������ 2.0 (15 ���� 2009 �)                         */
/*                                                                              */
/********************************************************************************/
#include "main.h"
// ��� ��������� � ��������� ������ "��������" '_'
#define SIMUL_DBG_

// ����� "������� ����������" ( 1, 2, 4, 8 ��� 16 )
#define NSUM 16

// ����� ������������ ������ ( < 256)
#define CNT_FRNT 256

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
/*                                �������� �����                                */
/*==============================================================================*/
union u_int                                 // ��� 2-� �������� ������
  {
   unsigned int  du_i;                      // data_union_int
   unsigned char du_c[2];                   // data_union_char
  };                                        // 256*[0] + [1]

idata struct MEASURE {                      // �������� ��������� 1-�� ������
  union u_int T1X;
  union u_int T2X;
  union u_int T1Y;
  union u_int T2Y;  };
/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
idata struct MEASURE Buff[NSUM];           // ������� �������
data struct MEASURE one;                   // 1-� �����

unsigned long s;                           // 0..0x0ffffffff
unsigned char i;                           // 0..0x0ff
unsigned char get_one;                     // ����� ������

/*==============================================================================*/
/*                                                                              */
/*                         ������� ������� ���������.                           */
/*                                                                              */
/*==============================================================================*/
void main(void)
{
#ifdef SIMUL_DBG
  for(i=0; i<NSUM; i++)
  {
    Buff[i].T1X.du_i = 0;
    Buff[i].T2X.du_i = 0;
    Buff[i].T1Y.du_i = 0;
    Buff[i].T2Y.du_i = 0;
  }
#endif

  IE = 0;                                  // ��� ���������� ���������

// ��������� �����������
  PLLCON = 0x001;                          // ���������� ������� ������� ����
                                           // Core Clk = XTALL*256 = 32768*256 = 8388608 ����
// ��������� ����������� �������
  WDWR = 1;                                // ���������� ������ � 'WDCON'
  WDCON = 0x072;                           // ������ WDT � ��������� ���������� 2�

// ��������� UART:
  SCON = 0x040;                            // ��������� ����������������� ����� �� 
                                           // ����������� 8-������� �����
// ��������� �/�0:
  TMOD = 0x001;                            // T/C0 - ������

// ��������� �/�2:
  RCAP2H = 0x0FF;                          // ������� �������� 9600 ��� 
  RCAP2L = 0x0E5;                          // (814code_asm)
  TH2    = 0x0FF;       
  TL2    = 0x0E5; 
  T2CON  = 0x030;                          // ��������� �/�2 � �������� ���������� �������� UART
  TR2    = 1;                              // ������ ���������� �������� 'UART'

  for(;;)
  {
    if(meas_xy())                          // �������� ���� �����
    {
      //  ������ "����������� ��������":
                                           // �������� "�������" � ��������� ����� �����
      for(i=NSUM-1; i>0; i--) Buff[i]=Buff[i-1]; Buff[0]=one;
                                           // ������� ������� �� "�������"
      s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1X.du_i;
      s=s>>KDEL; one.T1X.du_i = (unsigned int)(0x0ffff & s); // T1.X'
      s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2X.du_i;
      s=s>>KDEL; one.T2X.du_i = (unsigned int)(0x0ffff & s); // T2.X'
      s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1Y.du_i;
      s=s>>KDEL; one.T1Y.du_i = (unsigned int)(0x0ffff & s); // T1.Y'
      s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2Y.du_i;
      s=s>>KDEL; one.T2Y.du_i = (unsigned int)(0x0ffff & s); // T2.Y'

      Tx_means();                          // ������� "�������" � 'UART'

      // ����� ����������� �������
      EA   = 0;
      WDWR = 1;
      WDE  = 1;
      EA   = 1;
    }
  }
}

#ifndef SIMUL_DBG
  unsigned char meas_xy(void)
  {
    unsigned char cnt;

    TR0 = 0;                                 // ������� 'T0'

  // ��������� ������������ � ������� ��������� � ������ 'Xout' -------= X =--------
  
    cnt = CNT_FRNT;                          // ������������� ������������� ������
    TH0 = 0; TL0 = 0;                        // ����� 'T0'
    while(XOUT);                             // ������� ������ ���� '0'

    while(!XOUT && cnt) cnt--;               // ������������� �� '0' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")
    cnt = CNT_FRNT;                          // ������������� ������������� ������

    while(!XOUT);                            // �������� ������ 1-�� �������� � ������ 'Xout'
    TR0 = 1;                                 // ������ 'T0'

    while(XOUT && cnt) cnt--;                // ������������� �� '1' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")
    cnt = CNT_FRNT;                          // ������������� ������������� ������

    while(XOUT);                             // �������� ����� 1-�� �������� � ������ 'Xout'
    TR0 = 0;                                 // ������� 'T0'

    while(!XOUT && cnt) cnt--;               // ������������� �� '0' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")

    one.T1X.du_c[BYTE_ONE] = TL0;
    one.T1X.du_c[BYTE_TWO] = TH0;                   // -= ������������ �������� 'X' =-    *

    cnt = CNT_FRNT;                          // ������������� ������������� ������
    TH0 = 0; TL0 = 0;                        // ����� 'T0'
  
    while(!XOUT);                            // �������� ������ 2-�� �������� � ������ 'Xout'
    TR0 = 1;                                 // ������ 'T0'

    while(XOUT && cnt) cnt--;                // ������������� �� '1' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")
    cnt = CNT_FRNT;                          // �������������� ������������� ������

    while(XOUT);                             // �������� ����� 2-�� �������� � ������ 'Xout'

    while(!XOUT && cnt) cnt--;               // ������������� �� '0' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")
    cnt = CNT_FRNT;                          // �������������� ������������� ������

    while(!XOUT);                            // �������� ������ 3-�� �������� � ������ 'Xout'
    TR0 = 0;                                 // ������� 'T0'

    while(XOUT && cnt) cnt--;                // ������������� �� '1' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")

    one.T2X.du_c[BYTE_ONE] = TL0;
    one.T2X.du_c[BYTE_TWO] = TH0;            // -= ������ �������� 'X' =-          *
  
  // ��������� ������������ � ������� ��������� � ������ 'Yout' -------= 'Y' =------
  
    cnt = CNT_FRNT;                          // ������������� ������������� ������
    TH0 = 0; TL0 = 0;                        // ����� 'T0'
    while(YOUT);                             // ������� ������ ���� '0'

    while(!YOUT && cnt) cnt--;               // ������������� �� '0' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")
    cnt = CNT_FRNT;                          // ������������� ������������� ������

    while(!YOUT);                            // �������� ������ 1-�� �������� � ������ 'Yout'
    TR0 = 1;                                 // ������ 'T0'

    while(YOUT && cnt) cnt--;                // ������������� �� '1' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")
    cnt = CNT_FRNT;                          // ������������� ������������� ������

    while(YOUT);                             // �������� ����� 1-�� �������� � ������ 'Yout'
    TR0 = 0;                                 // ������� 'T0'

    while(!YOUT && cnt) cnt--;               // ������������� �� '0' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")

    one.T1Y.du_c[BYTE_ONE] = TL0;
    one.T1Y.du_c[BYTE_TWO] = TH0;            // -= ������������ �������� 'Y' =-    *

    cnt = CNT_FRNT;                          // ������������� ������������� ������
    TH0 = 0; TL0 = 0;                        // ����� 'T0'
  
    while(!YOUT);                            // �������� ������ 2-�� �������� � ������ 'Yout'
    TR0 = 1;                                 // ������ 'T0'

    while(YOUT && cnt) cnt--;                // ������������� �� '1' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")
    cnt = CNT_FRNT;                          // ������������� ������������� ������

    while(YOUT);                             // �������� ����� 2-�� �������� � ������ 'Yout'

    while(!YOUT && cnt) cnt--;               // ������������� �� '0' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")
    cnt = CNT_FRNT;                          // ������������� ������������� ������

    while(!YOUT);                            // �������� ������ 3-�� �������� � ������ 'Yout'
    TR0 = 0;                                 // ������� 'T0'

    while(YOUT && cnt) cnt--;                // ������������� �� '1' ������ 'CNT_FRNT' ���?
    if(cnt) return(0);                       // ������� �� ������ ("������")

    one.T2Y.du_c[BYTE_ONE] = TL0;
    one.T2Y.du_c[BYTE_TWO] = TH0;            // -= ������ �������� 'Y' =-          *

    return(1);
  }
#else                                        ///////////////////////////////////////
  unsigned char meas_xy(void)
  {
  // ��������� ������������ � ������� ��������� � ������ 'Xout' -------= X =--------
  
    one.T1X.du_c[BYTE_ONE] = 0xd2;           // 1234
    one.T1X.du_c[BYTE_TWO] = 0x04;           // -= ������������ �������� 'X' =-    *
  
    one.T2X.du_c[BYTE_ONE] = 0x2e;           // 5678
    one.T2X.du_c[BYTE_TWO] = 0x16;           // -= ������ �������� 'X' =-          *
  
  // ��������� ������������ � ������� ��������� � ������ 'Yout' -------= 'Y' =------
  
    one.T1Y.du_c[BYTE_ONE] = 0x3d;           // 8765
    one.T1Y.du_c[BYTE_TWO] = 0x22;            // -= ������������ �������� 'Y' =-    *
  
    one.T2Y.du_c[BYTE_ONE] = 0xe1;           // 4321
    one.T2Y.du_c[BYTE_TWO] = 0x10;            // -= ������ �������� 'Y' =-          *

    return(1);
  }
#endif

void Tx_means(void)
{                                          // �� ������ ��������
  unsigned char d, cc;                     // � ������� � 'UART'

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

void Tx_byte(char outbyte)                 // ����� ����� � 'UART'
{
  SBUF = outbyte;
  while (!TI);
  TI = 0;
}
