;
; interrupt\main.asm
; Change the 0 bit of PortB every 1ms using timer 0 interupts.
;
; Author : Emmanouil Petrakos
; Created: 6/10/2020
; Developed with AtmelStudio 7.0.129
;
.equ OCR_value = 155 ; Compare register value

.org $000
rjmp init ; Start from initialization routine
.org $026
rjmp ISR_TC0

.org $02A
; Initialization routine
init:
	; Set stack pointer so program returns from interrupt where it occurred
	ldi r16, HIGH(RAMEND) ; Upper byte
	out SPH,r16 ; to stack pointer
	ldi r16, LOW(RAMEND) ; Lower byte
	out SPL,r16 ; to stack pointer

	; Set Timer0
	ldi r16,(1<<CS01)|(1<<CS00)|(1<<WGM01)
	out TCCR0,r16 ; Timer clock = system clock / 64. Clear counter on match.
	ldi r16,1<<TOV0
	out TIFR,r16 ; Clear TOV0/ clear pending interrupts
	ldi r16,1<<OCIE0
	out TIMSK,r16 ; Enable Timer/Counter0 compare interrupt
	ldi r16,OCR_value
	out OCR0,r16 ; Set Compare Register

	; Mask used to calculate output
	clr R17
	sbr R17,1 ; set bit 0

	; Set Port B bit 0 as output
	sbi DDRB,0

	; Enable interrupts
	sei

loop: rjmp loop ; Infinite loop

; Interrupt service routine for timer/counter0 compare match mode
ISR_TC0:
	in r16,PORTB ; Read Port B
	eor r16, r17 ; Invert bit 0 of r16 register
	out PORTB,r16 ; Write Port B
	reti ; clears interrupt flag