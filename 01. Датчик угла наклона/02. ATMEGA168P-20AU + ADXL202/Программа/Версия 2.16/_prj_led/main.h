#ifndef _main_h_
#define _main_h_

#include "common.h"

#include "onewire.h"
#include "ds18x20.h"

#define BYTE_ONE 0                         // зависит от
#define BYTE_TWO 1                         // компилятора

#define BR19K2 19200                       // скорость обмена 19200
#define BR9K6 9600                         // скорость обмена 9600
#define BR2K4 2400                         // скорость обмена 2400
#define UBBR_19K2 ((F_CPU / (16L * BR19K2)) - 1)
#define UBBR_9K6 ((F_CPU / (16L * BR9K6)) - 1)
#define UBBR_2K4 ((F_CPU / (16L * BR2K4)) - 1)

// частота системного тика = 600 гц:
#define CONST_RELOAD 0xB8                  // константа перезагрузки таймера '0'
#define CBOD 4                             // константа БОД
#define SYSTEM_TICK 600                    // Тик системного таймера в Гц
#define NDEV 5                             // базовый адрес устройства
#define RR_CONST 150                       // 0.25 сек до перезапуска 'WDT'

#define TO_CEL_SET (SYSTEM_TICK*3)        // изм-е темп-ры 1 раз за 10 сек

// длина "очереди усреднения" ( 1, 2, 4, 8 или 16 )
#define NSUM 2

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

/*==============================================================================*/
/*                              Аппаратные особенности                          */
/*==============================================================================*/

// инициализация порта D

// PINх  - регистр входных данных, обеспечивает возможность только чтения 
// PORTx - регистр порта данных, обеспечивает возможность чтения/записи
// DDRx  - регистр направления данных, обеспечивает возможность чтения/записи

#define RXD PIND0                          // вывод входа приема данных ответа          IN  (^)
#define TXD PORTD1                         // вывод выхода данных по запросу            OUT (1)
#define XOUT PIND3                         // вывод входа приема данных по 'X'          IN  (^)
#define YOUT PIND4                         // вывод входа приема данных по 'Y'          IN  (^)
#define SET_ PIND5                         // вывод входа конфигурации установок        IN  (^)
#define KLINE PORTD6                       // вывод управления разрешения чипа 'K-LINE' OUT (1)
                                           // "разрешение приема = 1"     
#define RS485 PORTD7                       // вывод управления разрешения чипа 'RS485'  OUT (0)
                                           // "разрешение приема = 0"
// инициализация порта B
#define LED PORTB0                         // вывод управления сетодиодным индикатором  OUT (1)
#define DIR PORTB1                         // вывод управления направлением 'RS485'     OUT (0) 
                                           // "разрешение передачи = 1"
#define STR PORTB2                         // вывод управления сторожевым таймером      OUT (1)
/*
  обозначения:
  - IN  (^) - вывод входа(чтение), ^ - резистивная подтяжка;
  - OUT (0) - вывод выхода(запись) начальное состояние 0, активное состояние 1;
  - OUT (1) - вывод выхода(запись) начальное состояние 1, активное состояние 0;
  Начальное и активное состояние выводов OUT определяется по принцип. схеме.
*/
// инициализация порта 'One Wire'
#define ONEWIRE_PIN  PORTD2
#define ONEWIRE_IN   PIND
#define ONEWIRE_OUT  PORTD
#define ONEWIRE_DDR  DDRD

/*==============================================================================*/
/*                                Описание типов                                */
/*==============================================================================*/
union u_int                                // для 2-х байтовых данных
  {
   unsigned int  du_i;                     // data_union_int
   unsigned char du_c[2];                  // data_union_char
  };                                       // 256*[0] + [1]

union u_sig_int
  {
  signed int    du_i;                       // data_union_int
  unsigned char du_c[2];                    // data_union_char
  };

struct MEASURE {                           // описание структуры 1-го замера
  union u_int T1X;
  union u_int T2X;
  union u_int T1Y;
  union u_int T2Y;  };

/*==============================================================================*/
/*                                Прототипы                                     */
/*==============================================================================*/
void Get_Buff(void);
void Meas_Cel(void);
void meas_xy(void);
void Tx_means_tick(void);
void Tx_byte(char outbyte);

#endif
