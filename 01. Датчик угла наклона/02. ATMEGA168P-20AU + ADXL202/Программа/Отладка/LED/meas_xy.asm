;********************************************************************************
;*                                                                              *
;*                        ћигание светодиода дл€ 'ATMEGA168P'                   *
;*                               (22 окт€бр€ 2009 г)                            *
;*                                                                              *
;********************************************************************************

.NOLIST                            ; исключение последующего текста из файла листинга
.INCLUDE "m168def.inc"             ; подключить файл описаний имен регистров ввода/вывода
.LIST                              ; включение последующего текста в файл листинга

.equ F_CPU = 11059200              ; частота резонатора
.def temp = r16                    ; дл€ временного хранени€

; инициализаци€ порта B
.equ LED = PORTB0                  ; вывод управлени€ сетодиодным индикатором  OUT (1)
.equ STR = PORTB2                  ; вывод управлени€ сторожевым таймером      OUT (1)

.CSEG                              ; начало программного сегмента

        rjmp    START              ; старт при сбросе

.ORG    OVF0addr                   ; адрес вектора прерывани€ по переполнению таймера 0
                                   ; (им€ "OVF0addr" определено в файле "m168def.inc")
START:
        ldi     temp,low(RAMEND)   ; инициализаци€ указател€ стека путем записи  
        out     SPL,temp               ; в него значени€ верхнего адреса пам€ти данных
        ldi     temp,high(RAMEND)  ; ("микроконтроллеры AVR")
        out     SPH,temp           ; (им€ "RAMEND" определено в файле "m168def.inc") 

; инициализаци€ »—ѕќЋ№«”≈ћџ’ портов ввода/вывода
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
        rjmp    MAIN               ; на следущее измерение
WD0:    cbi     PORTB,STR          ; STR->'0'
        rjmp    MAIN               ; на следущее измерение
