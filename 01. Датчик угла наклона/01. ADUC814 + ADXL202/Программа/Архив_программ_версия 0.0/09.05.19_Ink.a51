;                       юдпеяю х йнмярюмрш
;                            *****
;
;
;
R15             .EQU    15      ;юдпеяю пецхярпнб R0-R15     
R14             .EQU    14           
R13             .EQU    13           
R12             .EQU    12           
R11             .EQU    11           
R10             .EQU    10           
R9              .EQU    9            
R8              .EQU    8            
R7              .EQU    7       
R6              .EQU    6
R5              .EQU    5
R4              .EQU    4
R3              .EQU    3
R2              .EQU    2
R1              .EQU    1
R0              .EQU    0
ACC             .EQU    0E0H    ;юдпея юййслскърнпю
B               .EQU    0F0H    ;юдпея пецхярпю б
PSW             .EQU    0D0H    ;юдпея пецхярпю (якнбю) янярнъмхъ
SP              .EQU    81H     ;юдпея сйюгюрекъ ярейю
DPL             .EQU    82H     ;юдпея лкюдьеи онкнбхмш DPTR
DPH             .EQU    83H     ;юдпея ярюпьеи онкнбхмш DPTR
P0              .EQU    80H     ;юдпея пецхярпю онпрю п0
P1              .EQU    90H     ;юдпея пецхярпю онпрю п1
P2              .EQU    0A0H    ;юдпея пецхярпю онпрю п2
P3              .EQU    0B0H    ;юдпея пецхярпю онпрю п3
B.0             .EQU    0F0H    ;юдпеяю нрдекэмшу ахрнб пецхярпю б
B.1             .EQU    0F1H
B.2             .EQU    0F2H
B.3             .EQU    0F3H
B.4             .EQU    0F4H
B.5             .EQU    0F5H
B.6             .EQU    0F6H
B.7             .EQU    0F7H
ACC.0           .EQU    0E0H    ;юдпеяю нрдекэмшу ахрнб юййслскърнпю
ACC.1           .EQU    0E1H
ACC.2           .EQU    0E2H
ACC.3           .EQU    0E3H
ACC.4           .EQU    0E4H
ACC.5           .EQU    0E5H
ACC.6           .EQU    0E6H
ACC.7           .EQU    0E7H
PSW.0           .EQU    0D0H    ;юдпеяю нрдекэмшу ахрнб пецхярпю PSW
PSW.1           .EQU    0D1H
PSW.2           .EQU    0D2H
PSW.3           .EQU    0D3H
PSW.4           .EQU    0D4H
PSW.5           .EQU    0D5H
PSW.6           .EQU    0D6H
PSW.7           .EQU    0D7H
P0.0            .EQU    080H    ;юдпеяю нрдекэмшу кхмхи онпрю п0
P0.1            .EQU    081H
P0.2            .EQU    082H
P0.3            .EQU    083H
P0.4            .EQU    084H
P0.5            .EQU    085H
P0.6            .EQU    086H
P0.7            .EQU    087H
P1.0            .EQU    090H    ;юдпеяю нрдекэмшу кхмхи онпрю п1
P1.1            .EQU    091H
P1.2            .EQU    092H
P1.3            .EQU    093H
P1.4            .EQU    094H
P1.5            .EQU    095H
P1.6            .EQU    096H
P1.7            .EQU    097H
P2.0            .EQU    0A0H    ;юдпеяю нрдекэмшу кхмхи онпрю п2
P2.1            .EQU    0A1H
P2.2            .EQU    0A2H
P2.3            .EQU    0A3H
P2.4            .EQU    0A4H
P2.5            .EQU    0A5H
P2.6            .EQU    0A6H
P2.7            .EQU    0A7H
P3.0            .EQU    0B0H    ;юдпеяю нрдекэмшу кхмхи онпрю п3
P3.1            .EQU    0B1H
P3.2            .EQU    0B2H
P3.3            .EQU    0B3H
P3.4            .EQU    0B4H
P3.5            .EQU    0B5H
P3.6            .EQU    0B6H
P3.7            .EQU    0B7H
;
TMOD            .EQU    089H    ;REGISTRY TAJMERA-SCHETCHIKA,
TCON            .EQU    088H    ;REGISTRY UPRAVLENIJA T/S I  
TL0             .EQU    08AH    ;PRERYVANIJAMI
TH0             .EQU    08CH
TH1             .EQU    08BH
TL1             .EQU    08DH
IE              .EQU    0A8H
IP              .EQU    0B8H
TR0             .EQU    08CH
TR1             .EQU    08EH
ET0             .EQU    0A9H
EA              .EQU    0AFH
;
        .ORG  0                 ;мхфеякедсчыюъ йнлюмдю я юдпеяю 0
;
        LJMP    TIMER           ;мю йнлюмдс оняке лерйх TIMER
;
        .ORG    100H            ;мхфеякедсчыюъ йнлюмдю я юдпеяю 100м
;
TIMER:                          ;опнцпюллю
        MOV IE,#0               ;VSE PRERYVANIJA ZAPRESCHENY
        MOV TMOD,#01010101B     ;T0 I T1 - 16-BITNYE T/S
        MOV TH0,#0              ;SBROS TAJMEROV T0 I T1
        MOV TL0,#0
        MOV TH1,#0
        MOV TL1,#0

X_F:    JNB P1.0,X_F            ;OZHIDANIE FRONTA 1-GO IMPULSA S VYHODA XOUT
        SETB TR0                ;ZAPUSK TAJMERA TO

X_S:    JB P1.0,X_S             ;OZHIDANIE SPADA 1-GO IMPULSA S VYHODA XOUT
        MOV A,TL0               ;V R2R1 - ZNACHENIE SPADA 1-GO IMPULSA S VYHODA XOUT
        MOV R1,A
        MOV A,TH0
        MOV R2,A

X_F2:   JNB P1.0,X_F2           ;OZHIDANIE FRONTA 2-GO IMPULSA S VYHODA XOUT
        MOV A,TL0               ;V R4R3 - ZNACHENIE FRONTA 2-GO IMPULSA S VYHODA XOUT
        MOV R3,A
        MOV A,TH0
        MOV R4,A
        CLR TR0

Y_F:    JNB P1.1,Y_F            ;OZHIDANIE FRONTA 1-GO IMPULSA S VYHODA YOUT
        SETB TR1                ;ZAPUSK TAJMERA T1

Y_S:    JB P1.1,Y_S             ;OZHIDANIE SPADA 1-GO IMPULSA S VYHODA YOUT
        MOV A,TL1               ;V R6R5 - ZNACHENIE SPADA 1-GO IMPULSA S VYHODA YOUT
        MOV R5,A
        MOV A,TH1
        MOV R6,A

Y_F2:   JNB P1.1,Y_F2           ;OZHIDANIE FRONTA 2-GO IMPULSA S VYHODA YOUT
        MOV A,TL1               ;V R8R7 - ZNACHENIE FRONTA 2-GO IMPULSA S VYHODA YOUT
        MOV R7,A
        MOV A,TH1
        MOV R8,A
        CLR TR1

        SJMP TIMER              ;ZACYKLIVANIE

        .END
