/********************************************************************************/
/*                                                                              */
/*                     Управляющая программа для 'ATMEGA168P'                   */
/*                         Версия 2.5 (7 декабря 2009 г)                        */
/*                                                                              */
/********************************************************************************/
#include "main.h"
#include <math.h>

#define PLATA_1 // 1,2 или 3 !!!
/*==============================================================================*/
/*                      Глобальные переменные программы.                        */
/*==============================================================================*/
struct MEASURE Buff[NSUM];                  // очередь замеров
struct MEASURE one;                         // 1-н замер -= XY =-
// замер температуры:
unsigned char Ttick;                        // градусы по Цельсию "целые"

float Teta_xy, Teta_yx;
int   iAx, iAy, iaAx, iaAy;

unsigned long s;                            // 0..0x0ffffffff
unsigned char i;                            // 0..0x0ff
unsigned char get_one;                      // номер замера

volatile unsigned char rr_cnt;              // интервал до перезапуска 'WDT'
volatile unsigned char rr_flg;              // флаг перезапуска 'WDT'
volatile unsigned char count_bod;           // счетчик БОД
unsigned char fl_set;                       // признак "УСТАНОВКИ"
unsigned char var;                          // врем. хран-е данных

void Const_ini(void)                        // 1-н раз рассчитываем константы
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
/*                         Главная функция программы.                           */
/*                                                                              */
/*==============================================================================*/
int main(void)
{

// инициализация ИСПОЛЬЗУЕМЫХ портов ввода/вывода
  PORTD = (1<<KLINE)|(1<<SET_)|(1<<YOUT)|(1<<XOUT)|(1<<TXD)|(1<<RXD);
  DDRD  = (1<<RS485)|(1<<KLINE)|(1<<TXD);
  PORTB = (1<<STR)|(1<<LED);
  DDRB  = (1<<STR)|(1<<DIR)|(1<<LED);

  if (PIND & (1<<SET_)) fl_set = 1; else fl_set = 0;

  get_one = 0;                              // начнем ... с "начала"

//////////// Работа в режиме -= УСТАНОВКИ =- /////////////////////////////////
  if (fl_set)
  {
    // инициализация USART (9600):
    UCSR0A = 0x00;                          // запрет на время установки скорости
    UCSR0B = 0x00;                          // передачи настроек USART
    UCSR0C = 0x06;                          // размер слова данных 8 бит
    UBRR0L = (unsigned char)UBBR_9K6;       // задание скорости 9600
    UBRR0H = (unsigned char)(UBBR_9K6>>8);
    UCSR0B = 0x08;                          // разрешение передачи

    // инициализация OpenWire:
    ow_set_bus(&ONEWIRE_IN,&ONEWIRE_OUT,&ONEWIRE_DDR,ONEWIRE_PIN);
    ow_reset();

    // настройка Т/С1:
    TCCR1B = 0x00;                          // останов Т/С1
    TCNT1H = 0x00;
    TCNT1L = 0x00;                          // сброс Т/С1

    for (;;)
    {
      Get_Buff();                           // данные -= XY =-
      Meas_Cel();
      Tx_means_tick();                      // выводим "средние" тики в 'USART'
      cpl(PORTB, STR);                      // cброс сторожевого таймера
    }
  }
//////////// Работа в режиме -= ДАТЧИК =- ////////////////////////////////////
  else
  {
    // инициализация USART (2400)
    UCSR0A = 0x00;                          // запрет на время установки скорости
    UCSR0B = 0x00;                          // передачи настроек USART
    UCSR0C = 0x06;                          // размер слова данных 8 бит
    UBRR0L = (unsigned char)UBBR_2K4;       // задание скорости 2400
    UBRR0H = (unsigned char)(UBBR_2K4>>8);
    UCSR0B = 0x08;                          // разрешение передачи

    // TIMER0 initialize - prescale:256
    // WGM:           Normal
    // desired value: 600Hz
    // actual value:  600.000Hz (0.0%)
    TCCR0B = 0x00;                          // останов Т/С0
    TCNT0  = 0xB8;                          // нач. знач-е
    TCCR0A = 0x00;
    TCCR0B = 0x04;                          // таймер запускаем

    TIMSK0 = 0x01;                          // Т/С0 <- источник прерываний

    rr_cnt = RR_CONST;
    rr_flg = 0x0f;                          // + 16*0.25=4 сек
    cpl(PORTB, STR);                        // cброс сторожевого таймера

    SMCR |= (1<<SE);                        // SLEEP enable

    Const_ini();                            // начальная инициализация
    for(i=0; i<NSUM; i++)                   // измерений датчика
    {
      meas_xy();
      Buff[i]=one;
    }
    cpl(PORTB, STR);                        // cброс сторожевого таймера
       
    __enable_interrupt();                   // прерывания разрешаем

    for (;;)
    {
      UCSR0B &= ~(1<<RXEN0);                // прием по 'RxD' 'OFF'
      count_bod = CBOD;                     // счетчик БОД
      do {
        if (!(PIND & (1<<RXD))) count_bod = CBOD;
      } while (count_bod);                  // ожидание СИНХРО-ПАУЗЫ

      UCSR0B |= (1<<RXEN0);                 // прием по 'RxD' 'ON'
      while (UCSR0A & (1<<RXC0))
        var = UDR0;                         // "чистим" приемный буфер

      while (!(UCSR0A & (1<<RXC0)));        // "ждем" байт

      if (UCSR0B & (1<<RXB80))
      {
        var  = UDR0;
        var &= 0x0f;
        if (var == NDEV)                    // обратились к этому датчику?
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
          else     // 'Z2' или 'Z3'?
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
        Get_Buff();                         // в люб. случае обновляем замер
        cpl(PORTB, STR);                    // cброс сторожевого таймера
        rr_flg = 0x0f;                      // + 16*0.25=4 сек
      }
    }
  }
//////////////////////////////////////////////////////////////////////////////
}

