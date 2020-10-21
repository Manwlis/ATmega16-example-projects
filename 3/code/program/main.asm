;
; main.asm
; Program that demonstrates the use of the 7 segment  and drivers.
;
; Created: 21/10/2020
; Author : Emmanouil Petrakos
; Developed with AtmelStudio 7.0.129
;


//--------------------------------------------------------------------
// temp regs: r16, r17, r18
// subroutine/interrupt param/return regs: r0, r1
// pointer regs: r26, r27, r28, r29
//--------------------------------------------------------------------


.equ data_pointer = 0x0060 ; position of SRAM used to store data


; Reset and Interrupt vectors
.org $000
rjmp init ; Start from initialization routine
.org $026
rjmp ISR_TC0


; code segment
.org $02A

// code from includes is moved here by the assembler
.include "7_segment_driver.asm"

//--------------------------------------------------------------------
// Main initialization routine
// Runs once at program startup. Sets SP, RAM, ports and interrupts
//--------------------------------------------------------------------
init:
	; Set stack pointer so program returns from interrupt where it occurred
	ldi r16, HIGH( RAMEND ) ; Upper byte
	out SPH,r16 ; to stack pointer
	ldi r16, LOW( RAMEND ) ; Lower byte
	out SPL,r16 ; to stack pointer

	rcall init_7_seg_driver

	; Enable interrupts
	sei

// Infinite loop. Main program
main_program: rjmp main_program 
