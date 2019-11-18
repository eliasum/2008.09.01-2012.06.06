;********************************************************************************
;*                                                                              *
;*                   ��������� �������� �������� ��� 'ATMEGA168P'               *
;*                              (23 ������� 2009 �)                             *
;*                                                                              *
;********************************************************************************

.NOLIST                            ; ���������� ������������ ������ �� ����� ��������
.INCLUDE "m168def.inc"             ; ���������� ���� �������� ���� ��������� �����/������
.LIST                              ; ��������� ������������ ������ � ���� ��������

.equ F_CPU = 11059200              ; ������� ����������
.equ BAUDRATE = 2400               ; �������� ������ �� LINE

; ��������� ������������ Tim0 (2400 ��)
.equ CTINT = 256-((F_CPU/64)/BAUDRATE) 

.def temp = r16                    ; ��� ���������� ��������
.def temp2 = r17                   ; ��� ���������� �������� 2 
.def temp3 = r18                   ; ��� ���������� �������� 2 
.def v_cnt = r19                   ; ������� ���������� ������� �������� 
                                   ; ������ ���� �� ����� ������ � ������ �������

; ������������� ����� D

; PIN� - ������� ������� ������, ������������ ����������� ������ ������ 
; PORTx - ������� ����� ������, ������������ ����������� ������/������
; DDRx - ������� ����������� ������, ������������ ����������� ������/������

.equ RXD = PIND0                   ; ����� ����� ������ ������ ������          IN  (^)
.equ TXD = PORTD1                  ; ����� ������ ������ �� �������            OUT (1)
.equ TMPR_WR = PORTD2              ; ����� ������� ������ � �����������        OUT (1)
.equ TMPR_RD = PIND2               ; ����� ����� ������ ������ � �����������   IN  (^)
.equ XOUT = PIND3                  ; ����� ����� ������ ������ �� 'X'          IN  (^)
.equ YOUT = PIND4                  ; ����� ����� ������ ������ �� 'Y'          IN  (^)
.equ SET_ = PIND5                  ; ����� ����� ������������ ���������        IN  (^)
.equ KLINE = PORTD6                ; ����� ���������� ���������� ���� 'K-LINE' OUT (1)
                                   ; "���������� ������ = 1"     
.equ RS485 = PORTD7                ; ����� ���������� ���������� ���� 'RS485'  OUT (1)
                                   ; "���������� ������ = 0"

; ������������� ����� B
.equ LED = PORTB0                  ; ����� ���������� ����������� �����������  OUT (1)
.equ DIR = PORTB1                  ; ����� ���������� ������������ 'RS485'     OUT (0) 
                                   ; "���������� �������� = 1"
.equ STR = PORTB2                  ; ����� ���������� ���������� ��������      OUT (1)

.CSEG                              ; ������ ������������ ��������

        rjmp    START              ; ����� ��� ������

.ORG    OVF0addr                   ; ����� ������� ���������� �� ������������ ������� 0
                                   ; (��� "OVF0addr" ���������� � ����� "m168def.inc")                                                                        

START:
        ldi     temp,low(RAMEND)   ; ������������� ��������� ����� ����� ������  
        out     SPL,temp               ; � ���� �������� �������� ������ ������ ������
        ldi     temp,high(RAMEND)  ; ("���������������� AVR")
        out     SPH,temp           ; (��� "RAMEND" ���������� � ����� "m168def.inc") 

; ������������� USART (9600)
        ldi     temp,0x00            
        sts     UCSR0A,temp        ; ������ �� ����� ��������� ��������        
        sts     UCSR0B,temp        ; �������� �������� USART
        ldi     temp,0x06           
        sts     UCSR0C,temp        ; ������ ����� ������ 8 ���
        ldi     temp,0x47           
        sts     UBRR0L,temp        ; ������� �������� 9600 ���
        ldi     temp,0x00           
        sts     UBRR0H,temp
        ldi     temp,0x08          
        sts     UCSR0B,temp        ; ���������� ��������

