/********************************************************************************/
/*                                                                              */
/*                     Управляющая программа для 'ATMEGA168P'                   */
/*                          Версия 2.0 (5 ноября 2009 г)                        */
/*                                                                              */
/********************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>

#define BYTE_ONE 0                         // зависит от
#define BYTE_TWO 1                         // компилятора

#undef F_CPU
#define F_CPU 11059200                     // частота резонатора
#define BAUDRATE 9600                      // скорость обмена UART
#define UBBR_UART ((F_CPU / (16L * BAUDRATE)) - 1)

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

/*==============================================================================*/
/*                              Аппаратные особенности                          */
/*==============================================================================*/

// инициализация порта D

// PINх - регистр входных данных, обеспечивает возможность только чтения 
// PORTx - регистр порта данных, обеспечивает возможность чтения/записи
// DDRx - регистр направления данных, обеспечивает возможность чтения/записи

#define RXD PIND0                          // вывод входа приема данных ответа          IN  (^)
#define TXD PORTD1                         // вывод выхода данных по запросу            OUT (1)
#define TMPR_WR PORTD2                     // вывод запроса данных о температуре        OUT (1)
#define TMPR_RD PIND2                      // вывод входа приема данных о температуре   IN  (^)
#define XOUT PIND3                         // вывод входа приема данных по 'X'          IN  (^)
#define YOUT PIND4                         // вывод входа приема данных по 'Y'          IN  (^)
#define SET_ PIND5                         // вывод входа конфигурации установок        IN  (^)
#define KLINE PORTD6                       // вывод управления разрешения чипа 'K-LINE' OUT (1)
                                           // "разрешение приема = 1"     
#define RS485 PORTD7                       // вывод управления разрешения чипа 'RS485'  OUT (1)
                                           // "разрешение приема = 0"

// инициализация порта B
#define LED PORTB0                         // вывод управления сетодиодным индикатором  OUT (1)
#define DIR PORTB1                         // вывод управления направлением 'RS485'     OUT (0) 
                                           // "разрешение передачи = 1"
#define STR PORTB2                         // вывод управления сторожевым таймером      OUT (1)

/*==============================================================================*/
/*                                Описание типов                                */
/*==============================================================================*/
union u_int                                // для 2-х байтовых данных
  {
   unsigned int  du_i;                     // data_union_int
   unsigned char du_c[2];                  // data_union_char
  };                                       // 256*[0] + [1]

struct MEASURE {                           // описание структуры 1-го замера
  union u_int T1X;
  union u_int T2X;
  union u_int T1Y;
  union u_int T2Y;  };

/*==============================================================================*/
/*                                Описание макросов                             */
/*==============================================================================*/
// Инвертирование бита:
#ifndef cpl
#define cpl(sfr, bit)                 \
  if(bit_is_set(_SFR_BYTE(sfr), bit)) \
  {                                   \
    _SFR_BYTE(sfr) &= ~_BV(bit);      \
  }                                   \
  else                                \
  {                                   \
    _SFR_BYTE(sfr) |=  _BV(bit);      \
  }
#endif

/*==============================================================================*/
/*                                Прототипы                                     */
/*==============================================================================*/
void meas_xy(void);
void Tx_means(void);
void Tx_byte(char outbyte);

/*==============================================================================*/
/*                      Глобальные переменные программы.                        */
/*==============================================================================*/
struct MEASURE Buff[NSUM];                 // очередь замеров
struct MEASURE one;                        // 1-н замер

unsigned long s;                           // 0..0x0ffffffff
unsigned char i;                           // 0..0x0ff
unsigned char get_one;                     // номер замера

