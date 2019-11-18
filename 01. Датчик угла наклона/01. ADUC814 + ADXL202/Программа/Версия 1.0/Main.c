/********************************************************************************/
/*                                                                              */
/*                       ����������� ��������� ��� 'ADXL202'                    */
/*                          ������ 1.0 (7 ���� 2009 �)                          */
/*                                                                              */
/********************************************************************************/
#include "main.h"
/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
#define NSUM 2                            // ����� ������� ����������
/*==============================================================================*/
/*                                                                              */
/*                         ������� ������� ���������.                           */
/*                                                                              */
/*==============================================================================*/
void main(void)
{
  idata struct MEASURE {                   // �������� ��������� 1-�� ������
    unsigned int T1X;
    unsigned int T2X;
    unsigned int T1Y;
    unsigned int T2Y;  };

  idata char*  get_one;                    // ��������� �� �����
  idata struct MEASURE Buff[NSUM];         // ������� �������
  idata struct MEASURE one;                // 1-� �����

  unsigned long s;                         // 0..0x0ffffffff
  unsigned char i;                         // 0..0x0ff

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
    get_one = meas_xy();                   // �������� ���� �����
                                           // ��������� ���
    i = *(get_one++); one.T1X = i;
    i = *(get_one++); one.T1X += 256*i;
    i = *(get_one++); one.T2X = i;
    i = *(get_one++); one.T2X += 256*i;
    i = *(get_one++); one.T1Y = i;
    i = *(get_one++); one.T1Y += 256*i;
    i = *(get_one++); one.T2Y = i;
    i = *(get_one++); one.T2Y += 256*i;
                                           // �������� ������� � ��������� ����� �����
    for(i=NSUM-1; i>0; i--) Buff[i]=Buff[i-1]; Buff[0]=one;
                                           // ������� ������� �� �������
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1X;
    s=s>>(NSUM/2); one.T1X = (unsigned int)(0x0ffff & s); // T1.X'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2X;
    s=s>>(NSUM/2); one.T2X = (unsigned int)(0x0ffff & s); // T2.X'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1Y;
    s=s>>(NSUM/2); one.T1Y = (unsigned int)(0x0ffff & s); // T1.Y'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2Y;
    s=s>>(NSUM/2); one.T2Y = (unsigned int)(0x0ffff & s); // T2.Y'

    get_one = &one;                        // ��������� �� ����������� ���������

    Tx_means(get_one);                     // ������� ��� � 'UART'

// ����� ����������� �������
    EA   = 0;
    WDWR = 1;
    WDE  = 1;
    EA   = 1;
  }
}

void Tx_means(char* outmeans)              // ������������� ��
{                                          // ������ ��������
  unsigned char d, cc, i;                  // � ������� � 'UART'

  d = 0; cc = 0x05a;
  Tx_byte(cc);
  for(i=0; i<8; i+=2)
  {
    d = *(outmeans+i+1);
    cc += d; Tx_byte(d);
    d = *(outmeans+i);
    cc += d; Tx_byte(d);
  }
  Tx_byte(cc);
}

void Tx_byte(char outbyte)                 // ����� ����� � 'UART'
{
  SBUF = outbyte;
  while (!TI);
  TI = 0;
}

// ����� :)                                // ��� �� !!! 3:07 2009.06.07