; ������������� ������������ ������ �����/������
        ldi     temp,(1<<RS485)|(1<<KLINE)|(1<<SET_)|(1<<YOUT)|(1<<XOUT)|(1<<TMPR_RD)|(1<<TXD)|(1<<RXD)
        out     PORTD,temp
        ldi     temp,(1<<RS485)|(1<<KLINE)|(1<<TXD)
        out     DDRD,temp
        ldi     temp,(1<<STR)|(1<<LED)
        out     PORTB,temp
        ldi     temp,(1<<STR)|(1<<DIR)|(1<<LED)
        out     DDRB,temp

MAIN: 

; ��������� �/�1
        ldi     temp,0x00          ; ������������ � �������� �/�1
        sts     TCCR1B,temp        ; ������� �/�1
        sts     TCNT1H,temp
        sts     TCNT1L,temp        ; ����� �/�1
        ldi     temp2,0x02         ; ���������� �������� �� '8'

; ��������

        sts     TCCR1B,temp2       ; ������ �/�1 � �������� �������� CLK_io/8

        ldi     temp,0xD2          ; 1234
        MOV     R0, temp
        ldi     temp,0x4
        MOV     R1,temp

        ldi     temp,0x2E          ; 5678
        MOV     R2, temp
        ldi     temp,0x16
        MOV     R3,temp

        ldi     temp,0x3D          ; 8765
        MOV     R4, temp
        ldi     temp,0x22
        MOV     R5,temp

        ldi     temp,0xE1          ; 4321
        MOV     R6, temp
        ldi     temp,0x10
        MOV     R7,temp

        sts     TCCR1B,temp        ; ������� �/�1 � �������� �������� CLK_io/8

; ������� ����������� �������:
        ldi     temp,0x5A          ; ����� ������ ����� ������
        mov     temp2,temp         ; ��������� �������� ����������� �����

        call    TX_BYTE            ; �������� ���������� �������� ����������� �����

        mov     temp,R0  
        call    TX_SUM             ; �������� ����������� �������� R0
         
        mov     temp,R1  
        call    TX_SUM             ; �������� ����������� �������� R1
         
        mov     temp,R2  
        call    TX_SUM             ; �������� ����������� �������� R2
          
        mov     temp,R3  
        call    TX_SUM             ; �������� ����������� �������� R3
         
        mov     temp,R4  
        call    TX_SUM             ; �������� ����������� �������� R4
          
        mov     temp,R5  
        call    TX_SUM             ; �������� ����������� �������� R5
          
        mov     temp,R6  
        call    TX_SUM             ; �������� ����������� �������� R6
         
        mov     temp,R7  
        call    TX_BYTE            ; �������� ����������� �������� R7

        add     temp,temp2              
        call    TX_BYTE            ; �������� ��������� �������� ����������� �����

        sbic    PINB,STR           ; STR=0 ?
        rjmp    WD0
        sbi     PORTB,STR          ; STR->'1'
        rjmp    MAIN               ; �� �������� ���������
WD0:    cbi     PORTB,STR          ; STR->'0'
        rjmp    MAIN               ; �� �������� ���������

; �������� ����� ����� 'USART'
TX_BYTE:
        lds     temp3,UCSR0A
        sbrs    temp3,UDRE0        ; ����� ������� ������ �����������
        rjmp    TX_BYTE
        sts     UDR0,temp          ; ��������� ������� ���� ������  
                                   ; � �����, ������ ��������
        ret                                         

; �������� ����� � ��������� ����������� �����
TX_SUM:      
        call    TX_BYTE
        add     temp2,temp      
        ret

; ----------------------------------------------------------
; BAUD = 9600
; BAUD = fck/16*(UBRR+1)
; UBRR = [fck/16*BAUD]-1 = 0x47 = 0b01000111
; UBRR0L = 0x47 = 0b01000111
; UBRR0H = 0x00 = 0b00000000
; ----------------------------------------------------------
