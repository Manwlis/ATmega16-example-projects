;
; main.asm
; Change the 0 bit of PortB every 1ms using timer 0 interupts.
;
; Author : Emmanouil Petrakos
; Created: 6/10/2020
; Developed with AtmelStudio 7.0.129
;

.org $000
rjmp init
.org $012
rjmp ISR_TOV0

.org $02A
init:
	; Set stack pointer so program returns from interrupt where it occured
	ldi r16, HIGH(RAMEND) ; Upper byte
	out SPH,r16 ; to stack pointer
	ldi r16, LOW(RAMEND) ; Lower byte
	out SPL,r16 ; to stack pointer

	; Set Timer0
	ldi r16,(1<<CS02)|(1<<CS00)
	out TCCR0,r16 ; Timer clock = system clock / 1024
	ldi r16,1<<TOV0
	out TIFR,r16 ; Clear TOV0/ clear pending interrupts
	ldi r16,1<<TOIE0
	out TIMSK,r16 ; Enable Timer/Counter0 Overflow Interrupt
	ser r16

	; Used to calculate output
	clr R17
	sbr R17,1 ; set bit 0

	; Set Port B as output
	out DDRB,r16

	; Enable interrupts
	sei

loop: rjmp loop ; Infinite loop


ISR_TOV0:
	in r16,PORTB ; Read Port B
	eor r16, r17 ; Invert bit 0 of r16 register
	out PORTB,r16 ; Write Port B
	reti