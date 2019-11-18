#include "i2c.h"

void i2c_init(void)
{
  PO_I2C &= ~(1<<SDA_PT);                        // ������ �������� ��� �����
  PO_I2C &= ~(1<<SCL_PT);                        // � �������������� �����������
  SDA_SET;                                       // ������� ��������� � '1'
  SCL_SET;
}

void i2c_start(void)
{
  SDA_SET;
  SCL_SET;
  delay_us(5);                                  // delay for halve period
  SDA_RES;
  delay_us(5);
  SCL_RES;
}

void i2c_stop(void)
{
  SDA_RES;
  delay_us(5);
  SCL_SET;
  delay_us(5);
  SDA_SET;
}

unsigned char i2c_read(unsigned char acknowledge)
{
  unsigned char sreg, res=0;
  unsigned char cnt;

  sreg=SREG;                                     
  __disable_interrupt();
  SDA_SET;
  for ( cnt=0; cnt<8; cnt++ )
  {
    SCL_SET;
    delay_us(5);
    res<<=1;                                     // ����� ������� ���������� ���� ���������� 
    if (PI_I2C & (1<<SDA_IN)) res |= 0x01;       // ����� "����������" ��. ������
    SCL_RES;
    delay_us(5);
  }
  if (acknowledge) SDA_RES;
  else SDA_SET;
  SCL_SET;
  delay_us(5);
  SCL_RES;
  SREG=sreg; // sei()
/*
��� ������ ����������� ��������� ���������� ������� ��������� SREG �� �����������. 
������� ������������ ������ �������������� ���������� ���������� ����� �������� 
��� ����� � ������������ ��������� ���������� (���� ��� ����������) �
��������������� ��� �������� ����� ������� ������� RETI.
*/
  return res;
}

unsigned char i2c_write(unsigned char d)
{
  unsigned char sreg, i;
  unsigned char ack;

  sreg=SREG;
  __disable_interrupt();
  for (i=0; i<8; i++)
  {
    if (d & 0x80) SDA_SET;
    else         SDA_RES;
    delay_us(5);
    SCL_SET;
    delay_us(5);
    SCL_RES;
    delay_us(5);
    d = d << 1;
  }
  SDA_SET;
  delay_us(5);
  SCL_SET;
  delay_us(5);
  if (PI_I2C & (1<<SDA_IN)) ack = 1;
  else                      ack = 0;
  SCL_RES;
  SREG=sreg; // sei()
  return(ack);
}

