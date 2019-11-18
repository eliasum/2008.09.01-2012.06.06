/********************************************************************************/
/*                                                                              */
/*              Управляющая программа для 'ATMEGA168P' + 'AS5130'               */
/*                      Версия 1.0 (23 января 2011 г)                          */
/*                                                                              */
/********************************************************************************/
#include "main.h"
#include <math.h>
/*==============================================================================*/
/*                      Глобальные переменные программы.                        */
/*==============================================================================*/
union u_int tick;
signed char multiturn, current, previous, memory;
unsigned char angle;

volatile unsigned char count_bod;           // счетчик БОД
volatile unsigned char t_led_on;            // 0.2 сек

unsigned char fl_set;                       // признак установки режима работы
unsigned char const __flash Title[] = "AS5130 - convertion of Ticks to Angle and Multiturn";
/*==============================================================================*/
/*                                                                              */
/*                         Главная функция программы.                           */
/*                                                                              */
/*==============================================================================*/

int main(void)
{
  // задание интервала сброса WDT: 
  __watchdog_reset();                       // WDTCSR = WDIF WDIE WDP3 WDCE WDE WDP2 WDP1 WDP0 (XXXX XXXX)
  WDTCSR |= (1<<WDCE) | (1<<WDE);           // разрешение настройки/системного сброса WDT      (XXX1 1XXX)
  WDTCSR  = (1<<WDE)  | (1<<WDP3);          // установка тайм-аута = 512K циклов (~ 4.0 с)     (0010 1000)

  PORTB = (1<<LED)|(1<<MOSI)|(1<<MISO)|(1<<SCK);
  DDRB  = (1<<LED)|(1<<MOSI)|(1<<SCK);
  
  PORTD = (1<<RXD)|(1<<TXD)|(1<<RE)|(1<<SET_)|(1<<WP);
  DDRD  = (1<<TXD)|(1<<RE)|(1<<DIO)|(1<<CS)|(1<<DCLK)|(1<<WP);
     
  // выбор режима работы датчика:
  if (PIND & (1<<SET_)) fl_set = 1; else fl_set = 0;
  
  count_bod = 1;                            // минимум 1 прерывание
  t_led_on  = 0;                            // время индикации
  
  __watchdog_reset();                       // 1-й "простой" cброс сторожевого таймера

  __enable_interrupt();                     // прерывания разрешаем

//////////////////////// Работа в режиме -= УСТАНОВКИ =- ///////////////////////
  //if (fl_set)
  {
    // инициализация USART (9600):
    UCSR0B = 0x00;                          // запрет настроек USART0 на время установки скорости передачи
    UCSR0A = 0x00;                          
    UCSR0C = 0x06;                          // размер слова данных 8 бит
    UBRR0L = (unsigned char)UBBR_9K6;       // задание скорости 9600; пример в datasheet
    UBRR0H = (unsigned char)(UBBR_9K6>>8);
    UCSR0B = 0x08;                          // разрешение передачи (TXENn = 1)
    
    // инициализация Т/С0:
    TCCR0B = 0x00;                          // останов Т/С0
    TCNT0  = RELOAD_TIM0;                   // нач. знач-е
    TCCR0A = 0x00;
    TCCR0B = START_TIM0;                    // таймер запускаем
    TIMSK0 = 0x01;                          // Т/С0 <- источник прерываний
    
    // вывод строки из ПЗУ, uart_putc(0x0a); - перевод на след. строку  
    uart_puts_p(Title);                                 uart_putc(0x0d); uart_putc(0x0a);                        
    uart_puts("------------------------------------");  uart_putc(0x0d); uart_putc(0x0a);  
     
    if (fl_set) memory = 0;                 // сброс значения КО при начале работы
    
    for (;;)
    {            
      while(count_bod);                     // ждать как
      count_bod = 1;                        // минимум 1 прерывание 

      SET(CS);                              // активация ведомого, инициализация чтения данных 

      tick.du_i = SSI_WriteRead(RD_BOTH);   // 2 байта данных от AS5130
      
      multiturn = tick.du_c[1];             // байт данных от AS5130 --> multiturn <7:0>
      angle = tick.du_c[0];                 // байт данных от AS5130 --> angle <7:0>
            
      previous = current;                   // предыдущее значение количества оборотов (КО) в ОЗУ
      current = multiturn;                  // текущее значение КО в ОЗУ
      
      memory = FM24Cxx_get(FM24C512, 1);    // извлечение значения КО из памяти
      
      if(current != previous)               // ОЗУ изменилось?
      {
        multiturn = memory + current - previous;
      } multiturn = memory;
      
      if(current != memory)                 // текущее значение КО = памяти?
      {
        memory = multiturn;
        FM24Cxx_put(FM24C512, 1, memory);   // занесение значения текущего КО в память
      }
      
      uart_puts("tick/multiturn = 0x");     // вывод надписи tick/multiturn = 0x
      uart_puthex_byte(tick.du_c[1]);       // вывод старших 8 бит данных от AS5130
      uart_puts("/");
      uart_puti(multiturn);                 // вывод значения количества оборотов
      uart_puts("`; ");                     // вывод знаков `; 
      
      uart_putc(0x0d);                      // возврат каретки
      uart_putc(0x0a);                      // перевод на следующую строку

      uart_puts("tick/angle = 0x");         // вывод надписи tick/angle = 0x
      uart_puthex_byte(tick.du_c[0]);       // вывод младших 8 бит данных от AS5130
      uart_puts("/");
      
      angle=(unsigned char)(1.40625*angle); // значение угла целое (360/256=1.40625)
      
      uart_puti(angle);                     // вывод значения угла поворота
      uart_puts("`; ");                     // вывод знаков `; 
      
      uart_putc(0x0d);                      // возврат каретки
      uart_putc(0x0a);                      // перевод на следующую строку
      
      LED_TOGGLE;                           // светодиод меняет состояние на противоположное
      SET(CS);                              // деактивация ведомого, завершение чтения данных

      __watchdog_reset();                   // "простой" cброс сторожевого таймера
      
    } // end of 'for (;;)' 
  } // end of 'if (fl_set)' 
/*  
///////////////////////// Работа в режиме -= ДАТЧИК =- /////////////////////////

  else
  {
    // инициализация USART (2400):
    UCSR0B = 0x00;                          // запрет настроек USART на время установки скорости передачи
    UCSR0A = 0x00;                          
    UCSR0C = 0x06;                          // размер слова данных 8 бит
    UBRR0L = UBBR_2K4L;                     // задание скорости 2400
    UBRR0H = UBBR_2K4H;   
    UCSR0B = 0x0C;                          // разрешение передачи (TXENn = 1); 9-Bit

    // инициализация Т/С0:
    TCCR0B = 0x00;                          // останов Т/С0
    TCNT0  = RELOAD_TIM0;                   // нач. знач-е
    TCCR0A = 0x00;
    TCCR0B = START_TIM0;                    // таймер запускаем
    TIMSK0 = 0x01;                          // Т/С0 <- источник прерываний
       
    __enable_interrupt();                   // прерывания разрешаем   
////////////////////////////////////////////////////////////////////////////////  
          
                    //Организация работы линии обмена K-LINE                    
          
////////////////////////////////////////////////////////////////////////////////      
    for (;;)
    {
      UCSR0B &= ~(1<<RXEN0);                // отключение приемника USART
      count_bod = CBOD;                     // счетчик БОД

      do
      {
        if (!(PIND & (1<<RXD))) count_bod = CBOD;
      } while (count_bod);                  // ожидание СИНХРО-ПАУЗЫ (всегда только перед командой)

      UCSR0B |= (1<<RXEN0);                 // включение приемника USART

      while (!(UCSR0A & (1<<RXC0)));        // ждем завершение приема байта 

      if (UCSR0B & (1<<RXB80))              // если 9-й бит =1, то принята команда
      {
        unsigned char var;
        var  = UDR0;                        // UDRn - USART I/O Data Register n
        var &= 0x0f;                        // адрес датчика д/б не более 15
        if (var == NDEV)                    // обратились к этому датчику?
        {    
                 
          __watchdog_reset();               // cброс сторожевого таймера
          
        } // end of 'if (var == NDEV)' 
      } // end of 'if (UCSR0B & (1<<RXB80))' 
    } // end of 'for (;;)' 
  } // end of 'else' 
*/
} // end of 'main' 
/*==============================================================================*/
/*                                                                              */
/*                                Подпрограммы.                                 */
/*                                                                              */
/*==============================================================================*/

