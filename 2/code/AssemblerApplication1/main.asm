;
; main.asm
; TO_DO titlo kai perigrafh
;
; Created: 13/10/2020
; Author : Emmanouil Petrakos
; Developed with AtmelStudio 7.0.129
;

//--------------------------------------------------------------------
// temp regs: r16, r17, r18
// subroutine/interrupt param/return regs: r0, r1
// pointer regs: r26, r27, r28, r29
//--------------------------------------------------------------------

.equ OCR_value = 155 ; Compare register value
.equ data_pointer = 0x0060 ; position of SRAM used to store data
.equ segments_pointer = 0x0068 ; position of SRAM used to store 7 segments encodings
.equ num_of_data = 8 ;
.equ offset = 1 ; value of first data

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

	; save encoding
	call write_7_segments
	; set output
	call write_data

	; Set Ports A and C as outputs and init
	ldi r16, 0xFF
	out DDRA, R16
	out DDRC, R16
	out PORTA, R16 ; all segments off
	ldi r16, 0b10000000 ; in order to start from rightmost 7 segment
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
// Save in SRAM numbers 0-9 in 7 segment format.
// arguments: none
// returns: none
// changes: r16, r28, r29
//--------------------------------------------------------------------
write_7_segments:
	; Set memory pointer 
	ldi r29, HIGH( segments_pointer ) ; pointer registers Y (r28:r29)
    ldi r28, LOW( segments_pointer ) ; index X doesn't support displacement

	; save encodings
	ldi r16, 0b00000011
	std Y+0, r16

	ldi r16, 0b10011111
	std Y+1, r16

	ldi r16, 0b00100101
	std Y+2, r16

	ldi r16, 0b00001101
	std Y+3, r16

	ldi r16, 0b10011001
	std Y+4, r16

	ldi r16, 0b01001001
	std Y+5, r16

	ldi r16, 0b01000001
	std Y+6, r16

	ldi r16, 0b00011111
	std Y+7, r16

	ldi r16, 0b00000001
	std Y+8, r16
	
	ldi r16, 0b00001001
	std Y+9, r16

	ret


//--------------------------------------------------------------------
// Save in SRAM the data that are going to be shown. LSB to MSB
// first_pos_value = offset
// next_pos_value = prev_pos_value + 1
// num_of_data times
// arguments: none
// returns: none
// changes: r16, r26, r27
//--------------------------------------------------------------------
write_data:
	; Set memory pointer 
	ldi r27, HIGH( data_pointer ) ; pointer registers X (r26:r27)
    ldi r26, LOW( data_pointer )
	ldi r16, offset ; starting value
	set_data_loop:
		st X+, r16 ; store to SRAM / post increment
		inc r16	
		cpi r16, offset + num_of_data
		brne set_data_loop
	ret


//--------------------------------------------------------------------
// Interrupt service routine for timer/counter0 compare match mode.
// Controls 7 segments outputs.
// arguments: none
// returns: none
// changes: r0, r1, r16, r26, r27
//--------------------------------------------------------------------
ISR_TC0:
	; show nothing
	ldi r16, 0xFF
	out PORTA, R16
	
	in r0, PORTC ; get currenty enabled 7 seg
	rcall ring_counter ; calculate next 7 seg
	out PORTC, r0 ; disable enabled 7 seg and enable next

	; read from mem based using ring counter value as offset
	rcall ring_to_bcd
	mov r16, r1
	dec r16 ; 1st number is in pos X+0, 2nd number is in pos X+1, etc.

	ldi r27, HIGH( data_pointer ) ; pointer registers X (r26:r27)
    ldi r26, LOW( data_pointer )
	; add offset to mem pointer registers.
	clr r17 ; empty reg
	add r26, r16
	adc r27, r17

	ld r0, X ; read data in bcd format

	rcall BCD_to_7_segment_Decoder ; decode
	out PORTA, r1 ; put output

	reti ; clears interrupt flag


//--------------------------------------------------------------------
// Circular shift to the left. ex: 00001000 -> 00010000
// arguments: input in r0
// returns: shifted output in r0
// changes: r0, r16
//--------------------------------------------------------------------
ring_counter:
	clr r16 ; empty reg
	lsl r0 ; Logical Shift Left / bit 0 cleared, bit 7 to carry
	adc r0, r16 ; Add with carry and empty reg / carry to bit 0
	ret


//--------------------------------------------------------------------
// Transform a ring number to bcd. ex: 01000000 -> 00000111
// arguments: ring format number in r0
// returns: bcd number in r1
// changes: r0, r1, r16
//--------------------------------------------------------------------
ring_to_bcd:
	clr r1
	mov r16, r0
; bcd number = num of shift that are needed to clear ring number.
rtb_loop:
	inc r1
	lsr r16
	brne rtb_loop
	ret


//--------------------------------------------------------------------
// Decodes a BCD number (0-9) to 7 segment format. ex: 00000010 -> 00100101
// 7 segment is in common cathode format.
// arguments: bcd number in r0
// returns: 7 segment number in r1
// changes: r1, r16, r17, r18, r26, r27
//--------------------------------------------------------------------
BCD_to_7_segment_Decoder:
	; check if data can be translated in 7 segment format
	mov r16,r0
	ldi r17, 0b11111111 ; no segment enabled
	cpi r16, 10
	brsh bcd_to_7seg_exit ; if data >= 10 (unsigned), skip

	; get 7 segment format
	ldi r27, HIGH( segments_pointer ) ; pointer registers X (r26:r27)
    ldi r26, LOW( segments_pointer )
	; add offset to mem pointer registers.
	clr r18 ; empty reg
	add r26, r16
	adc r27, r18

	ld r17, X ; read 7 segment number

bcd_to_7seg_exit:
	mov r1, r17
	ret