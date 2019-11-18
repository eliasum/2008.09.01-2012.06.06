#ifndef _main_h_
#define _main_h_

#include "common.h"
#include "terminal.h"

#define BR9K6        9600                  // скорость обмена 9600
#define UBBR_9K6     ((F_CPU/(16L*BR9K6))-1)

#define CBR2K4       (unsigned int) ((F_CPU/(16L*2400))-1)
#define UBBR_2K4L    (unsigned char)(CBR2K4&0x0FF)
#define UBBR_2K4H    (unsigned char)((CBR2K4>>8)&0x0FF)

#define RELOAD_TIM0  0xB8                  // константа перезагрузки 'TIM0'                        
#define START_TIM0   0x04                  // предв. делитель = 256
#define CBOD         4                     // константа Ѕќƒ
#define SYSTEM_TICK  600                   // “ик системного таймера в √ц
#define NDEV         4                     // базовый адрес устройства
#define T_LED_ON     (SYSTEM_TICK/5)       // врем€ индикации обращени€ к датчику

#define TEMP_OUT     0x0B                  // регистр температурных данных
#define INCL_OUT     0x0D                  // регистр угловых данных
#define INCL_180_OUT 0x0F                  // регистр угловых данных ±180∞

// длина "очереди усреднени€" ( 1, 2, 4, 8 или 16 )
#define NSUM 8

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
/*                              јппаратные особенности                          */
/*==============================================================================*/

// определени€ выводов порта D
#define RXD   PIND0                        // вывод входа приема данных ответа             IN  (^)
#define TXD   PORTD1                       // вывод выхода данных по запросу               OUT (1)
#define NAGR  PORTD4                       // вывод управлени€ нагревательным элементом    OUT (0)
#define SET_  PIND5                        // вывод входа конфигурации установок           IN  (^)
#define KLINE PORTD6                       // вывод управлени€ разрешени€ чипа 'K-LINE'    OUT (1)
                                           // "разрешение приема = 1"
#define RST   PORTB7                       // вывод управлени€ сбросом "ведомого"          OUT (1)
                                            
// определени€ выводов порта B
#define LED   PORTB0                       // вывод управлени€ сетодиодным индикатором     OUT (1)
#define CS    PORTB2                       // вывод управлени€ запуском "ведомого"         OUT (1)
#define MOSI  PORTB3                       // вывод выхода "ведущего"                      OUT (1)
#define MISO  PINB4                        // вывод входа "ведущего"                       IN  (^)
#define SCK   PORTB5                       // вывод выхода тактового сигнала от "ведущего" OUT (0)
/*
  обозначени€:
  - IN  (^) - вывод входа(чтение), ^ - резистивна€ подт€жка;
  - OUT (0) - вывод выхода(запись) начальное состо€ние 0, активное состо€ние 1;
  - OUT (1) - вывод выхода(запись) начальное состо€ние 1, активное состо€ние 0;
  Ќачальное и активное состо€ние выводов OUT определ€етс€ по принцип. схеме.
*/
/*==============================================================================*/
/*                                ќписание типов                                */
/*==============================================================================*/
union u_int                                // дл€ 2-х байтовых данных без знака
  {
    unsigned int  du_i;                    // data_union_int
    unsigned char du_c[2];                 // data_union_char
  };                                       // 256*[0] + [1]

union u_sint                               // дл€ 2-х байтовых данных со знаком
  {
    signed int  du_si;                     // data_union_int
    signed char du_sc[2];                  // data_union_char
  };                                       // 256*[0] + [1]

/*==============================================================================*/
/*                                ѕрототипы                                     */
/*==============================================================================*/
void SPI_MasterInit(void);
unsigned char SPI_WriteRead(unsigned char dataout);
unsigned int moving_average(unsigned int znachenie);
void termostat(signed int temperatura, unsigned char t0); 
#endif
