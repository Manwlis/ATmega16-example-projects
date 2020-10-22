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

	ret


//--------------------------------------------------------------------
// Interrupt service routine for USART receive completed.
// arguments: none
// returns: none
// changes: r16
//--------------------------------------------------------------------
ISR_URXC:

	; receive frame
	in r16, UDR
	in r16, UDR ; for debug
	mov r16, r15 ; for debug
	mov r17, r16

	cpi r17, 0x43 ; check if C
	brne not_C
	rcall clr_7seg_data
	reti
not_C:

	cpi r17, 0x4E ; check if N
	brne not_N
	rcall clr_7seg_data
	reti
not_N:

	cpi r17, 0x0D ; check if <CR>
	brne not_CR
	reti
not_CR:

	cpi r17, 0x0A ; check if <LF>
	brne not_LF
	rcall USART_Transmit
	reti
not_LF:

	; if program reach here, frame is a character ( transmition is errorless )
	
	andi r17, 0x0F ; clears top half byte, ascii -> bcd
	mov r0, r17
	rcall save_to_7seg_data ; save bcd it in SRAM



	reti

	
USART_Transmit:
	; Wait for empty transmit buffer
	sbis UCSRA,UDRE
	rjmp USART_Transmit
	ldi r16, 0x41
	out TCNT2,r16

	a:
	sbis UCSRA,UDRE
	rjmp a
	ldi r16, 0x54
	out TCNT2,r16

	b:
	sbis UCSRA,UDRE
	rjmp b
	ldi r16, 0x0D
	out TCNT2,r16

	c:
	sbis UCSRA,UDRE
	rjmp c
	ldi r16, 0x0A
	out TCNT2,r16

	ret