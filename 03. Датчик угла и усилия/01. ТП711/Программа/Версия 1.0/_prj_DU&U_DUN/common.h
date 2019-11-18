#ifndef _common_h_
#define _common_h_

#include <ioavr.h>
#include <intrinsics.h>

#define F_CPU 11059200

#define F_CPU_kHz (unsigned long)(CPU_CLK_Hz/1000)

#define delay_ns(x) __delay_cycles(x*F_CPU_kHz*0.000001)
#define delay_us(x) __delay_cycles(x*(F_CPU/1000000))
#define delay_ms(x) __delay_cycles(x*(F_CPU/1000))
#define delay_s(x)  __delay_cycles(x*F_CPU)

// Инвертирование бита:
#ifndef cpl
#define cpl(sfr, bit) \
  if(sfr & (1<<bit))  \
  {                   \
   sfr &= ~(1<<bit);  \
  }                   \
  else                \
  {                   \
    sfr |= (1<<bit);  \
  }
#endif // cpl

#endif // _common_h_

