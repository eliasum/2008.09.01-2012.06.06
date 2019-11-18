/********************************************************************************/
/*                                                                              */
/*                 ���� ����������� ��� ����������� ���������                   */
/*                                                                              */
/********************************************************************************/

#ifndef _MAIN_H
#define _MAIN_H

#include "Aduc814.h"

#define XOUT P10
#define YOUT P11

#define BYTE_ONE 1
#define BYTE_TWO 0

// ��������� 'c'-������� � ����� 'Main.c'
unsigned char meas_xy(void);
void Tx_means(void);
void Tx_byte(char outbyte);

#endif

