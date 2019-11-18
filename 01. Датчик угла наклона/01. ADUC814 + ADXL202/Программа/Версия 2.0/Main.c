/********************************************************************************/
/*                                                                              */
/*                       ”правл€юща€ программа дл€ 'ADXL202'                    */
/*                          ¬ерси€ 2.0 (15 июн€ 2009 г)                         */
/*                                                                              */
/********************************************************************************/
#include "main.h"
// дл€ симул€ции в отладчике убрать "последий" '_'
#define SIMUL_DBG_

// длина "очереди усреднени€" ( 1, 2, 4, 8 или 16 )
#define NSUM 16

// число подтвеждений фронта ( < 256)
#define CNT_FRNT 256

// сопоставл€ем соответствующий коэф.делени€
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
/*                                ќписание типов                                */
/*==============================================================================*/
union u_int                                 // дл€ 2-х байтовых данных
  {
   unsigned int  du_i;                      // data_union_int
   unsigned char du_c[2];                   // data_union_char
  };                                        // 256*[0] + [1]

idata struct MEASURE {                      // описание структуры 1-го замера
  union u_int T1X;
  union u_int T2X;
  union u_int T1Y;
  union u_int T2Y;  };
/*==============================================================================*/
/*                      √лобальные переменные программы.                        */
/*==============================================================================*/
idata struct MEASURE Buff[NSUM];           // очередь замеров
data struct MEASURE one;                   // 1-н замер

unsigned long s;                           // 0..0x0ffffffff
unsigned char i;                           // 0..0x0ff
unsigned char get_one;                     // номер замера

/*==============================================================================*/
/*                                                                              */
/*                         √лавна€ функци€ программы.                           */
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

  IE = 0;                                  // все прерывани€ запрещены

// настройка синтезатора
  PLLCON = 0x001;                          // синтезатор рабочей частоты €дра
                                           // Core Clk = XTALL*256 = 32768*256 = 8388608 герц
// настройка сторожевого таймера
  WDWR = 1;                                // разрешение записи в 'WDCON'
  WDCON = 0x072;                           // запуск WDT с временным интервалом 2с

// настройка UART:
  SCON = 0x040;                            // настройка последовательного порта на 
                                           // асинхронный 8-битовый режим
// настройка “/—0:
  TMOD = 0x001;                            // T/C0 - таймер

