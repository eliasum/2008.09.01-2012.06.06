/********************************************************************************/
/*                                                                              */
/*                 ���� ����������� ��� ����������� ���������                   */
/*                                                                              */
/********************************************************************************/

#ifndef _TSMAIN_H
#define _TSMAIN_H

#include "Aduc814.h"

#define CBOD (11+5)/4                           // ��������� ���

/*==============================================================================*/
/*                                ���������                                     */
/*==============================================================================*/
#define FCORE 8388608                           // ������� ������� ������� ����
// ��������� �������� ������� ������������� ������� ������ �� UART
#define BAUD_RATE 600*4                         // �������� ����� 600 ���
#define CONST_BAUD_RATE (unsigned char)(256-(FCORE/(32L*12L*BAUD_RATE)))
// ��������� �������� ������� ������������� ������� ���������� ����
#define SYSTEM_TICK 600                         // ��� ���������� ������� � ��
#define LD_TIMER_SYS_TICK (unsigned int)(65536-(FCORE/(12L*SYSTEM_TICK)))
#define CLSTH (unsigned char)(LD_TIMER_SYS_TICK/256)
#define CLSTL (unsigned char)(LD_TIMER_SYS_TICK-(CLSTH*256))
// ��������� �������� ������� ������������� ������� ADC
#define ADC_RATE 6400*4                         // ������� 'ADC �� �������' � ��
#define CONST_ADC_RATE (unsigned int)(65536-(FCORE/(12L*ADC_RATE)))
#define CLATH (unsigned char)(CONST_ADC_RATE/256)
#define CLATL (unsigned char)(CONST_ADC_RATE-(CLATH*256))
// ��������� �������� ���
#define UDAC0 4095
#define UDAC0H (unsigned char)(UDAC0/256)
#define UDAC0L (unsigned char)(UDAC0-(UDAC0H*256))
#define UDAC1 (0x0fff-300)
#define UDAC1H (unsigned char)(UDAC1/256)
#define UDAC1L (unsigned char)(UDAC1-(UDAC1H*256))

#define ON_PIN 0                                // �������� ������� ������
#define OFF_PIN 1                               // ��������� ������� ������
#define DIR P36                                 // ���������� ��� ������������� ����� RX (RS485)
#define REQ_ME P35  // ???                      // ��������� ��������� � �������

#define NSUM 4                                  // ���������� �������� ���������� 2/4/8
#define AIN0 0                                  // N ����� AIN0

#define NDEV 6                                  // ������� ����� AIN0 (6,7,...,13)

/*==============================================================================*/
/*                                �������� �����                                */
/*==============================================================================*/
union u_int                                     // ��� 2-� �������� ������
  {
   unsigned int du_i;                           // data_union_int
   unsigned char du_c[2];                       // data_union_char
  };                                            // 256*[0] + [1]

union u_lng                                     // ��� 4-� �������� ������
  {
   unsigned long du_l;                          // data_union_long
   unsigned char du_c[4];                       // data_union_char
  };
/*==============================================================================*/
/*                                ���������                                     */
/*==============================================================================*/
void Adc_sw (unsigned char chanell);
void Go_adc (unsigned char chanell);
void Out_data (unsigned char chanell);
void Adc_ini(void);
void Add_sum (void);
void Refr(void);

/*==============================================================================*/
/*                                �������                                       */
/*==============================================================================*/
#define RWDT {EA=0; WDWR=1; WDE=1; EA=1;} 

#endif

