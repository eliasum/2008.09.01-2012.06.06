#ifndef _DRIVER_I2C_H
#define _DRIVER_I2C_H

#include "..\main.h"

enum nak_ack {nak, ack};

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
unsigned char i2c_read(unsigned char acknowledge);
unsigned char i2c_write(unsigned char d);

// ������������ I2C ����:

#define PO_I2C PORTC
#define PI_I2C PINC
#define DD_I2C DDRC

#define SCL_PT PORTC5
#define SCL_DD DDC5
#define SDA_PT PORTC4
#define SDA_DD DDC4
#define SDA_IN PINC4

// �������� I2C ����:

#define SCL_SET DD_I2C &=~(1<<SCL_DD)       // SCL ������ 1 
#define SCL_RES DD_I2C |= (1<<SCL_DD)       // SCL ������ 0 
#define SDA_SET DD_I2C &=~(1<<SDA_DD)       // SDA ������ 1 
#define SDA_RES DD_I2C |= (1<<SDA_DD)       // SDA ������ 0
/*
������ �������� I2C ���� ���������� ��� ������������� ������� "�������� ���������".
� ������ ������ ���������� �������� DDR ����������� ����� 0 ��� 1.
������ SCL � SDA ����� � �������� �� ����� (��� ������������� SCL_PT � SDA_PT = 0):
- ��� DDR = 1 ����� ����;
- ��� DDR = 0 ����� �������, �.�. � ���� ������ ����� ����� � �����������������
���������, � �� ����� ����� 1, �.�. ���� �������� ����� �������� �� �������.
*/
#endif
