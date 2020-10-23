;
; USART_driver.asm
; Driver for the USART on PORTC.
; 9600 baudrate, 8 data bits, 1 stop bit, no parity
;
; Created: 22/10/2020
; Author : Emmanouil Petrakos
; Developed with AtmelStudio 7.0.129
;

; ( F_CPU / ( baudrate * 16 ) ) - 1 = ~64,1
.equ UBBR_value = 64

; Transmitter need an FSM. This position is used to keep the status.
.equ transmiter_status_pointer = 0x0073
; how many responces have piled up
.equ num_transmittion_left = 0x0074
; Transmitter States
.equ none = 0xFF
.equ O = 0x4F
.equ K = 0x4B
.equ CR = 0x0D
.equ LF = 0x0A

//--------------------------------------------------------------------
// Initialize memory, ports and timer that are used by the driver.
// arguments: none
// returns: none
// changes: r16
//--------------------------------------------------------------------
init_USART_driver:

	; set RXD as input and TXD as output.
	cbi DDRD, 0 ; RXD
	sbi DDRD, 1 ; TXD

	; Set UBRR for 9600 baudrate when clk is 10MHz
	ldi r16, HIGH( UBBR_value ) ; Upper half byte
	out UBRRH, r16 ; to upper Baud Rate Register
	ldi r16, LOW( UBBR_value ) ; Lower byte
	out UBRRL, r16 ; to lower Baud Rate Register

	; UCSRA doesn't need to change, initial values are okay

	; Enable receiver, recieve completed interrupt the transmiter
	; 8 bit word: UCSZ2 = 0
	ldi r16, ( 1 << RXEN ) | ( 1 << RXCIE ) | ( 1 << TXEN )
	out UCSRB, r16

	; Write in UCSRC: URSEL = 1. Asynchronous operation: UMSEL = 0.
	; Parity Disabled: UPM1:0 = 00. 8 bit word: UCSZ1:0 = 11
	ldi r16, ( 1 << URSEL ) | ( 1 << UCSZ1 ) | ( 1 << UCSZ0 )
	out UCSRC, r16

	; initialize transmitter's FSM to neutral state and num of remaining transmittions to 0.
	ldi r16, none
	sts transmiter_status_pointer, r16
	clr r16
	sts num_transmittion_left, r16

	ret


//--------------------------------------------------------------------
// Interrupt service routine for USART receive completed.
// Transmition is errorless so previous inputs are irrelevant and a state-machine isn't needed.
// Only the current input is needed to take the appropiate action.
// arguments: none
// returns: none
// changes: r16, r17, r15(for debug)
//--------------------------------------------------------------------
ISR_URXC:

	; receive frame
	in r16, UDR
	in r16, UDR ; for debug
	mov r16, r15 ; for debug
	mov r17, r16 ; r16 change values in subroutines and need a reg with immediate capabilities.

	cpi r17, 0x43 ; check if C
	breq frame_C_N
	cpi r17, 0x4E ; check if N
	breq frame_C_N
	cpi r17, 0x0D ; check if <CR>
	breq frame_CR
	cpi r17, 0x0A ; check if <LF>
	breq frame_LF
	; if program reach here, frame is a character ( transmition is errorless )
	rjmp frame_number
	
frame_C_N:
	rcall clr_7seg_data
	rjmp exit_ISR_URXC

frame_CR:
	rjmp exit_ISR_URXC

frame_LF:
	; increase the number of responces left
	lds r16, num_transmittion_left
	inc r16 ; if >= 256 responses pile up, oopsie. It isn't expected to happen during normal operation.
	sts num_transmittion_left, r16
	; enable transmitter interrupts to start the response. If it is already enabled, nothing changes.
	sbi UCSRB, UDRIE
	rjmp exit_ISR_URXC

frame_number:
	; send data to bcd
	andi r17, 0x0F ; clears top half byte, ascii -> bcd
	mov r0, r17
	rcall save_to_7seg_data ; save bcd it in SRAM

exit_ISR_URXC:
	reti


//--------------------------------------------------------------------
// Interrupt service routine for USART transmitte completed.
// FSM is oneway (see report), input is irrelevant. Only current state is needed to compute next state.
// Oopsie if new <LF> is received before the transmittion is completed.
// arguments: none
// returns: none
// changes: r16, r17
//--------------------------------------------------------------------
ISR_UDRE:

	; read old state
	lds r16, transmiter_status_pointer
	; check state
	cpi r16, none
	breq prv_state_none
	cpi r16, O
	breq prv_state_O
	cpi r16, K
	breq prv_state_K
	cpi r16, CR
	breq prv_state_CR
	cpi r16, LF
	breq prv_state_LF

; new state
prv_state_none:
	; decrease the number of responces left
	lds r16, num_transmittion_left
	dec r16
	sts num_transmittion_left, r16
	; change state
	ldi r16, O
	sts transmiter_status_pointer, r16
	; send character. The character is the state.
	out UDR,r16
	; Transmiter's UDR is write-only and can't be read by the simulator.
	; Also only I/O registers can be logged so r16 can't be used for logging purposes.
	; TCNT2 is used for logging.
	out TCNT2,r16
	rjmp ISR_UDRE_exit

prv_state_O:
	; change state
	ldi r16, K
	sts transmiter_status_pointer, r16
	; send character. The character is the state.
	out UDR,r16
	out TCNT2,r16
	rjmp ISR_UDRE_exit

prv_state_K:
	; change state
	ldi r16, CR
	sts transmiter_status_pointer, r16
	; send character. The character is the state.
	out UDR,r16
	out TCNT2,r16
	rjmp ISR_UDRE_exit

prv_state_CR:
	; change state
	ldi r16, LF
	sts transmiter_status_pointer, r16
	; send character. The character is the state.
	out UDR,r16
	out TCNT2,r16
	rjmp ISR_UDRE_exit

prv_state_LF:
	; change state
	ldi r16, none
	sts transmiter_status_pointer, r16
	; if no more transmittion required, disable transmitter interrupts.
	lds r16, num_transmittion_left
	cpi r16, 0
	brne ISR_UDRE_exit
	; disable interrupt
	cbi UCSRB, UDRIE

	; exit
ISR_UDRE_exit:
	reti

