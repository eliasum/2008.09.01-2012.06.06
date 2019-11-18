/********************************************************************************/
/*                                                                              */
/*               Управляющая программа для 'ATMEGA168P' + 'KMA200'              */
/*                       Версия 1.0 (01 сентября 2010 г)                        */
/*                                                                              */
/********************************************************************************/
#include "main.h"
#include <math.h>

/*==============================================================================*/
/*                      Глобальные переменные программы.                        */
/*==============================================================================*/
union u_int angle, tick;
union u_sint angle_180, tick_180;
union u_sint temp, tock;

unsigned char const __flash Title[] = "KMA200 - Programmable angle sensor";

volatile unsigned char rr_cnt;              // интервал до перезапуска 'WDT'
volatile unsigned char rr_flg;              // флаг перезапуска 'WDT'
volatile unsigned char count_bod;           // счетчик БОД

unsigned char fl_set;                       // признак установки режима работы

/*==============================================================================*/
/*                                                                              */
/*                         Главная функция программы.                           */
/*                                                                              */
/*==============================================================================*/
int main(void)
{
/*  
  Разряд DDxn регистра DDx определяет направление передачи данных
через контакт ввода/вывода. Если этот разряд установлен в «1», то n-й вывод
порта является выходом, если же сброшен в «0» — входом.
  Разряд PORTxn регистра PORTx выполняет двойную функцию. Если
вывод функционирует как выход (DDxn = «1»), этот разряд определяет состояние 
вывода порта. Если разряд установлен в «1», на выводе устанавливается 
напряжение ВЫСОКОГО уровня. Если разряд сброшен в «0», на выводе 
устанавливается напряжение НИЗКОГО уровня. Если же вывод функционирует как 
вход (DDxn = «0»), разряд PORTxn определяет состояние внутреннего 
подтягивающего резистора для данного вывода. При установке разряда PORTxn в «1»
подтягивающий резистор подключается между выводом микроконтроллера 
и проводом питания.
  Состояние вывода микроконтроллера (независимо от установок разряда DDxn) 
 может быть получено путем чтения разряда PINxn регистра PINx.
*/
  PORTD = (1<<RXD)|(1<<TXD)|(1<<STR)|(1<<SET_)|(1<<KLINE);
  DDRD  = (1<<TXD)|(1<<STR)|(1<<VDD)|(1<<VPRG)|(1<<KLINE)|(1<<RS485);

  PORTB = (1<<LED)|(1<<CS)|(1<<MOSI)|(1<<MISO);
  DDRB  = (1<<LED)|(1<<DIR)|(1<<CS)|(1<<MOSI)|(1<<SCK);
  
  #define VDD_ON       PORTD &= ~(1<<VDD)
  #define VDD_OFF      PORTD |=  (1<<VDD)
  #define VPRG_ON      PORTD |=  (1<<VPRG)
  #define VPRG_OFF     PORTD &= ~(1<<VPRG)
  #define SCK_SET      PORTB |=  (1<<SCK)
  #define SCK_RES      PORTB &= ~(1<<SCK)
  #define CS_SET       PORTB |=  (1<<CS)
  #define CS_RES       PORTB &= ~(1<<CS)
  #define MOSI_TO_IN   DDRB  &= ~(1<<MOSI)
  #define LED_TOGGLE   cpl(PORTB, LED)
  #define RWDT         cpl(PORTD, STR)
  
  // выбор режима работы датчика:
  if (PIND & (1<<SET_)) fl_set = 1; else fl_set = 0; 

  TCCR0B = 0x00;                            // останов Т/С0
  TCNT0  = 0xB8;                            // нач. знач-е
  TCCR0A = 0x00;
  TCCR0B = 0x04;                            // таймер запускаем
  TIMSK0 = 0x01;                            // Т/С0 <- источник прерываний

  rr_flg  = 0;                              // только "простой" сторожевой таймер
  count_bod = 1;                            // минимум 1 прерывание
  
  RWDT;                                     // 1-й "простой" cброс сторожевого таймера
  
  __enable_interrupt();                     // прерывания разрешаем

  // инициализация USART (9600):
  UCSR0B = 0x00;                            // UCSRnB – USART Control and Status Register n B
  UCSR0A = 0x00;                            // UCSRnA – USART Control and Status Register n A
  // ^=> запрет на время установки скорости передачи настроек USART0

  UCSR0C = 0x06;                            // UCSRnC – USART Control and Status Register n C *1)
  // ^=> размер слова данных 8 бит
  UBRR0L = (unsigned char)UBBR_9K6;         // задание скорости 9600; пример в datasheet
  UBRR0H = (unsigned char)(UBBR_9K6>>8);
  UCSR0B = 0x08;                            // разрешение передачи *2)
       
  SPI_MasterInit();                         // инициализация SPI в режиме Master
     
  // вывод строки из ПЗУ, uart_putc(0x0a); - перевод на след. строку  
  uart_puts_p(Title);                                 uart_putc(0x0d); uart_putc(0x0a);                        
  uart_puts("------------------------------------");  uart_putc(0x0d); uart_putc(0x0a);
  
  for (;;)
  {      
    while(count_bod);                       // ждать как
    count_bod = 1;                          // минимум 1 прерывание 
      
    unsigned char byte;
      
    if (!fl_set)                           // выбор режима работы Command mode
    {       
      LED_TOGGLE;                          // светодиод меняет состояние на противоположное
      VDD_OFF;                             // выключение питания "ведомого"
         
      CS_SET;                              // активация режима работы Command mode        
      SCK_RES;
      VDD_ON;
      delay_ms(6);                         // t_init_cmd1 
      SCK_SET;
      delay_ms(5);                         // t_init_cmd2
         
      CS_RES;                              // активация ведомого
      byte = CTRL1;                        // адрес управляющего слова 1 в EEPROM "ведомого"
      SPI_WriteRead(byte);                 // передача
         
      tick.du_i = 0x4006;                  // выбор режима вывода данных - SPI
      byte = tick.du_c[1];
      SPI_WriteRead(byte);                 // запись команды в ОЗУ
      byte = tick.du_c[0];
      SPI_WriteRead(byte);               
         
      byte = CTRL2;                        // адрес управляющего слова 2 в EEPROM "ведомого"
      SPI_WriteRead(byte);                 // передача
   
      tick.du_i = 0x504;                   // отключение автоматического возвращения в Normal operation mode
      byte = tick.du_c[1];
      SPI_WriteRead(byte);                 // запись команды в ОЗУ
      byte = tick.du_c[0];
      SPI_WriteRead(byte); 
                          
      byte = 0x30;                         // команда программирования EEPROM
      SPI_WriteRead(byte);                 // передача
      CS_SET;                              // деактивация ведомого
      delay_ms(1);                         // t_cmd1
      VPRG_ON;                             // включение напряжения питания программирования EEPROM        
      delay_ms(500);                       // t_prog
      VPRG_OFF;                            // выключение напряжения питания программирования EEPROM
      CS_RES;
      delay_ms(1);                         // t_ics
       
      LED_TOGGLE;                          // светодиод меняет состояние на противоположное
      CS_SET;                              // деактивация ведомого
    } 
    else                                   // выбор режима работы Normal operation mode
    {
      MOSI_TO_IN;                          // конфигурирование вывода MOSI как вход
      
      CS_RES;                              // активация ведомого     
      byte = 0x01;                         // байт произволных данных    
      SPI_WriteRead(byte);                 // передача байта произволных данных
     
      uart_puts("Tick/Angle = 0x");        // вывод надписи Tick/Angle = 0x
      
      CS_SET;                              // деактивация ведомого
      LED_TOGGLE;                          // светодиод меняет состояние на противоположное
      delay_ms(300);
    }
    RWDT;                                  // "простой" cброс сторожевого таймера
  }
}

