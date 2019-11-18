/*********************************************************************************
Title:    DS18X20-Functions via One-Wire-Bus
**********************************************************************************/

#include "common.h"

#include "ds18x20.h"
#include "onewire.h"
#include "crc8.h"

/* start measurement (CONVERT_T) for all sensors if input id==NULL
   or for single sensor. then id is the rom-code */
unsigned char DS18X20_start_meas( unsigned char with_power_extern, unsigned char id[])
{
  ow_reset(); //**
  if ( ow_input_pin_state() )
  { // only send if bus is "idle" = high
    ow_command( DS18X20_CONVERT_T, id );
    if (with_power_extern != DS18X20_POWER_EXTERN)
      ow_parasite_enable();

    return DS18X20_OK;
  }
  else
  { //
    return DS18X20_START_FAIL;
  }
}


/* считывает температуру из RAM DS18B20
   и возвращает число [0..255], пропор-
   циональное температуре с дискр. 1'C
*/
unsigned char DS18X20_read_meas_in_tick(unsigned char *ttick)
{
  unsigned char meas, i;
  unsigned char sp[DS18X20_SP_SIZE];

  ow_command(DS18X20_READ, NULL);
  for ( i=0 ; i< DS18X20_SP_SIZE; i++ ) sp[i]=ow_byte_rd();
  if ( crc8( &sp[0], DS18X20_SP_SIZE ) )
    return DS18X20_ERROR_CRC;
  meas = sp[0];  // LSB
  meas >>= 4;
  meas &= 0x0f;
  i = sp[1];     // MSB
  i <<= 4;
  i &= 0xf0;
  meas |= i;
  if(0x80 & meas)
  {
    meas &= 0xef;
    meas  = 0x7f - meas;
  }
  else
  {
    meas |= 0x80;
  }
  *ttick = meas;
  return DS18X20_OK;
}


