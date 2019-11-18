/********************************************************************************/
/*                                                                              */
/*                     Управляющая программа для 'ATMEGA168P'                   */
/*                         Версия 1.0 (29 октября 2009 г)                       */
/*                                                                              */
/********************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>

/*==============================================================================*/
/*                              Аппаратные особенности                          */
/*==============================================================================*/
#undef F_CPU
#define F_CPU 11059200                     // частота резонатора
#define BAUDRATE 9600                      // скорость обмена UART
#define UBBR_UART ((F_CPU / (16L * BAUDRATE)) - 1)

#define NSUM 25                            // коэффициент усреднения

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
struct MEASURE Z;                          // 1-н УСРЕДНЕННЫЙ замер

unsigned long s1, s2, s3, s4;              // 0..0x0ffffffff
unsigned char i;                           // 0..0x0ff              
unsigned int n1, n2, n3, n4;               // номер замера

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
  PORTD = 0b11111111;
  DDRD  = 0b11000010;
  PORTB = 0b00000101;
  DDRB  = 0b00000111;

// настройка Т/С1
  TCCR1B = 0x00;                           // останов Т/С1
  TCNT1H = 0x00;
  TCNT1L = 0x00;                           // сброс Т/С1

  s1=0; n1=0;
  s2=0; n2=0;
  s3=0; n3=0;
  s4=0; n4=0;

  for (i=0; i<25; i++) {Buff[i].T1X.du_i=0; 
                        Buff[i].T2X.du_i=0; 
                        Buff[i].T1Y.du_i=0; 
                        Buff[i].T2Y.du_i=0;}
  for(;;)
  {
    meas_xy();                             // получаем один замер

//  расчет "скользящего среднего":

    s1 -= Buff[n1].T1X.du_i;
    s1 += one.T1X.du_i;
    Buff[n1].T1X.du_i=one.T1X.du_i;
    n1++;
    if (n1==NSUM) n1=0;
    Z.T1X.du_i=s1/NSUM;

    s2 -= Buff[n2].T2X.du_i;
    s2 += one.T2X.du_i;
    Buff[n2].T2X.du_i=one.T2X.du_i;
    n2++;
    if (n2==NSUM) n2=0;
    Z.T2X.du_i=s2/NSUM;

    s3 -= Buff[n3].T1Y.du_i;
    s3 += one.T1Y.du_i;
    Buff[n3].T1Y.du_i=one.T1Y.du_i;
    n3++;
    if (n3==NSUM) n3=0;
    Z.T1Y.du_i=s3/NSUM;

    s4 -= Buff[n4].T2Y.du_i;
    s4 += one.T2Y.du_i;
    Buff[n4].T2Y.du_i=one.T2Y.du_i;
    n4++;
    if (n4==NSUM) n4=0;
    Z.T2Y.du_i=s4/NSUM;      

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
  one.T1X.du_c[0] = TCNT1L;
  one.T1X.du_c[1] = TCNT1H;                // -= ДЛИТЕЛЬНОСТЬ ИМПУЛЬСА 'X' =-    
  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1

  loop_until_bit_is_clear(PIND,XOUT);      // корректировка расчета периода импульсов с выхода 'Xout'
  loop_until_bit_is_set(PIND,XOUT);        // ожидание фронта 2-го импульса с выхода 'Xout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  loop_until_bit_is_clear(PIND,XOUT);      // ожидание спада 2-го импульса с выхода 'Xout'
  loop_until_bit_is_set(PIND,XOUT);        // ожидание фронта 3-го импульса с выхода 'Xout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T2X.du_c[0] = TCNT1L;
  one.T2X.du_c[1] = TCNT1H;                // -= ПЕРИОД ИМПУЛЬСА 'X' =-          

// измерение длительности и периода импульсов с выхода 'Yout' -------= 'Y' =------

  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1
  loop_until_bit_is_clear(PIND,YOUT);      // корректировка расчета длительности импульсов с выхода 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // ожидание фронта 1-го импульса с выхода 'Yout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  loop_until_bit_is_clear(PIND,YOUT);      // ожидание спада 1-го импульса с выхода 'Yout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T1Y.du_c[0] = TCNT1L;
  one.T1Y.du_c[1] = TCNT1H;                // -= ДЛИТЕЛЬНОСТЬ ИМПУЛЬСА 'Y' =-    
  TCNT1H = 0; TCNT1L = 0;                  // сброс Т/С1

  loop_until_bit_is_clear(PIND,YOUT);      // корректировка расчета периода импульсов с выхода 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // ожидание фронта 2-го импульса с выхода 'Yout'
  TCCR1B = 0x02;                           // запуск Т/С1 с тактовой частотой CLK_io/8

  loop_until_bit_is_clear(PIND,YOUT);      // ожидание спада 2-го импульса с выхода 'Yout'
  loop_until_bit_is_set(PIND,YOUT);        // ожидание фронта 3-го импульса с выхода 'Yout'
  TCCR1B = 0x00;                           // останов Т/С1
  one.T2Y.du_c[0] = TCNT1L;
  one.T2Y.du_c[1] = TCNT1H;                // -= ПЕРИОД ИМПУЛЬСА 'Y' =- 
}

void Tx_means(void)
{                                          // из памяти побайтно
  unsigned char d, cc;                     // и выводим в 'USART'

  cc = 0x05a;
  Tx_byte(cc);

  d = Z.T1X.du_c[0]; cc += d; Tx_byte(d);
  d = Z.T1X.du_c[1]; cc += d; Tx_byte(d);
  d = Z.T2X.du_c[0]; cc += d; Tx_byte(d);
  d = Z.T2X.du_c[1]; cc += d; Tx_byte(d);

  d = Z.T1Y.du_c[0]; cc += d; Tx_byte(d);
  d = Z.T1Y.du_c[1]; cc += d; Tx_byte(d);
  d = Z.T2Y.du_c[0]; cc += d; Tx_byte(d);
  d = Z.T2Y.du_c[1]; cc += d; Tx_byte(d);

  Tx_byte(cc);
}

void Tx_byte(char outbyte)                 // вывод байта в 'USART'
{

  loop_until_bit_is_set(UCSR0A,UDRE0);     // ждать очистки буфера передатчика
  UDR0 = outbyte;                          // загрузить младший байт данных                                           // в буфер, начать передачу
}


// http://avr-libc.narod.ru/group__avr__sfr.html
