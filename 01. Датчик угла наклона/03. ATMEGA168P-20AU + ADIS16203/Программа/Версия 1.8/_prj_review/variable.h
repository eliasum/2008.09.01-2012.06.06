#ifndef _variable_h_
#define _variable_h_

#include "main.h"
/*==============================================================================*/
/*                      Глобальные переменные программы.                        */
/*==============================================================================*/
unsigned char var;

// длина "очереди усреднения" ( 1, 2, 4, 8 или 16 )
#define NSUM 16

// сопоставляем соответствующий коэф.деления
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

unsigned int Buff[NSUM];                    // очередь замеров
unsigned char N_zamera;                     // номер замера
unsigned long sum;                          // 0..0x0ffffffff
unsigned char i;                            // 0..0x0ff

union u_int angle, tick;
union u_int avg;
union u_int smpl;
union u_sint angle_180, tick_180;
union u_sint temp, tock;

volatile unsigned char count_bod;           // счетчик БОД
volatile unsigned char t_led_on;            // 0.2 сек

unsigned char fl_set;                       // признак установки режима работы
unsigned char const __flash Title[] = "ADIS16203 - convertion of Ticks to Angle";

#endif // _variable_h_

