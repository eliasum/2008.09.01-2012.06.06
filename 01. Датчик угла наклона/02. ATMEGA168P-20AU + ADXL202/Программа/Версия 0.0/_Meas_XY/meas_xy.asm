;********************************************************************************
;*                                                                              *
;*                     Управляющая программа для 'ATMEGA168P'                   *
;*                         Версия 0.0 (16 октября 2009 г)                       *
;*                                                                              *
;********************************************************************************

.NOLIST                            ; исключение последующего текста из файла листинга
.INCLUDE "m168def.inc"             ; подключить файл описаний имен регистров ввода/вывода
.LIST                              ; включение последующего текста в файл листинга

.equ F_CPU = 11059200              ; частота резонатора
.equ BAUDRATE = 2400               ; скорость обмена по LINE

; константа перезагрузки Tim0 (2400 Гц)
.equ CTINT = 256-((F_CPU/64)/BAUDRATE) 

.def temp = r16                    ; для временного хранения
.def temp2 = r17                   ; для временного хранения 2 
.def temp3 = r18                   ; для временного хранения 2 
.def v_cnt = r19                   ; счетчик интервалов времени передачи 
                                   ; одного бита по линии обмена с платой датчика

; инициализация порта D

; PINх - регистр входных данных, обеспечивает возможность только чтения 
; PORTx - регистр порта данных, обеспечивает возможность чтения/записи
; DDRx - регистр направления данных, обеспечивает возможность чтения/записи

.equ RXD = PIND0                   ; вывод входа приема данных ответа          IN  (^)
.equ TXD = PORTD1                  ; вывод выхода данных по запросу            OUT (1)
.equ TMPR_WR = PORTD2              ; вывод запроса данных о температуре        OUT (1)
.equ TMPR_RD = PIND2               ; вывод входа приема данных о температуре   IN  (^)
.equ XOUT = PIND3                  ; вывод входа приема данных по 'X'          IN  (^)
.equ YOUT = PIND4                  ; вывод входа приема данных по 'Y'          IN  (^)
.equ SET_ = PIND5                  ; вывод входа конфигурации установок        IN  (^)
.equ KLINE = PORTD6                ; вывод управления разрешения чипа 'K-LINE' OUT (1)
                                   ; "разрешение приема = 1"     
.equ RS485 = PORTD7                ; вывод управления разрешения чипа 'RS485'  OUT (1)
                                   ; "разрешение приема = 0"

; инициализация порта B
.equ LED = PORTB0                  ; вывод управления сетодиодным индикатором  OUT (1)
.equ DIR = PORTB1                  ; вывод управления направлением 'RS485'     OUT (0) 
                                   ; "разрешение передачи = 1"
.equ STR = PORTB2                  ; вывод управления сторожевым таймером      OUT (1)

.CSEG                              ; начало программного сегмента

        rjmp    START              ; старт при сбросе

.ORG    OVF0addr                   ; адрес вектора прерывания по переполнению таймера 0
                                   ; (имя "OVF0addr" определено в файле "m168def.inc")
                                                                          
;TIMER0_OVF_ISR:                    ; старт прерывания по Tim0
;        ldi     temp2,CTINT
;        out     TCNT0,temp2        ; константа перезагрузки (2400 Гц)
;        mov     temp2,v_cnt
;        tst     temp2
;        breq    TIMER0_OVF_1
;        dec     v_cnt

;TIMER0_OVF_1:    
;        reti

START:
        ldi     temp,low(RAMEND)   ; инициализация указателя стека путем записи  
        out     SPL,temp               ; в него значения верхнего адреса памяти данных
        ldi     temp,high(RAMEND)  ; ("микроконтроллеры AVR")
        out     SPH,temp           ; (имя "RAMEND" определено в файле "m168def.inc") 

