#ifndef _main_h_
#define _main_h_

#include "common.h"

#define BYTE_ONE 0                         // ������� ��
#define BYTE_TWO 1                         // �����������

#define BR19K2 19200                       // �������� ������ 19200
#define BR9K6 9600                         // �������� ������ 9600
#define BR2K4 2400                         // �������� ������ 2400
#define UBBR_19K2 ((F_CPU / (16L * BR19K2)) - 1)
#define UBBR_9K6 ((F_CPU / (16L * BR9K6)) - 1)
#define UBBR_2K4 ((F_CPU / (16L * BR2K4)) - 1)

// ������� ���������� ���� = 600 ��:
#define CONST_RELOAD 0xB8                  // ��������� ������������ ������� '0'
#define CBOD 4                             // ��������� ���
#define SYSTEM_TICK 600                    // ��� ���������� ������� � ��
#define NDEV 15                            // ������� ����� ����������
#define RR_CONST 150                       // 0.25 ��� �� ����������� 'WDT'

#define TO_CEL_SET (SYSTEM_TICK*3)         // ���-� ����-�� 1 ��� �� 10 ���

#define TEMP_OUT 0x0B                      // ������� ������������� ������
#define INCL_180_OUT 0x0F                  // ������� ������� ������ �180�


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

// ����������� ������� ����� D

#define RXD PIND0                          // ����� ����� ������ ������ ������             IN  (^)
#define TXD PORTD1                         // ����� ������ ������ �� �������               OUT (1)
#define STR PORTD4                         // ����� ���������� ���������� ��������         OUT (1)
#define SET_ PIND5                         // ����� ����� ������������ ���������           IN  (^)
#define KLINE PORTD6                       // ����� ���������� ���������� ���� 'K-LINE'    OUT (1)
                                           // "���������� ������ = 1"
#define RST PORTB7                         // ����� ���������� ������� "��������"          OUT (1)

//#define NAGR PORTDx                        // ����� ���������� �������������� ���������    OUT (0)
                                            
// ����������� ������� ����� B

#define LED PORTD0                         // ����� ���������� ����������� �����������     OUT (1)
#define DIR PORTD1                         // ����� ���������� ������������ 'RS485'        OUT (0)
                                           // "���������� �������� = 1"
#define CS PORTB2                          // ����� ���������� �������� "��������"         OUT (1)
#define MOSI PORTB3                        // ����� ������ "��������"                      OUT (1)
#define MISO PINB4                         // ����� ����� "��������"                       IN  (^)
#define SCK PORTB5                         // ����� ������ ��������� ������� �� "��������" OUT (0)
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
union u_int                                // ��� 2-� �������� ������
  {
    unsigned int  du_i;                    // data_union_int
    unsigned char du_c[2];                 // data_union_char
  };                                       // 256*[0] + [1]

union u_sig_int
  {
    signed int    du_i;                    // data_union_int
    unsigned char du_c[2];                 // data_union_char
  };

struct MEASURE {                           // �������� ��������� 1-�� ������
  union u_int T1X;
  union u_int T2X;
  union u_int T1Y;
  union u_int T2Y;  };

/*==============================================================================*/
/*                                ���������                                     */
/*==============================================================================*/
void Get_Buff(void);
void Meas_Cel(void);
void meas_xy(void);
void Tx_means_tick(void);
void Tx_byte(char outbyte);
//void termostat(void);
void SPI_MasterInit(void);
void SPI_MasterTransmit(char cData);
void SPI_SlaveInit(void);
char SPI_SlaveReceive(void);
unsigned char SPI_WriteRead(unsigned char dataout);
#endif
