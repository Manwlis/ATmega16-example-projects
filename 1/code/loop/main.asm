;
; lab1.asm
; Change output periodically using loop
; Created: 4/10/2020
; Author : Emmanouil Petrakos
;

; loop iterations
.equ limit = 199

; used to track loop iterations
clr R31 ; clear register 

; used to change output
cbr R16,1 ; clear bit 0
sbr R17,1 ; set bit 0

; set port bit 0 as output
sbi DDRB,0

loop:
	nop				; 1 cycle
	inc R31			; 1 cycle
	cpi R31, limit	; 1 cycle
	brne loop		; 2 cycle

	nop
	; clear register
	clr R31			; 1 cycle
	; flip R16
	eor R16, R17	; 1 cycle
	; change output
	out PORTB, R16	; 1 cycle
	rjmp loop		; 2 cycle