// настройка “/—2:
  RCAP2H = 0x0FF;                          // задание скорости 9600 бод 
  RCAP2L = 0x0E5;                          // (814code_asm)
  TH2    = 0x0FF;       
  TL2    = 0x0E5; 
  T2CON  = 0x030;                          // установка “/—2 в качестве генератора скорости UART
  TR2    = 1;                              // запуск генератора скорости 'UART'

  for(;;)
  {
    if(meas_xy())                          // получаем один замер
    {
      //  расчет "скольз€щего среднего":
                                           // сдвигаем "очередь" и задвигаем новый замер
      for(i=NSUM-1; i>0; i--) Buff[i]=Buff[i-1]; Buff[0]=one;
                                           // считаем среднее из "очереди"
      s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1X.du_i;
      s=s>>KDEL; one.T1X.du_i = (unsigned int)(0x0ffff & s); // T1.X'
      s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2X.du_i;
      s=s>>KDEL; one.T2X.du_i = (unsigned int)(0x0ffff & s); // T2.X'
      s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1Y.du_i;
      s=s>>KDEL; one.T1Y.du_i = (unsigned int)(0x0ffff & s); // T1.Y'
      s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2Y.du_i;
      s=s>>KDEL; one.T2Y.du_i = (unsigned int)(0x0ffff & s); // T2.Y'

      Tx_means();                          // выводим "среднее" в 'UART'

      // сброс сторожевого таймера
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

    TR0 = 0;                                 // останов 'T0'

  // измерение длительности и периода импульсов с выхода 'Xout' -------= X =--------
  
    cnt = CNT_FRNT;                          // инициализаци€ подтвержнени€ фронта
    TH0 = 0; TL0 = 0;                        // сброс 'T0'
    while(XOUT);                             // сначапа должен быть '0'

    while(!XOUT && cnt) cnt--;               // действительно ли '0' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")
    cnt = CNT_FRNT;                          // инициализаци€ подтвержнени€ фронта

    while(!XOUT);                            // ожидание фронта 1-го импульса с выхода 'Xout'
    TR0 = 1;                                 // запуск 'T0'

    while(XOUT && cnt) cnt--;                // действительно ли '1' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")
    cnt = CNT_FRNT;                          // инициализаци€ подтвержнени€ фронта

    while(XOUT);                             // ожидание спада 1-го импульса с выхода 'Xout'
    TR0 = 0;                                 // останов 'T0'

    while(!XOUT && cnt) cnt--;               // действительно ли '0' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")

    one.T1X.du_c[BYTE_ONE] = TL0;
    one.T1X.du_c[BYTE_TWO] = TH0;                   // -= ƒЋ»“≈Ћ№Ќќ—“№ »ћѕ”Ћ№—ј 'X' =-    *

    cnt = CNT_FRNT;                          // инициализаци€ подтвержнени€ фронта
    TH0 = 0; TL0 = 0;                        // сброс 'T0'
  
    while(!XOUT);                            // ожидание фронта 2-го импульса с выхода 'Xout'
    TR0 = 1;                                 // запуск 'T0'

    while(XOUT && cnt) cnt--;                // действительно ли '1' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")
    cnt = CNT_FRNT;                          // иинициализаци€ подтвержнени€ фронта

    while(XOUT);                             // ожидание спада 2-го импульса с выхода 'Xout'

    while(!XOUT && cnt) cnt--;               // действительно ли '0' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")
    cnt = CNT_FRNT;                          // иинициализаци€ подтвержнени€ фронта

    while(!XOUT);                            // ожидание фронта 3-го импульса с выхода 'Xout'
    TR0 = 0;                                 // останов 'T0'

    while(XOUT && cnt) cnt--;                // действительно ли '1' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")

    one.T2X.du_c[BYTE_ONE] = TL0;
    one.T2X.du_c[BYTE_TWO] = TH0;            // -= ѕ≈–»ќƒ »ћѕ”Ћ№—ј 'X' =-          *
  
  // измерение длительности и периода импульсов с выхода 'Yout' -------= 'Y' =------
  
    cnt = CNT_FRNT;                          // инициализаци€ подтвержнени€ фронта
    TH0 = 0; TL0 = 0;                        // сброс 'T0'
    while(YOUT);                             // сначапа должен быть '0'

    while(!YOUT && cnt) cnt--;               // действительно ли '0' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")
    cnt = CNT_FRNT;                          // инициализаци€ подтвержнени€ фронта

    while(!YOUT);                            // ожидание фронта 1-го импульса с выхода 'Yout'
    TR0 = 1;                                 // запуск 'T0'

    while(YOUT && cnt) cnt--;                // действительно ли '1' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")
    cnt = CNT_FRNT;                          // инициализаци€ подтвержнени€ фронта

    while(YOUT);                             // ожидание спада 1-го импульса с выхода 'Yout'
    TR0 = 0;                                 // останов 'T0'

    while(!YOUT && cnt) cnt--;               // действительно ли '0' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")

    one.T1Y.du_c[BYTE_ONE] = TL0;
    one.T1Y.du_c[BYTE_TWO] = TH0;            // -= ƒЋ»“≈Ћ№Ќќ—“№ »ћѕ”Ћ№—ј 'Y' =-    *

    cnt = CNT_FRNT;                          // инициализаци€ подтвержнени€ фронта
    TH0 = 0; TL0 = 0;                        // сброс 'T0'
  
    while(!YOUT);                            // ожидание фронта 2-го импульса с выхода 'Yout'
    TR0 = 1;                                 // запуск 'T0'

    while(YOUT && cnt) cnt--;                // действительно ли '1' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")
    cnt = CNT_FRNT;                          // инициализаци€ подтвержнени€ фронта

    while(YOUT);                             // ожидание спада 2-го импульса с выхода 'Yout'

    while(!YOUT && cnt) cnt--;               // действительно ли '0' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")
    cnt = CNT_FRNT;                          // инициализаци€ подтвержнени€ фронта

    while(!YOUT);                            // ожидание фронта 3-го импульса с выхода 'Yout'
    TR0 = 0;                                 // останов 'T0'

    while(YOUT && cnt) cnt--;                // действительно ли '1' подр€д 'CNT_FRNT' раз?
    if(cnt) return(0);                       // возврат по ошибке ("помеха")

    one.T2Y.du_c[BYTE_ONE] = TL0;
    one.T2Y.du_c[BYTE_TWO] = TH0;            // -= ѕ≈–»ќƒ »ћѕ”Ћ№—ј 'Y' =-          *

    return(1);
  }
#else                                        ///////////////////////////////////////
  unsigned char meas_xy(void)
  {
  // измерение длительности и периода импульсов с выхода 'Xout' -------= X =--------
  
    one.T1X.du_c[BYTE_ONE] = 0xd2;           // 1234
    one.T1X.du_c[BYTE_TWO] = 0x04;           // -= ƒЋ»“≈Ћ№Ќќ—“№ »ћѕ”Ћ№—ј 'X' =-    *
  
    one.T2X.du_c[BYTE_ONE] = 0x2e;           // 5678
    one.T2X.du_c[BYTE_TWO] = 0x16;           // -= ѕ≈–»ќƒ »ћѕ”Ћ№—ј 'X' =-          *
  
  // измерение длительности и периода импульсов с выхода 'Yout' -------= 'Y' =------
  
    one.T1Y.du_c[BYTE_ONE] = 0x3d;           // 8765
    one.T1Y.du_c[BYTE_TWO] = 0x22;            // -= ƒЋ»“≈Ћ№Ќќ—“№ »ћѕ”Ћ№—ј 'Y' =-    *
  
    one.T2Y.du_c[BYTE_ONE] = 0xe1;           // 4321
    one.T2Y.du_c[BYTE_TWO] = 0x10;            // -= ѕ≈–»ќƒ »ћѕ”Ћ№—ј 'Y' =-          *

    return(1);
  }
#endif

void Tx_means(void)
{                                          // из пам€ти побайтно
  unsigned char d, cc;                     // и выводим в 'UART'

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

void Tx_byte(char outbyte)                 // вывод байта в 'UART'
{
  SBUF = outbyte;
  while (!TI);
  TI = 0;
}