/*==============================================================================*/
/*                                                                              */
/*                         Главная функция программы.                           */
/*                                                                              */
/*==============================================================================*/
int main(void)
{

// инициализация USART (9600)         
  UCSR0A = 0x00;                           // запрет на время установки скорости        
  UCSR0B = 0x00;                           // передачи настроек USART          
  UCSR0C = 0x06;                           // размер слова данных 8 бит           
  UBRR0L = (unsigned char)UBBR_UART;       // задание скорости 9600
  UBRR0H = (unsigned char)(UBBR_UART>>8);  // или 2400 бод
  UCSR0B = 0x08;                           // разрешение передачи

// инициализация ИСПОЛЬЗУЕМЫХ портов ввода/вывода
  PORTD = (1<<RS485)|(1<<KLINE)|(1<<SET_)|(1<<YOUT)|(1<<XOUT)|(1<<TMPR_RD)|(1<<TXD)|(1<<RXD);
  DDRD  = (1<<RS485)|(1<<KLINE)|(1<<TXD);
  PORTB = (1<<STR)|(1<<LED);
  DDRB  = (1<<STR)|(1<<DIR)|(1<<LED);

// настройка Т/С1
  TCCR1B = 0x00;                           // останов Т/С1
  TCNT1H = 0x00;
  TCNT1L = 0x00;                           // сброс Т/С1

  get_one = 0;

  for(;;)
  {
    meas_xy();                             // получаем один замер

    //  расчет "скользящего среднего":
                                           // заносим новый замер
    Buff[get_one]=one;
                                           // считаем среднее из "очереди"
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1X.du_i;
    s=s>>KDEL; one.T1X.du_i = (unsigned int)(0x0ffff & s); // T1.X'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2X.du_i;
    s=s>>KDEL; one.T2X.du_i = (unsigned int)(0x0ffff & s); // T2.X'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T1Y.du_i;
    s=s>>KDEL; one.T1Y.du_i = (unsigned int)(0x0ffff & s); // T1.Y'
    s=0; for(i=0; i<NSUM; i++) s += Buff[i].T2Y.du_i;
    s=s>>KDEL; one.T2Y.du_i = (unsigned int)(0x0ffff & s); // T2.Y'
    get_one++; if(get_one == NSUM) get_one = 0;

    Tx_means();                            // выводим "среднее" в 'USART'

    cpl(PORTB, STR);                       // cброс сторожевого таймера
  }
}

void meas_xy(void)
{
// измерение длительности и периода импульсов с выхода 'Xout' -------= X =--------

  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1
  loop_until_bit_is_clear(PIND,XOUT);      // корректировка расчета длительности импульсов с выхода 'Xout'
  loop_until_bit_is_set(PIND,XOUT);        // ожидание фронта 1-го импульса с выхода 'Xout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  loop_until_bit_is_clear(PIND,XOUT);      // ожидание спада 1-го импульса с выхода 'Xout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T1X.du_c[BYTE_ONE] = TCNT1L;
  one.T1X.du_c[BYTE_TWO] = TCNT1H;         // -= ДЛИТЕЛЬНОСТЬ ИМПУЛЬСА 'X' =-    
  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1

  loop_until_bit_is_clear(PIND,XOUT);      // корректировка расчета периода импульсов с выхода 'Xout'
  loop_until_bit_is_set(PIND,XOUT);        // ожидание фронта 2-го импульса с выхода 'Xout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  loop_until_bit_is_clear(PIND,XOUT);      // ожидание спада 2-го импульса с выхода 'Xout'
  loop_until_bit_is_set(PIND,XOUT);        // ожидание фронта 3-го импульса с выхода 'Xout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T2X.du_c[BYTE_ONE] = TCNT1L;
  one.T2X.du_c[BYTE_TWO] = TCNT1H;         // -= ПЕРИОД ИМПУЛЬСА 'X' =-          

// измерение длительности и периода импульсов с выхода 'Yout' -------= 'Y' =------

  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1
  loop_until_bit_is_clear(PIND,YOUT);      // корректировка расчета длительности импульсов с выхода 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // ожидание фронта 1-го импульса с выхода 'Yout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  loop_until_bit_is_clear(PIND,YOUT);      // ожидание спада 1-го импульса с выхода 'Yout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T1Y.du_c[BYTE_ONE] = TCNT1L;
  one.T1Y.du_c[BYTE_TWO] = TCNT1H;         // -= ДЛИТЕЛЬНОСТЬ ИМПУЛЬСА 'Y' =-    
  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1

  loop_until_bit_is_clear(PIND,YOUT);      // корректировка расчета периода импульсов с выхода 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // ожидание фронта 2-го импульса с выхода 'Yout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  loop_until_bit_is_clear(PIND,YOUT);      // ожидание спада 2-го импульса с выхода 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // ожидание фронта 3-го импульса с выхода 'Yout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T2Y.du_c[BYTE_ONE] = TCNT1L;
  one.T2Y.du_c[BYTE_TWO] = TCNT1H;         // -= ПЕРИОД ИМПУЛЬСА 'Y' =-  
}

void Tx_means(void)
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

  Tx_byte(cc);
}

void Tx_byte(char outbyte)                 // вывод байта в 'USART'
{
  loop_until_bit_is_set(UCSR0A,UDRE0);     // ждать очистки буфера передатчика
  UDR0 = outbyte;                          // загрузить младший байт данных                                           // в буфер, начать передачу
}

