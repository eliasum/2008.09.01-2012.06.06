#ifndef _DRIVER_24cXX_H
#define _DRIVER_24cXX_H

#include "..\i2c\i2c.h"

signed char FM24Cxx_get(unsigned char chip, unsigned int addr);

void FM24Cxx_put(unsigned char chip, unsigned int addr, signed char data);

#endif