; инициализация USART (9600)
        ldi     temp,0x00            
        sts     UCSR0A,temp        ; запрет на время установки скорости        
        sts     UCSR0B,temp        ; передачи настроек USART
        ldi     temp,0x06           
        sts     UCSR0C,temp        ; размер слова данных 8 бит
        ldi     temp,0x47           
        sts     UBRR0L,temp        ; задание скорости 9600 бод
        ldi     temp,0x00           
        sts     UBRR0H,temp
        ldi     temp,0x08          
        sts     UCSR0B,temp        ; разрешение передачи

; инициализация ИСПОЛЬЗУЕМЫХ портов ввода/вывода
        ldi     temp,(1<<RS485)|(1<<KLINE)|(1<<SET_)|(1<<YOUT)|(1<<XOUT)|(1<<TMPR_RD)|(1<<TXD)|(1<<RXD)
        out     PORTD,temp
        ldi     temp,(1<<RS485)|(1<<KLINE)|(1<<TXD)
        out     DDRD,temp
        ldi     temp,(1<<STR)|(1<<LED)
        out     PORTB,temp
        ldi     temp,(1<<STR)|(1<<DIR)|(1<<LED)
        out     DDRB,temp

; инициализация TIMER0
;        ldi     temp,CTINT
;        out     TCNT0,temp         ; константа перезагрузки (2400 Гц)
;        in      temp,TCCR0B
;        ori     temp,(1<<CS01)|(1<<CS00)
;        out     TCCR0B,temp        ; старт Tim0 [CLK_io/64]
;        lds     temp,TIMSK0
;        ori     temp,(1<<TOIE0)
;        sts     TIMSK0,temp        ; разрешение прер-я по переполнению Tim0

; У-во ведомое или ведущее ?
;        sbis    PIND,SET_          ; не равен '1', если у-во ведомое
 ;       rjmp    SETUP

MAIN: 

; настройка Т/С1
        ldi     temp,0x00          ; приготовимся к останову Т/С1
        sts     TCCR1B,temp        ; останов Т/С1
        sts     TCNT1H,temp
        sts     TCNT1L,temp        ; сброс Т/С1
        ldi     temp2,0x02        ; приготовим делитель на '8'

; измерение длительности и периода импульсов с выходов Xout и Yout
X_K1:   sbic    PIND,XOUT          ; корректировка расчета длительности импульсов с выхода Xout
        rjmp    X_K1

X_F:    sbis    PIND,XOUT          ; ожидание фронта 1-го импульса с выхода Xout        
        rjmp    X_F
        sts     TCCR1B,temp2       ; запуск Т/С1 с тактовой частотой CLK_io/8

X_S:    sbic    PIND,XOUT          ; ожидание спада 1-го импульса с выхода Xout
        rjmp    X_S
        sts     TCCR1B,temp        ; останов Т/С1

        lds     R0,TCNT1L
        lds     R1,TCNT1H          ; в R1R0 - значение длительности импульсов с выхода Xout

        sts     TCNT1H,temp
        sts     TCNT1L,temp        ; сброс Т/С1

X_K2:   sbic    PIND,XOUT          ; корректировка расчета периода импульсов с выхода Xout
        rjmp    X_K2

X_F2:   sbis    PIND,XOUT          ; ожидание фронта 2-го импульса с выхода Xout        
        rjmp    X_F2
        sts     TCCR1B,temp2       ; запуск Т/С1 с тактовой частотой CLK_io/8

X_S2:   sbic    PIND,XOUT          ; ожидание спада 2-го импульса с выхода Xout
        rjmp    X_S2
X_F3:   sbis    PIND,XOUT          ; ожидание фронта 3-го импульса с выхода Xout        
        rjmp    X_F3
        sts     TCCR1B,temp        ; останов Т/С1

        lds     R2,TCNT1L
        lds     R3,TCNT1H          ; в R2R3 - значение периода импульсов с выхода Xout

        sts     TCNT1H,temp
        sts     TCNT1L,temp        ; сброс Т/С1

Y_K3:   sbic    PIND,YOUT          ; корректировка расчета длительности импульсов с выхода Yout
        rjmp    Y_K3

