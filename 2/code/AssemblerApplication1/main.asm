;
; main.asm
; TO_DO titlo kai perigrafh
;
; Created: 13/10/2020
; Author : Emmanouil Petrakos
; Developed with AtmelStudio 7.0.129
;

//--------------------------------------------------------------------
// temp regs: r16
// routine/interrupt param/return regs: r0
// pointer regs: r26, r27
//--------------------------------------------------------------------

.equ OCR_value = 155 ; Compare register value
.equ data_pointer = 0x0060 ; potision of SRAM used to store data
.equ num_of_data = 8 ;
.equ offset = 0 ; value of first data

; Reset and Interrupt vectors
.org $000
rjmp init ; Start from initialization routine
.org $026
rjmp ISR_TC0

; code segment
.org $02A
//--------------------------------------------------------------------
// Initialization routine
// Runs once at program startup. Sets SP, RAM, ports and interrupts
//--------------------------------------------------------------------
init:
	; Set stack pointer so program returns from interrupt where it occurred
	ldi r16, HIGH( RAMEND ) ; Upper byte
	out SPH,r16 ; to stack pointer
	ldi r16, LOW( RAMEND ) ; Lower byte
	out SPL,r16 ; to stack pointer

	; Set memory pointer 
	ldi r27, HIGH( data_pointer ) ; pointer registers X (r26:r27)
    ldi r26, LOW( data_pointer )
// Save in SRAM the data that are going to be shown 
// first_pos_value = offset
// next_pos_value = prev_pos_value + 1
// num_of_data times
//---
	ldi r16, offset ; starting value
	set_data_loop:
		st X+, r16 ; store to SRAM / post increment
		inc r16	
		cpi r16, offset + num_of_data
		brne set_data_loop
//---

	; Set Ports A and C as outputs and init
	ldi r16, 0xFF
	out DDRA, R16
	out DDRC, R16
	out PORTA, R16 ; 1 is off
	clr r16
	out PORTC, R16

	; Set Timer0 at ~1ms
	ldi r16,(1<<CS01)|(1<<CS00)|(1<<WGM01)
	out TCCR0,r16 ; Timer clock = system clock / 64. Clear counter on match.
	ldi r16,1<<TOV0
	out TIFR,r16 ; Clear TOV0/ clear pending interrupts
	ldi r16,1<<OCIE0
	out TIMSK,r16 ; Enable Timer/Counter0 compare interrupt
	ldi r16,OCR_value
	out OCR0,r16 ; Set Compare Register

	; Enable interrupts
	sei

// Infinite loop.
loop: rjmp loop 


//--------------------------------------------------------------------
// Interrupt service routine for timer/counter0 compare match mode.
// Controls 7 segments outputs.
// arguments: none
// returns: none
// changes: r0, r16
//--------------------------------------------------------------------
ISR_TC0:
; TO_DO

	; show nothing
	ldi r16, 0xFF
	out PORTA, R16
	
	in r0, PORTC ; get currenty enabled 7 seg 
	rcall circular_counter ; calculate next 7 seg
	out PORTC, r0 ; disable enabled 7 seg and enable next

	;read from mem based on r0 value

	; decode, invert and put output
	rcall BCD_to_7_segment_Decoder
	com r0 ; 0 = on, 1 = off. Need to invert
	out PORTA, r0

	reti ; clears interrupt flag


//--------------------------------------------------------------------
// Circular shift to the left.
// arguments: input in r0
// returns: shifted output in r0
// changes: r0, r16
//--------------------------------------------------------------------
circular_counter:
	clr r16 ; empty reg
	lsl r0 ; Logical Shift Left / bit 0 cleared, bit 7 to carry
	adc r0, r16 ; Add with carry and empty reg / carry to bit 0
	ret


//--------------------------------------------------------------------
// Decodes a BCD number (0-9) to 7 segment format
// arguments: bcd number in r0
// returns: 7 segment number in r0
// changes: r0, r16
//--------------------------------------------------------------------
BCD_to_7_segment_Decoder:
; TO_DO

	
	ret