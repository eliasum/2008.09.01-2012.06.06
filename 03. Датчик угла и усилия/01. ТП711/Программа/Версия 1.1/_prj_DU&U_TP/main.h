#ifndef _main_h_
#define _main_h_

#include "common.h"
#include "terminal.h"

#define BR9K6        9600                  // �������� ������ 9600
#define UBBR_9K6     ((F_CPU/(16L*BR9K6))-1)

#define CBR2K4       (unsigned int) ((F_CPU/(16L*2400))-1)
#define UBBR_2K4L    (unsigned char)(CBR2K4&0x0FF)
#define UBBR_2K4H    (unsigned char)((CBR2K4>>8)&0x0FF)

#define RELOAD_TIM0  0xB8                  // ��������� ������������ 'TIM0'                        
#define START_TIM0   0x04                  // �����. �������� = 256
#define CBOD         4                     // ��������� ���
#define SYSTEM_TICK  600                   // ��� ���������� ������� � ��
#define NDEV         6                     // ������� ����� ����������
#define T_LED_ON     (SYSTEM_TICK/5)       // ����� ��������� ��������� � �������

#define TEMP_OUT     0x0B                  // ������� ������������� ������
#define AUX_DAC      0x31                  // ������� ������ ���

// ����� "������� ����������" ( 1, 2, 4, 8 ��� 16 )
#define NSUM 8

// ������������ ��������������� ����.�������
#if NSUM == 2
  #define KDEL 1
#else
  #if NSUM == 4
    #define KDEL 2
  #else
    #if NSUM == 8
      #define KDEL 3
    #else
      #if NSUM == 16
        #define KDEL 4
      #else
        #define KDEL 0
      #endif
    #endif
  #endif
#endif
/*==============================================================================*/
/*                              ���������� �����������                          */
/*==============================================================================*/

// ����������� ������� ����� B
#define LED   PORTB0                       // ����� ���������� ����������� �����������     OUT (1)
#define RST   PORTB1                       // ����� ���������� ������� "��������"          OUT (1)
#define CS    PORTB2                       // ����� ���������� �������� "��������"         OUT (1)
#define MOSI  PORTB3                       // ����� ������ "��������"                      OUT (1)
#define MISO  PINB4                        // ����� ����� "��������"                       IN  (^)
#define SCK   PORTB5                       // ����� ������ ��������� ������� �� "��������" OUT (0)

// ����������� ������� ����� D
#define RXD   PIND0                        // ����� ����� ������ ������ ������             IN  (^)
#define TXD   PORTD1                       // ����� ������ ������ �� �������               OUT (1)
#define SET_  PIND2                        // ����� ����� ������������ ���������           IN  (^)
#define TST   PORTD3                       // ����� ���������� �����������                 OUT (0)
#define S1    PIND4                        // ����� ������� ��� ���������� ��������� �1    IN  (^)
#define S2    PIND5                        // ����� ������� ��� ���������� ��������� �2    IN  (^)
#define S3    PIND6                        // ����� ������� ��� ���������� ��������� �3    IN  (^)
#define DAC0  PORTD7                       // ����� ���������� �������� �����              OUT (0)

/*
  �����������:
  - IN  (^) - ����� �����(������), ^ - ����������� ��������;
  - OUT (0) - ����� ������(������) ��������� ��������� 0, �������� ��������� 1;
  - OUT (1) - ����� ������(������) ��������� ��������� 1, �������� ��������� 0;
  ��������� � �������� ��������� ������� OUT ������������ �� �������. �����.
*/
/*==============================================================================*/
/*                                �������� �����                                */
/*==============================================================================*/
union u_int                                // ��� 2-� �������� ������ ��� �����
  {
    unsigned int  du_i;                    // data_union_int
    unsigned char du_c[2];                 // data_union_char
  };                                       // 256*[0] + [1]

union u_sint                               // ��� 2-� �������� ������ �� ������
  {
    signed int  du_si;                     // data_union_int
    signed char du_sc[2];                  // data_union_char
  };                                       // 256*[0] + [1]

/*==============================================================================*/
/*                                ���������                                     */
/*==============================================================================*/
void SPI_MasterInit(void);
unsigned char SPI_WriteRead(unsigned char dataout);
unsigned int moving_average(unsigned int znachenie);
void termostat(signed int temperatura, unsigned char t0); 
#endif