Y_F:    sbis    PIND,YOUT          ; ожидание фронта 1-го импульса с выхода Yout        
        rjmp    Y_F
        sts     TCCR1B,temp2       ; запуск Т/С1 с тактовой частотой CLK_io/8

Y_S:    sbic    PIND,YOUT          ; ожидание спада 1-го импульса с выхода Yout
        rjmp    Y_S
        sts     TCCR1B,temp        ; останов Т/С1

        lds     R4,TCNT1L
        lds     R5,TCNT1H          ; в R4R5 - значение длительности импульсов с выхода Yout

        sts     TCNT1H,temp
        sts     TCNT1L,temp        ; сброс Т/С1

Y_K4:   sbic    PIND,YOUT          ; корректировка расчета периода импульсов с выхода Yout
        rjmp    Y_K4

Y_F2:   sbis    PIND,YOUT          ; ожидание фронта 2-го импульса с выхода Yout        
        rjmp    Y_F2
        sts     TCCR1B,temp2       ; запуск Т/С1 с тактовой частотой CLK_io/8

Y_S2:   sbic    PIND,YOUT          ; ожидание спада 2-го импульса с выхода Yout
        rjmp    Y_S2
Y_F3:   sbis    PIND,YOUT          ; ожидание фронта 3-го импульса с выхода Yout        
        rjmp    Y_F3
        sts     TCCR1B,temp        ; останов Т/С1

        lds     R6,TCNT1L
        lds     R7,TCNT1H          ; в R6R7 - значение периода импульсов с выхода Yout

; посылка результатов замеров:
        ldi     temp,0x5A          ; метка начала блока данных
        mov     temp2,temp         ; начальное значение контрольной суммы

        call    TX_BYTE            ; передача начального значения контрольной суммы

        mov     temp,R0  
        call    TX_SUM             ; передача содержимого регистра R0
         
        mov     temp,R1  
        call    TX_SUM             ; передача содержимого регистра R1
         
        mov     temp,R2  
        call    TX_SUM             ; передача содержимого регистра R2
          
        mov     temp,R3  
        call    TX_SUM             ; передача содержимого регистра R3
         
        mov     temp,R4  
        call    TX_SUM             ; передача содержимого регистра R4
          
        mov     temp,R5  
        call    TX_SUM             ; передача содержимого регистра R5
          
        mov     temp,R6  
        call    TX_SUM             ; передача содержимого регистра R6
         
        mov     temp,R7  
        call    TX_BYTE            ; передача содержимого регистра R7

        add     temp,temp2              
        call    TX_BYTE            ; передача конечного значения контрольной суммы

        sbic    PINB,STR           ; STR=0 ?
        rjmp    WD0
        sbi     PORTB,STR          ; STR->'1'
        rjmp    MAIN               ; на следущее измерение
WD0:    cbi     PORTB,STR          ; STR->'0'
        rjmp    MAIN               ; на следущее измерение

; передача байта через 'USART'
TX_BYTE:
        lds     temp3,UCSR0A
        sbrs    temp3,UDRE0        ; ждать очистки буфера передатчика
        rjmp    TX_BYTE
        sts     UDR0,temp          ; загрузить младший байт данных  
                                   ; в буфер, начать передачу
        ret                     

;        А я делаю обычно так:
;TX_BYTE:
;        sts     UDR0,temp          ; передать байт данных  
;        lds     temp3,UCSR0A
;TXB2:   sbrs    temp3,TXC0         ; ждать конца передачи
;        rjmp    TXB2
;
;        ret                     

; передача байта и коррекция контрольной суммы
TX_SUM:      
        call    TX_BYTE
        add     temp2,temp      
        ret
      
; переход в режим установок
;SETUP:
;        ret

;.NOLIST

; ----------------------------------------------------------
; BAUD = 9600
; BAUD = fck/16*(UBRR+1)
; UBRR = [fck/16*BAUD]-1 = 0x47 = 0b01000111
; UBRR0L = 0x47 = 0b01000111
; UBRR0H = 0x00 = 0b00000000
; ----------------------------------------------------------