// загрузка 1-го измерения в буфер и расчет среднего
void Get_Buff(void)
{
  meas_xy();                               // получаем один замер

  // расчет "скользящего среднего":
  // заносим новый замер
  Buff[get_one]=one;
  // считаем среднее из "очереди"
  s=0; for (i=0; i<NSUM; i++) s += Buff[i].T1X.du_i;
  s=s>>KDEL; one.T1X.du_i = (unsigned int)(0x0ffff & s); // T1.X'
  s=0; for (i=0; i<NSUM; i++) s += Buff[i].T2X.du_i;
  s=s>>KDEL; one.T2X.du_i = (unsigned int)(0x0ffff & s); // T2.X'
  s=0; for (i=0; i<NSUM; i++) s += Buff[i].T1Y.du_i;
  s=s>>KDEL; one.T1Y.du_i = (unsigned int)(0x0ffff & s); // T1.Y'
  s=0; for (i=0; i<NSUM; i++) s += Buff[i].T2Y.du_i;
  s=s>>KDEL; one.T2Y.du_i = (unsigned int)(0x0ffff & s); // T2.Y'
  get_one++; if (get_one == NSUM) get_one = 0;

  __enable_interrupt();                    // прерывания разрешаем
}

void meas_xy(void)
{
// измерение длительности и периода импульсов с выхода 'Xout' -------= X =--------

  __disable_interrupt();                   //disable all interrupts

  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1
  while (PIND & (1<<XOUT));                // корректировка расчета длительности импульсов с выхода 'Xout'
  while (!(PIND & (1<<XOUT)));             // ожидание фронта 1-го импульса с выхода 'Xout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  while (PIND & (1<<XOUT));                // ожидание спада 1-го импульса с выхода 'Xout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T1X.du_c[BYTE_ONE] = TCNT1L;
  one.T1X.du_c[BYTE_TWO] = TCNT1H;         // -= ДЛИТЕЛЬНОСТЬ ИМПУЛЬСА 'X' =-
  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1

  while (PIND & (1<<XOUT));                // корректировка расчета периода импульсов с выхода 'Xout'
  while (!(PIND & (1<<XOUT)));             // ожидание фронта 2-го импульса с выхода 'Xout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  while (PIND & (1<<XOUT));                // ожидание спада 2-го импульса с выхода 'Xout'
  while (!(PIND & (1<<XOUT)));             // ожидание фронта 3-го импульса с выхода 'Xout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T2X.du_c[BYTE_ONE] = TCNT1L;
  one.T2X.du_c[BYTE_TWO] = TCNT1H;         // -= ПЕРИОД ИМПУЛЬСА 'X' =-

// измерение длительности и периода импульсов с выхода 'Yout' -------= 'Y' =------

  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1
  while (PIND & (1<<YOUT));                // корректировка расчета длительности импульсов с выхода 'Yout'
  while (!(PIND & (1<<YOUT)));             // ожидание фронта 1-го импульса с выхода 'Yout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  while (PIND & (1<<YOUT));                // ожидание спада 1-го импульса с выхода 'Yout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T1Y.du_c[BYTE_ONE] = TCNT1L;
  one.T1Y.du_c[BYTE_TWO] = TCNT1H;         // -= ДЛИТЕЛЬНОСТЬ ИМПУЛЬСА 'Y' =-
  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1

  while (PIND & (1<<YOUT));                // корректировка расчета периода импульсов с выхода 'Yout'
  while (!(PIND & (1<<YOUT)));             // ожидание фронта 2-го импульса с выхода 'Yout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  while (PIND & (1<<YOUT));                // ожидание спада 2-го импульса с выхода 'Yout'
  while (!(PIND & (1<<YOUT)));             // ожидание фронта 3-го импульса с выхода 'Yout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T2Y.du_c[BYTE_ONE] = TCNT1L;
  one.T2Y.du_c[BYTE_TWO] = TCNT1H;         // -= ПЕРИОД ИМПУЛЬСА 'Y' =-
}

void Meas_Cel(void)
{
  DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL );
  delay_ms(DS18B20_TCONV_10BIT);
  DS18X20_read_meas_in_tick(&Ttick);
}

void Tx_means_tick(void)
{                                          // из памяти побайтно
  unsigned char d, cc;                     // и выводим в 'USART'

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

void Tx_byte(char outbyte)                 // вывод байта в 'USART'
{
  PORTB  |= (1<<DIR);                      // RS485 -> направление ПЕРЕДАЧА
  UCSR0B &= ~(1<<RXEN0);                   // подавление эха       ON
  UCSR0A |=  (1<<TXC0);
  UDR0 = outbyte;                          // передать байт данных
  while (!(UCSR0A & (1<<TXC0)));           // ждать конца передачи байта
  PORTB &= ~(1<<DIR);                      // RS485 -> направление ПРИЕМ
  UCSR0B |=  (1<<RXEN0);                   // подавление эха       OFF
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

