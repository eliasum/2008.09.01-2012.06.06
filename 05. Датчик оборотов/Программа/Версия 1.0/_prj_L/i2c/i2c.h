#ifndef _DRIVER_I2C_H
#define _DRIVER_I2C_H

#include "..\main.h"

enum nak_ack {nak, ack};

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
unsigned char i2c_read(unsigned char acknowledge);
unsigned char i2c_write(unsigned char d);

// Конфигурация I2C шины:

#define PO_I2C PORTC
#define PI_I2C PINC
#define DD_I2C DDRC

#define SCL_PT PORTC5
#define SCL_DD DDC5
#define SDA_PT PORTC4
#define SDA_DD DDC4
#define SDA_IN PINC4

// Операции I2C шины:

#define SCL_SET DD_I2C &=~(1<<SCL_DD)       // SCL выдает 1 
#define SCL_RES DD_I2C |= (1<<SCL_DD)       // SCL выдает 0 
#define SDA_SET DD_I2C &=~(1<<SDA_DD)       // SDA выдает 1 
#define SDA_RES DD_I2C |= (1<<SDA_DD)       // SDA выдает 0
/*
Данные операции I2C шины определены при использовании понятия "открытый коллектор".
В данном случае изменением регистра DDR достигается вывод 0 или 1.
Выводы SCL и SDA порта С работают на вывод (при инициализации SCL_PT и SDA_PT = 0):
- при DDR = 1 вывод нуля;
- при DDR = 0 вывод единицы, т.к. в этом случае вывод будет в высокоимпедансном
состоянии, а на ножке будет 1, т.к. есть подтяжка через резистор на питание.
*/
#endif
