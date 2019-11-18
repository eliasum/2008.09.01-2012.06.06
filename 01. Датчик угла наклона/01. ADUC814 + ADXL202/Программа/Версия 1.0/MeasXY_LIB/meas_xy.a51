$NOMOD51                        ; "���������" ����������� '51'
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
; =  ��������� 1-�� ������ 'X&Y' � 'ADXL202'                                  =
; = ------------------------------------------------------------------------- =
; =  ����:  ������                                                            =
; =  �����: R7 - ����� ������������ ������ ������� ������                     =
; =============================================================================
; char idata* meas_xy(void) {



        RSEG    ?PR?_meas_xy?MEAS_XY
        USING   0

meas_xy:

        MOV     TH0,#0          ; ����� T0 
        MOV     TL0,#0

; ��������� ������������ � ������� ��������� � ������� Xout � Yout

X_K1:   JB      XOUT,X_K1       ; ������������� ������� ������������ ��������� � ������ Xout

X_F:    JNB     XOUT,X_F        ; �������� ������ 1-�� �������� � ������ Xout      	
	    SETB    TR0             ; ������ T0
		
X_S:    JB      XOUT,X_S        ; �������� ����� 1-�� �������� � ������ Xout
	  	CLR     TR0             ; ������� T0                  

        MOV     A,TL0           ; � R1R0 - �������� ������������ ��������� � ������ Xout
        MOV     R0,A
        MOV     A,TH0
        MOV     R1,A

        MOV     TH0,#0          ; ����� T0
        MOV     TL0,#0

X_F2:   JNB     XOUT,X_F2       ; �������� ������ 2-�� �������� � ������ Xout
	    SETB    TR0             ; ������ T0

X_S2:   JB      XOUT,X_S2       ; �������� ����� 2-�� �������� � ������ Xout
X_F3:   JNB     XOUT,X_F3       ; �������� ������ 3-�� �������� � ������ Xout
		CLR     TR0             ; ������� T0

        MOV     A,TL0           ; � R3R2 - �������� ������� ��������� � ������ Xout
        MOV     R2,A
        MOV     A,TH0
        MOV     R3,A
              
        MOV     TH0,#0          ; ����� T0
        MOV     TL0,#0

Y_K2:   JB      YOUT,Y_K2       ; ������������� ������� ������������ ��������� � ������ Yout 

Y_F:    JNB     YOUT,Y_F        ; �������� ������ 1-�� �������� � ������ Yout      	
	    SETB    TR0             ; ������ T0
		
Y_S:    JB      YOUT,Y_S        ; �������� ����� 1-�� �������� � ������ Yout
        CLR     TR0             ; ������� T0                  

        MOV     A,TL0           ; � R5R4 - �������� ������������ ��������� � ������ Yout
        MOV     R4,A
        MOV     A,TH0
        MOV     R5,A

        MOV     TH0,#0          ; ����� T0
        MOV     TL0,#0

Y_F2:   JNB     YOUT,Y_F2       ; �������� ������ 2-�� �������� � ������ Yout
	    SETB    TR0             ; ������ T0

Y_S2:   JB      YOUT,Y_S2       ; �������� ����� 2-�� �������� � ������ Yout
Y_F3:   JNB     YOUT,Y_F3       ; �������� ������ 3-�� �������� � ������ Yout
        CLR     TR0             ; ������� T0

        MOV     A,TL0           ; � R7R6 - �������� ������� ��������� � ������ Yout
        MOV     R6,A
        MOV     A,TH0
        MOV     R7,A

; ��������� ����� � ������
        MOV     A,R0
        MOV     TL0,A
        MOV     A,R1
        MOV     TH0,A           ; �������� 'R0|R1'
        MOV     R0,#buff?040
; �������� ����������� 'R0-R7', �������� � ������
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

