/********************************************************************************/
/*                                                                              */
/*                       Управляющая программа для 'ADXL202'                    */
/*                          Версия 1.0 (7 июня 2009 г)                          */
/*                                                                              */
/********************************************************************************/
#include "main.h"
/*==============================================================================*/
/*                      Глобальные переменные программы.                        */
/*==============================================================================*/
#define NSUM 2                            // длина очереди усреднения
/*==============================================================================*/
/*                                                                              */
/*                         Главная функция программы.                           */
/*                                                                              */
/*==============================================================================*/
void main(void)
{
  idata struct MEASURE {                   // описание структуры 1-го замера
    unsigned int T1X;
    unsigned int T2X;
    unsigned int T1Y;
    unsigned int T2Y;  };

  idata char*  get_one;                    // указатель на замер
  idata struct MEASURE Buff[NSUM];         // очередь замеров
  idata struct MEASURE one;                // 1-н замер

  unsigned long s;                         // 0..0x0ffffffff
  unsigned char i;                         // 0..0x0ff

  IE = 0;                                  // все прерывания запрещены

// настройка синтезатора
  PLLCON = 0x001;                          // синтезатор рабочей частоты ядра
                                           // Core Clk = XTALL*256 = 32768*256 = 8388608 герц
// настройка сторожевого таймера
  WDWR = 1;                                // разрешение записи в 'WDCON'
  WDCON = 0x072;                           // запуск WDT с временным интервалом 2с

// настройка UART:
  SCON = 0x040;                            // настройка последовательного порта на 
                                           // асинхронный 8-битовый режим
// настройка Т/С0:
  TMOD = 0x001;                            // T/C0 - таймер

// настройка Т/С2:
  RCAP2H = 0x0FF;                          // задание скорости 9600 бод 
  RCAP2L = 0x0E5;                          // (814code_asm)
  TH2    = 0x0FF;       
  TL2    = 0x0E5; 
  T2CON  = 0x030;                          // установка Т/С2 в качестве генератора скорости UART
  TR2    = 1;                              // запуск генератора скорости 'UART'

  for(;;)
  {
    get_one = meas_xy();                   // получаем один замер
                                           // извлекаем его
    i = *(get_one++); one.T1X = i;
    i = *(get_one++); one.T1X += 256*i;
    i = *(get_one++); one.T2X = i;
    i = *(get_one++); one.T2X += 256*i;
    i = *(get_one++); one.T1Y = i;
    i = *(get_one++); one.T1Y += 256*i;
    i = *(get_one++); one.T2Y = i;
    i = *(get_one++); one.T2Y += 256*i;
                                           // сдвигаем очередь и задвигаем новый замер
    for(i=NSUM-1; i>0; i--) Buff[i]=Buff[i-1]; Buff[0]=one;
                                           // считаем среднее из очереди
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1X;
    s=s>>(NSUM/2); one.T1X = (unsigned int)(0x0ffff & s); // T1.X'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2X;
    s=s>>(NSUM/2); one.T2X = (unsigned int)(0x0ffff & s); // T2.X'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1Y;
    s=s>>(NSUM/2); one.T1Y = (unsigned int)(0x0ffff & s); // T1.Y'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2Y;
    s=s>>(NSUM/2); one.T2Y = (unsigned int)(0x0ffff & s); // T2.Y'

    get_one = &one;                        // указываем на усредненный результат

    Tx_means(get_one);                     // выводим его в 'UART'

// сброс сторожевого таймера
    EA   = 0;
    WDWR = 1;
    WDE  = 1;
    EA   = 1;
  }
}

void Tx_means(char* outmeans)              // разворачиваем из
{                                          // памяти побайтно
  unsigned char d, cc, i;                  // и выводим в 'UART'

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

void Tx_byte(char outbyte)                 // вывод байта в 'UART'
{
  SBUF = outbyte;
  while (!TI);
  TI = 0;
}

// Минин :)                                // Это Фсё !!! 3:07 2009.06.07
