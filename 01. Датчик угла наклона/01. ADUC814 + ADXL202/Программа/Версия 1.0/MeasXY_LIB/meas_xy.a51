$NOMOD51                        ; "отключаем" стандартный '51'
$INCLUDE (ADuC814.inc)

NAME    MEAS_XY


?PR?_meas_xy?MEAS_XY  SEGMENT CODE
?DT?_meas_xy?MEAS_XY  SEGMENT DATA OVERLAYABLE

        PUBLIC  meas_xy


        RSEG    ?DT?_meas_xy?MEAS_XY

?_meas_xy?BYTE:
        buff?040:   DS   8

XOUT    EQU     P1.0
YOUT    EQU     P1.1

; =============================================================================
; =  Получение 1-го замера 'X&Y' с 'ADXL202'                                  =
; = ------------------------------------------------------------------------- =
; =  Вход:  Ничего                                                            =
; =  Выход: R7 - адрес заполненного буфера данными замера                     =
; =============================================================================
; char idata* meas_xy(void) {



        RSEG    ?PR?_meas_xy?MEAS_XY
        USING   0

meas_xy:

        MOV     TH0,#0          ; сброс T0 
        MOV     TL0,#0

; измерение длительности и периода импульсов с выходов Xout и Yout

X_K1:   JB      XOUT,X_K1       ; корректировка расчета длительности импульсов с выхода Xout

X_F:    JNB     XOUT,X_F        ; ожидание фронта 1-го импульса с выхода Xout      	
	    SETB    TR0             ; запуск T0
		
X_S:    JB      XOUT,X_S        ; ожидание спада 1-го импульса с выхода Xout
	  	CLR     TR0             ; останов T0                  

        MOV     A,TL0           ; в R1R0 - значение длительности импульсов с выхода Xout
        MOV     R0,A
        MOV     A,TH0
        MOV     R1,A

        MOV     TH0,#0          ; сброс T0
        MOV     TL0,#0

X_F2:   JNB     XOUT,X_F2       ; ожидание фронта 2-го импульса с выхода Xout
	    SETB    TR0             ; запуск T0

X_S2:   JB      XOUT,X_S2       ; ожидание спада 2-го импульса с выхода Xout
X_F3:   JNB     XOUT,X_F3       ; ожидание фронта 3-го импульса с выхода Xout
		CLR     TR0             ; останов T0

        MOV     A,TL0           ; в R3R2 - значение периода импульсов с выхода Xout
        MOV     R2,A
        MOV     A,TH0
        MOV     R3,A
              
        MOV     TH0,#0          ; сброс T0
        MOV     TL0,#0

Y_K2:   JB      YOUT,Y_K2       ; корректировка расчета длительности импульсов с выхода Yout 

Y_F:    JNB     YOUT,Y_F        ; ожидание фронта 1-го импульса с выхода Yout      	
	    SETB    TR0             ; запуск T0
		
Y_S:    JB      YOUT,Y_S        ; ожидание спада 1-го импульса с выхода Yout
        CLR     TR0             ; останов T0                  

        MOV     A,TL0           ; в R5R4 - значение длительности импульсов с выхода Yout
        MOV     R4,A
        MOV     A,TH0
        MOV     R5,A

        MOV     TH0,#0          ; сброс T0
        MOV     TL0,#0

Y_F2:   JNB     YOUT,Y_F2       ; ожидание фронта 2-го импульса с выхода Yout
	    SETB    TR0             ; запуск T0

Y_S2:   JB      YOUT,Y_S2       ; ожидание спада 2-го импульса с выхода Yout
Y_F3:   JNB     YOUT,Y_F3       ; ожидание фронта 3-го импульса с выхода Yout
        CLR     TR0             ; останов T0

        MOV     A,TL0           ; в R7R6 - значение периода импульсов с выхода Yout
        MOV     R6,A
        MOV     A,TH0
        MOV     R7,A

; сохраняем замер в буфере
        MOV     A,R0
        MOV     TL0,A
        MOV     A,R1
        MOV     TH0,A           ; сохраним 'R0|R1'
        MOV     R0,#buff?040
; начинаем освобождать 'R0-R7', размещая в буфере
        MOV     A,TL0
        MOV     @R0,A           ; 'R0'
        INC     R0
        MOV     A,TH0
        MOV     @R0,A           ; 'R1'
        INC     R0
        MOV     A,R2
        MOV     @R0,A           ; 'R2'
        INC     R0
        MOV     A,R3
        MOV     @R0,A           ; 'R3'
        INC     R0
        MOV     A,R4
        MOV     @R0,A           ; 'R4'
        INC     R0
        MOV     A,R5
        MOV     @R0,A           ; 'R5'
        INC     R0
        MOV     A,R6
        MOV     @R0,A           ; 'R6'
        INC     R0
        MOV     A,R7
        MOV     @R0,A           ; 'R7'

        MOV     R7,#buff?040

; }
        RET

        END