/*==============================================================================*/
/*                                                                              */
/*                                Подпрограммы.                                 */
/*                                                                              */
/*==============================================================================*/
#pragma vector=TIMER0_OVF_vect
__interrupt void timer0_ovf_isr(void)
{
  //TIMER0 has overflowed
  TCNT0 = RELOAD;                          // reload counter value

  if (count_bod)  count_bod--;

  if (rr_flg & 0x0f)
  {
    if (rr_cnt) rr_cnt--;
    else
    {
      rr_flg--;
      rr_cnt = RR_CONST;
      RWDT;                                // "простой" cброс сторожевого таймера
    }
  }
}

// Подпрограмма инициализации SPI в режиме Master
void SPI_MasterInit(void)
{
/* - Разрешение SPI в режиме мастера;
   - CPOL = 1 - генерируются тактовые импульсы отрицательной полярности;
   - CPHA = 1 - обработка данных производится по заднему фронту импульсов сигнала SCK;
   - установка скорости обмена fck/128. */
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPOL)|(1<<CPHA)|(1<<SPR1)|(1<<SPR0);
}

// Подпрограмма записи/чтения через SPI
unsigned char SPI_WriteRead(unsigned char dataout)
{
  // Запись байта в регист данных ведущего (инициализация передачи)
  SPDR = dataout;
  // Ожидание завершения передачи (пока бит SPIF не установлен)
  while(!(SPSR & (1<<SPIF)));
  // Чтение принятых данных и выход из процедуры
  return SPDR;
}
/* *1)
Bit    7         6        5       4       3       2        1        0
    UMSELn1 | UMSELn0 | UPMn1 | UPMn0 | USBSn | UCSZn1 | UCSZn0 | UCPOLn | UCSRnC
       0    |    0    |       |       |       |        |        |        | Asynchronous USART
            |         |   0   |   0   |       |        |        |        | Parity Mode disabled
            |         |       |       |   0   |        |        |        | Stop Bit(s) - 1-bit
            |         |       |       |       |   1    |   1    |        | Character Size - 8-bit
            |         |       |       |       |        |        |   0    | zero when asynchronous mode is used
размер слова данных 8 бит => 0b00000110 = 0x06
   *2)
Bit    7         6        5       4       3       2        1       0
     RXCIEn | TXCIEn | UDRIEn | RXENn | TXENn | UCSZn2 | RXB8n | TXB8n | UCSRnB
       0    |        |        |       |       |        |       |       | RX Complete Interrupt Enable n
            |    0   |        |       |       |        |       |       | TX Complete Interrupt Enable n
            |        |    0   |       |       |        |       |       | USART Data Register Empty Interrupt Enable n
            |        |        |   0   |       |        |       |       | Receiver Enable n
            |        |        |       |   1   |        |       |       | Transmitter Enable n
            |        |        |       |       |   0    |       |       | Character Size - 8-bit
            |        |        |       |       |        |   0   |       | Receive Data Bit 8 n
            |        |        |       |       |        |       |   0   | Transmit Data Bit 8 n
разрешение передачи => 0b00001000 = 0x08
*/