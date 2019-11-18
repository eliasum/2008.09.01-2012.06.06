#ifndef _variable_h_
#define _variable_h_

#include "main.h"
/*==============================================================================*/
/*                      ���������� ���������� ���������.                        */
/*==============================================================================*/
// ����� "������� ����������" ( 1, 2, 4, 8 ��� 16 )
#define NSUM 8

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

unsigned int Buff[NSUM];                    // ������� �������
unsigned char N_zamera;                     // ����� ������
unsigned long sum;                          // 0..0x0ffffffff
unsigned char i;                            // 0..0x0ff

union u_sint angle_x, tick_x;
union u_sint angle_y, tick_y;
union u_sint temp, tock;
union u_int avg;
union u_int smpl;

volatile unsigned char count_bod;           // ������� ���
volatile unsigned char t_led_on;            // 0.2 ���

unsigned char fl_set;                       // ������� ��������� ������ ������
unsigned char const __flash Title[] = "ADIS16209 - convertion of Ticks to Angle";
#endif // _variable_h_

