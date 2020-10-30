;
; 7_segment_driver.asm
; Driver for an 8 digits 7 segment display. Shows data from SRAM.
;
; Created: 13/10/2020
; Author : Emmanouil Petrakos
; Developed with AtmelStudio 7.0.129
;

#include <avr/io.h>
#include "program.h"

; defined in C code
.extern data 
.extern segments_encoding


//--------------------------------------------------------------------
// Interrupt service routine for timer/counter0 compare match mode.
// Controls 7 segments outputs.
// arguments: none
// returns: none
// changes: r0, r16, r18, r19, r26, r27, r31
//--------------------------------------------------------------------
.global TIMER0_COMP_vect
TIMER0_COMP_vect:
	; show nothing
	ldi r18, 0xFF
	out _SFR_IO_ADDR( PORTA ), R18
	
	in r0, _SFR_IO_ADDR( PORTC ) ; get currenty enabled 7 seg
	rcall ring_counter ; calculate next 7 seg
	out _SFR_IO_ADDR( PORTC ), r0 ; disable enabled 7 seg and enable next

	; read from mem based using ring counter value as offset
	rcall ring_to_bcd
	mov r18, r31
	dec r18 ; 1st number is in pos X+0, 2nd number is in pos X+1, etc.

	ldi r27, hi8( data ) ; pointer registers X (r26:r27)
    ldi r26, lo8( data )
	; add offset to mem pointer registers.
	add r26, r18
	adc r27, r1	; r1 is always empty

	ld r0, X ; read data in bcd format

	rcall BCD_to_7_segment_Decoder ; decode
	out _SFR_IO_ADDR( PORTA ), r31 ; put output

	reti ; clears interrupt flag


//--------------------------------------------------------------------
// Circular shift to the left. ex: 00001000 -> 00010000
// arguments: input in r0
// returns: shifted output in r0
// changes: r0, r18
//--------------------------------------------------------------------
ring_counter:
	clr r18 ; empty reg
	lsl r0 ; Logical Shift Left / bit 0 cleared, bit 7 to carry
	adc r0, r18 ; Add with carry and empty reg / carry to bit 0

	ret


//--------------------------------------------------------------------
// Transform a ring number to bcd. ex: 01000000 -> 00000111
// arguments: ring format number in r0
// returns: bcd number in r31
// changes: r18, r31
//--------------------------------------------------------------------
ring_to_bcd:
	clr r31
	mov r18, r0
; bcd number = num of shift that are needed to clear ring number.
rtb_loop:
	inc r31
	lsr r18
	brne rtb_loop

	ret


//--------------------------------------------------------------------
// Decodes a BCD number (0-9) to 7 segment format. ex: 00000010 -> 00100101
// 7 segment is in common cathode format.
// arguments: bcd number in r0
// returns: 7 segment number in r31
// changes: r1, r18, r26, r27
//--------------------------------------------------------------------
BCD_to_7_segment_Decoder:
	; check if data can be translated in 7 segment format
	mov r18,r0
	ldi r19, 0b11111111 ; no segment enabled
	cpi r18, 0x0B
	brsh bcd_to_7seg_exit ; if data >= 0x0B (unsigned), skip

	; get 7 segment format
	ldi r27, hi8( segments_encoding ) ; pointer registers X (r26:r27)
    ldi r26, lo8( segments_encoding )
	; add offset to mem pointer registers.
	add r26, r18
	adc r27, r1	; r1 is always empty

	ld r19, X ; read 7 segment number

bcd_to_7seg_exit:
	mov r31, r19
	ret


//--------------------------------------------------------------------
// Saves a bcd in the first position of drivers data mem.
// Moves all old data one position forward. Last element gets discarded.
// arguments: input character in r0
// returns: none
// changes: r18, r19, r26, r27
//--------------------------------------------------------------------
.global save_to_7seg_data
save_to_7seg_data:

	ldi r27, hi8( data ) ; pointer registers X (r26:r27)
    ldi r26, lo8( data )
	adiw X, num_of_data - 2 ; pos of the second from last element.
	ldi r18, num_of_data - 1 ; loop iterations. -1 because the last element will be discarded.

	; Move all pre-existing data one position up. Top one gets discarded.
move_up_loop:
	ld r19, X+ ; Read element. Post increment to get the pos where it will be stored.
	st X, r19 ; Save it.
	sbiw X, 2 ; Next element.
	
	dec r18
	brne move_up_loop

	ldi r27, hi8( data ) ; new data get saved in first position
    ldi r26, lo8( data )
	st X, r0

	ret


//--------------------------------------------------------------------
// Initialize the SRAM where the data is stored.
// arguments: none
// returns: none
// changes: r18, r19, r26, r27
//--------------------------------------------------------------------
.global clr_7seg_data
clr_7seg_data:
	; Set memory pointer 
	ldi r27, hi8( data ) ; pointer registers X (r26:r27)
    ldi r26, lo8( data )
	clr r18 ; empty reg
	ldi r19, 0x0A
	set_data_loop:
		st X+, r19 ; store to SRAM / post increment
		inc r18
		cpi r18, num_of_data
		brne set_data_loop
	ret