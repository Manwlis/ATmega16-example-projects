;
; loop\main.asm
; Change the 0 bit of PortB every 1ms using time loop.
;
; Author : Emmanouil Petrakos
; Created: 4/10/2020
; Developed with AtmelStudio 7.0.129
;

; loop iterations
.equ limit = 2498

; initialization routine. Runs once at program start.
init:
	; set port bit 0 as output
	sbi DDRB,0			; 2 cycle

; Subsequent iterations have to pay 2 extra cycle from the final rjmp. Start them here in order to keep them time alligned with the first.
start:
	; mask used to flip only bit 0
	clr R17				; 1 cycle
	sbr R17,1			; 1 cycle

	; Instructions for delay
	; Set limit. Limit > 256, register pair is needed
	ldi R25,HIGH(limit)	; 1 cycle
	ldi R24,LOW(limit)	; 1 cycle
loop:
	; Decrement register pair till zero
	sbiw R24,1			; 2 cycle
	brne loop			; 2 cycle if true, 1 if false

	; Instructions needed to change output
	; eor PortB with mask. Bit 0 flips, the rest are unafected.
	in r16,PORTB		; 1 cycle
	eor R16, R17		; 1 cycle
	; change output
	out PORTB, R16		; 1 cycle
	rjmp start			; 2 cycle