#pragma vector=TIMER0_OVF_vect
__interrupt void timer0_ovf_isr(void)
{
  //TIMER0 has overflowed
  TCNT0  = RELOAD_TIM0;                     // reload counter value

  if (count_bod)  count_bod--;

  if(t_led_on)  t_led_on--;                 // время подсвечивания --
  else          LED_OFF;                    // LED-OFF
}

unsigned int SSI_WriteRead(unsigned int dataout) 
{
  unsigned char bit;
  unsigned int datain = 0;                  // принимаемые данные
    
  for(bit=0; bit<8; bit++)                  // запись команды
  {
    SET(DCLK);                              // передний фронт положительного импульса синхронизации
    
    if(dataout&0x80) SET(DIO);              // если старший бит команды = 1 --> на выводе DIO единица
    else             RES(DIO);              // иначе --> на выводе DIO нуль

    dataout<<=1;                            // сдвиг передаваемой команды на 1 бит
    RES(DCLK);                              // задний фронт положительного импульса синхронизации
  }  
  
  for(bit=0; bit<16; bit++)                 // чтение ответа (2-х байтное слово)
  {
    SET(DCLK);                              // передний фронт положительного импульса синхронизации
    datain<<=1;                             // сдвиг принимаемого слова на 1 бит
    
    if(PIND&(1<<DIO))                       // если на выводе DIO единица  --> младший бит считанного слова = 1 
      {
        datain+=0x01;
      }                                     // иначе --> младший бит считанного байта = 0                               
    RES(DCLK);                              // задний фронт положительного импульса синхронизации
  } 
  return datain;
}