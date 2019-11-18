#include "24Cxx.h"

//  Подпрограмма записи байта в EEPROM (Byte Write)
void FM24Cxx_put(unsigned char chip, unsigned int addr, signed char data)
{
  unsigned char ack;
  unsigned char i=100;

  do
  {
    i2c_start();
    ack=i2c_write(((chip<<1)&0x0E)|0xA0);        // chip = A2 A1 A15 --> chip = 1 0 1 0 A2 A1 A15 0
    i--;
  } while (i && ack);
  if (i)
  {
    i2c_write((unsigned char)(addr/256));        // addr_high
    i2c_write((unsigned char)(addr&0x00FF));     // addr_low
    i2c_write(data);
  }
  i2c_stop();

  delay_us(5);
}

// Подпрограмма чтения байта из EEPROM (Random Read)
signed char FM24Cxx_get(unsigned char chip, unsigned int addr)
{
  unsigned char res;
// Ложная запись:
  res=(((chip<<1)&0x0E)|0xA0);
  i2c_start();
  i2c_write(res);
  i2c_write((unsigned char)(addr/256));          // addr_high
  i2c_write((unsigned char)(addr&0x00FF));       // addr_low
// Само чтение:
  i2c_start();
  i2c_write(res+1);
  res=i2c_read(nak);
  i2c_stop();
  return res;
}

/*   
      {MSB_____Device Address____LSB} {addr_high} {addr_low} ...

      1  0  1  0    a2  a1  a15  r/w
*/

