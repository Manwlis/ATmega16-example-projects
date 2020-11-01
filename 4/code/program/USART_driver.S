;
; USART_driver.asm
;
; Driver for the USART. Controls the data in SRAM for the 7 segment display.
;
; Created: 22/10/2020
; Author : Emmanouil Petrakos
; Developed with AtmelStudio 7.0.129
;

#include <avr/io.h>
#include "program.h"

; defined in C code
.extern transmitter_status
.extern num_transmits_left


//--------------------------------------------------------------------
// Interrupt service routine for USART receive completed.
// Transmition is errorless and previous/next inputs/states are irrelevant so a state-machine isn't needed.
// Only the current input is needed to take the appropiate action.
// arguments: none
// returns: none
// changes: r18, r19, r20(for debug)
//--------------------------------------------------------------------
.global USART_RXC_vect
USART_RXC_vect:

	; receive frame
	in r18, _SFR_IO_ADDR( UDR )
	in r18, _SFR_IO_ADDR( UDR ) ; for debug
	mov r18, r20 ; for debug
	mov r19, r18 ; r18 change values in subroutines and need a reg with immediate capabilities.

	cpi r19, 0x43 ; check if C
	breq frame_C_N
	cpi r19, 0x4E ; check if N
	breq frame_C_N
	cpi r19, 0x41 ; check if A
	breq frame_A_T_CR
	cpi r19, 0x54 ; check if T
	breq frame_A_T_CR
	cpi r19, 0x0D ; check if <CR>
	breq frame_A_T_CR
	cpi r19, 0x0A ; check if <LF>
	breq frame_LF
	; if program reach here, frame is a character ( transmition is errorless )
	rjmp frame_number
	
frame_C_N:
	rcall clr_7seg_data
	rjmp exit_ISR_URXC

frame_A_T_CR:
	rjmp exit_ISR_URXC

frame_LF:
	; increase the number of responces left
	lds r18, num_transmits_left
	inc r18 ; if >= 256 responses pile up, oopsie. It isn't expected to happen during normal operation.
	sts num_transmits_left, r18
	; enable transmitter interrupts to start the response. If it is already enabled, nothing changes.
	sbi _SFR_IO_ADDR( UCSRB ), UDRIE
	rjmp exit_ISR_URXC

frame_number:
	; send data to bcd
	andi r19, 0x0F ; clears top half byte, ascii -> bcd
	mov r0, r19
	rcall save_to_7seg_data ; save bcd it in SRAM

exit_ISR_URXC:
	reti


//--------------------------------------------------------------------
// Interrupt service routine for USART transmitte completed.
// FSM is oneway (see report), input is irrelevant. Only current state is needed to compute next state.
// Oopsie if new <LF> is received before the transmittion is completed.
// arguments: none
// returns: none
// changes: r18, r19
//--------------------------------------------------------------------
.global USART_UDRE_vect
USART_UDRE_vect:

	; read old state
	lds r18, transmitter_status
	; check state
	cpi r18, none
	breq prv_state_none
	cpi r18, O
	breq prv_state_O
	cpi r18, K
	breq prv_state_K
	cpi r18, CR
	breq prv_state_CR
	cpi r18, LF
	breq prv_state_LF

; new state
prv_state_none:
	; decrease the number of responces left
	lds r18, num_transmits_left
	dec r18
	sts num_transmits_left, r18
	; change state
	ldi r18, O
	sts transmitter_status, r18
	; send character. The character is the state.
	out _SFR_IO_ADDR( UDR ), r18
	; Transmiter's UDR is write-only and can't be read by the simulator.
	; Also only I/O registers can be logged so r18 can't be used for logging purposes.
	; TCNT2 is used for logging.
	out _SFR_IO_ADDR( TCNT2 ), r18
	rjmp ISR_UDRE_exit

prv_state_O:
	; change state
	ldi r18, K
	sts transmitter_status, r18
	; send character. The character is the state.
	out _SFR_IO_ADDR( UDR ), r18
	out _SFR_IO_ADDR( TCNT2 ), r18
	rjmp ISR_UDRE_exit

prv_state_K:
	; change state
	ldi r18, CR
	sts transmitter_status, r18
	; send character. The character is the state.
	out _SFR_IO_ADDR( UDR ), r18
	out _SFR_IO_ADDR( TCNT2 ), r18
	rjmp ISR_UDRE_exit

prv_state_CR:
	; change state
	ldi r18, LF
	sts transmitter_status, r18
	; send character. The character is the state.
	out _SFR_IO_ADDR( UDR ), r18
	out _SFR_IO_ADDR( TCNT2 ), r18
	rjmp ISR_UDRE_exit

prv_state_LF:
	; change state
	ldi r18, none
	sts transmitter_status, r18
	; if no more transmittion required, disable transmitter interrupts.
	lds r18, num_transmits_left
	cpi r18, 0
	brne ISR_UDRE_exit
	; disable interrupt
	cbi _SFR_IO_ADDR( UCSRB ), UDRIE

	; exit
ISR_UDRE_exit:
	reti
