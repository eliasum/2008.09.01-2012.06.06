;                       АДРЕСА И КОНСТАНТЫ
;                            *****
$INCLUDE (ADuC814.inc) 
;
;
        ORG  0                  ;НИЖЕСЛЕДУЮЩАЯ КОМАНДА С АДРЕСА 0

        LJMP   TIMER            ;НА КОМАНДУ ПОСЛЕ МЕТКИ TIMER

        ORG    100H             ;НИЖЕСЛЕДУЮЩАЯ КОМАНДА С АДРЕСА 100Н

TIMER:                          ;ПРОГРАММА

;Настройка синтезатора
        MOV     PLLCON,#01H     ;Синтезатор рабочей частоты ядра 8388608 герц

;Настройка сторожевого таймера
        SETB    WDWR            ;разрешение записи в 'WDCON'
        MOV     WDCON,#72H;     ;enable WDT for 2.0s timeout

;Настройка UART
        MOV     RCAP2H,#0FFh    ;config UART for 9600 baud
        MOV     RCAP2L,#-7 
        MOV     TH2,#0FFh
        MOV     TL2,#-7
        MOV     SCON,#52h
        MOV     T2CON,#34h

;NASTROJKA T/S
        MOV IE,#0               ;VSE PRERYVANIJA ZAPRESCHENY
        MOV TMOD,#00000101B     ;T0 - 16-BITNYJ T/S V REZHIME "SCHETCHIK"
        MOV TH0,#0              ;SBROS T0
        MOV TL0,#0

;IZMERENIE DLITELNOSTI I PERIODA IMPULSOV S VYHODOV XOUT I YOUT 
X_F:    JNB P1.0,X_F            ;OZHIDANIE FRONTA 1-GO IMPULSA S VYHODA XOUT
        SETB TR0                ;ZAPUSK T0

X_S:    JB P1.0,X_S             ;OZHIDANIE SPADA 1-GO IMPULSA S VYHODA XOUT
        MOV A,TL0               ;V R1R0 - ZNACHENIE SPADA 1-GO IMPULSA S VYHODA XOUT
        MOV R0,A
        MOV A,TH0
        MOV R1,A

X_F2:   JNB P1.0,X_F2           ;OZHIDANIE FRONTA 2-GO IMPULSA S VYHODA XOUT
        MOV A,TL0               ;V R3R2 - ZNACHENIE FRONTA 2-GO IMPULSA S VYHODA XOUT
        MOV R2,A
        MOV A,TH0
        MOV R3,A
        CLR TR0                 ;OSTANOV T0
                          
        MOV TH0,#0              ;SBROS T0
        MOV TL0,#0
         
Y_F:    JNB P1.1,Y_F            ;OZHIDANIE FRONTA 1-GO IMPULSA S VYHODA YOUT
        SETB TR0                ;ZAPUSK T0

Y_S:    JB P1.1,Y_S             ;OZHIDANIE SPADA 1-GO IMPULSA S VYHODA YOUT
        MOV A,TL0               ;V R5R4 - ZNACHENIE SPADA 1-GO IMPULSA S VYHODA YOUT
        MOV R4,A
        MOV A,TH0
        MOV R5,A

Y_F2:   JNB P1.1,Y_F2           ;OZHIDANIE FRONTA 2-GO IMPULSA S VYHODA YOUT
        MOV A,TL0               ;V R7R6 - ZNACHENIE FRONTA 2-GO IMPULSA S VYHODA YOUT
        MOV R6,A
        MOV A,TH0
        MOV R7,A
        CLR TR0                 ;OSTANOV T0

        SJMP TIMER              ;ZACYKLIVANIE

BYTE:                           ;sends BYTE to UART
        JNB TI,$                ;wait til present byte gone
        CLR TI                  ;must clear TI
        MOV SBUF,A

        RET
        END