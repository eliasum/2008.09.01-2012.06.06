/*********************************************************************************
Title:    DS18X20-Functions via One-Wire-Bus
**********************************************************************************/

#include <avr/io.h>

#include "ds18x20.h"
#include "onewire.h"
#include "crc8.h"

///*
//   convert raw value from DS18x20 to Celsius
//   input is:
//   - familycode fc (0x10/0x28 see header)
//   - scratchpad-buffer
//     output is:
//   - cel full celsius
//   - fractions of celsius in millicelsius*(10^-1)/625 (the 4 LS-Bits)
//   - subzero =0 positiv / 1 negativ
//   always returns  DS18X20_OK
//   TODO invalid-values detection (but should be covered by CRC)
//*/
//uint8_t DS18X20_meas_to_cel( uint8_t fc, uint8_t *sp,
//                             uint8_t* subzero, uint8_t* cel, uint8_t* cel_frac_bits)
//{
//  uint16_t meas;
//  uint8_t  i;
//
//  meas = sp[0];  // LSB
//  meas |= ((uint16_t)sp[1])<<8; // MSB
//  //meas = 0xff5e; meas = 0xfe6f;
//
//  //  only work on 12bit-base
//  if ( fc == DS18S20_ID )
//  { // 9 -> 12 bit if 18S20
//    /* Extended measurements for DS18S20 contributed by Carsten Foss */
//    meas &= (uint16_t) 0xfffe;  // Discard LSB , needed for later extended precicion calc
//    meas <<= 3;                 // Convert to 12-bit , now degrees are in 1/16 degrees units
//    meas += (16 - sp[6]) - 4;   // Add the compensation , and remember to subtract 0.25 degree (4/16)
//  }
//  // check for negative
//  if ( meas & 0x8000 )
//  {
//    *subzero=1;      // mark negative
//    meas ^= 0xffff;  // convert to positive => (twos complement)++
//    meas++;
//  }
//  else *subzero=0;
//
//  // clear undefined bits for B != 12bit
//  if ( fc == DS18B20_ID )
//  { // check resolution 18B20
//    i = sp[DS18B20_CONF_REG];
//    if ( (i & DS18B20_12_BIT) == DS18B20_12_BIT )
//      ;
//    else if ( (i & DS18B20_11_BIT) == DS18B20_11_BIT )
//      meas &= ~(DS18B20_11_BIT_UNDF);
//    else if ( (i & DS18B20_10_BIT) == DS18B20_10_BIT )
//      meas &= ~(DS18B20_10_BIT_UNDF);
//    else
//      // if ( (i & DS18B20_9_BIT) == DS18B20_9_BIT ) {
//      meas &= ~(DS18B20_9_BIT_UNDF);
//  }
//
//  *cel  = (uint8_t)(meas >> 4);
//  *cel_frac_bits = (uint8_t)(meas & 0x000F);
//  return DS18X20_OK;
//}

/* start measurement (CONVERT_T) for all sensors if input id==NULL
   or for single sensor. then id is the rom-code */
uint8_t DS18X20_start_meas( uint8_t with_power_extern, uint8_t id[])
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

///* reads temperature (scratchpad) of a single sensor (uses skip-rom)
//   output: subzero==1 if temp.<0, cel: full celsius, mcel: frac
//   in millicelsius*0.1
//   i.e.: subzero=1, cel=18, millicel=5000 = -18,5000°C */
//uint8_t DS18X20_read_meas_single(uint8_t familycode, uint8_t *subzero,
//                                 uint8_t *cel, uint8_t *cel_frac_bits)
//
//{
//  uint8_t i;
//  uint8_t sp[DS18X20_SP_SIZE];
//
//  ow_command(DS18X20_READ, NULL);
//  for ( i=0 ; i< DS18X20_SP_SIZE; i++ ) sp[i]=ow_byte_rd();
//  if ( crc8( &sp[0], DS18X20_SP_SIZE ) )
//    return DS18X20_ERROR_CRC;
//  DS18X20_meas_to_cel(familycode, sp, subzero, cel, cel_frac_bits);
//  return DS18X20_OK;
//}


/* считывает температуру из RAM DS18B20
   и возвращает число [0..255], пропор-
   циональное температуре с дискр. 1'C
*/
uint8_t DS18X20_read_meas_in_tick(uint8_t *ttick)
{
  uint8_t meas, i;
  uint8_t sp[DS18X20_SP_SIZE];

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


