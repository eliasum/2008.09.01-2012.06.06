;********************************************************************************
;*                                                                              *
;*                        ������� ���������� ��� 'ATMEGA168P'                   *
;*                               (22 ������� 2009 �)                            *
;*                                                                              *
;********************************************************************************

.NOLIST                            ; ���������� ������������ ������ �� ����� ��������
.INCLUDE "m168def.inc"             ; ���������� ���� �������� ���� ��������� �����/������
.LIST                              ; ��������� ������������ ������ � ���� ��������

.equ F_CPU = 11059200              ; ������� ����������
.def temp = r16                    ; ��� ���������� ��������

; ������������� ����� B
.equ LED = PORTB0                  ; ����� ���������� ����������� �����������  OUT (1)
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

; ������������� ������������ ������ �����/������
        ldi     temp,(1<<STR)|(1<<LED)
        out     PORTB,temp
        ldi     temp,(1<<STR)|(1<<LED)
        out     DDRB,temp

MAIN: 

        sbi     PORTB,LED
        call    s1s
        cbi     PORTB,LED
        call    s1s

s5mks:   NOP
		NOP
		NOP
		NOP
		NOP

		ret

s25mks:  call    s5mks 
        call    s5mks
		call    s5mks
		call    s5mks
		call    s5mks

		ret

s100mks: call    s25mks 
		call    s25mks
		call    s25mks
		call    s25mks

		ret

s1ms:    call    s100mks
		call    s100mks
		call    s100mks
		call    s100mks
		call    s100mks
		call    s100mks
		call    s100mks
		call    s100mks
		call    s100mks
		call    s100mks

		ret

s10ms:   call    s1ms
		call    s1ms
		call    s1ms
		call    s1ms
		call    s1ms
		call    s1ms
		call    s1ms
		call    s1ms
		call    s1ms
		call    s1ms

		ret

s100ms:  call    s10ms
        call    s10ms
		call    s10ms
		call    s10ms
		call    s10ms
		call    s10ms
		call    s10ms
		call    s10ms
		call    s10ms
		call    s10ms

		ret

s1s:     call    s100ms
        call    s100ms
		call    s100ms
		call    s100ms
		call    s100ms
		call    s100ms
		call    s100ms
		call    s100ms
		call    s100ms
		call    s100ms

		ret

        sbic    PINB,STR           ; STR=0 ?
        rjmp    WD0
        sbi     PORTB,STR          ; STR->'1'
        rjmp    MAIN               ; �� �������� ���������
WD0:    cbi     PORTB,STR          ; STR->'0'
        rjmp    MAIN               ; �� �������� ���������
