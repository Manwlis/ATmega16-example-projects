/*
 * main.c
 *
 * Initializations for the drivers and main loop.
 * Functionality is served through interrupts.
 *
 * Created: 30/10/2020
 * Author : Emmanouil Petrakos
 * Developed with AtmelStudio 7.0.129
 */ 

#include <avr/io.h> // Required for the I/O registers macros
#include <avr/interrupt.h> // Required for the intrinsic function sei()
#include "program.h"


// __attribute__ ((section (".noinit"))) because there is no need to be 
// initialized to 0 by the compiler.
// "volatile" to show to compiler that they can change outside the program.
volatile unsigned char data[8] __attribute__ ((section (".noinit")));
volatile unsigned char segments_encoding[11] __attribute__ ((section (".noinit")));

volatile unsigned char transmitter_status __attribute__ ((section (".noinit")));
volatile unsigned char OK_transmits_left __attribute__ ((section (".noinit")));
volatile unsigned char R_transmits_left __attribute__ ((section (".noinit")));

void init_7_seg_driver_IO();
void init_7_seg_driver_mem();
void init_USART_driver_IO();
void init_USART_driver_mem();


/*-------------------------------------------------------------------------
* Main function. Checks reset source, calls appropriate initialization functions, 
* enables interrupt and stay in a infinite loop. Functionality is serviced through interrupts.
*------------------------------------------------------------------------*/
int main()
{
	// avr-libc recommends to store MCUSR in a local variable early after the reset and use that
	unsigned char reset_source = MCUCSR;
	// clear MCUCSR
	MCUCSR = 0x00;
	// On ATmega16 WDT clears after reset. No need to manually disable it
	
	// Compiler sets stack pointer. No need for the program to do anything
	
	// Initialize drivers
	// Watchdog timer clears I/O registers, they must always get reinitialized
	init_7_seg_driver_IO();
	init_USART_driver_IO();
	
	// if power-on reset, or warm start is disabled
	if( ( reset_source & ( 1 << PORF ) ) || !warm_start_enable )
	{	// Watchdog reset doesn't affect SRAM, no need to be initialized again
		init_7_seg_driver_mem();
		init_USART_driver_mem();
	}

	// Send reset response
	if( reset_source & ( 1 << WDRF ) )
	{
		// Increase reset pending responses counter
		R_transmits_left++;
		// Enable transmitter interrupts to start the response
		UCSRB |= ( 1 << UDRIE );
	}
	
	// Enable global interrupts
	sei(); // Breakpoint here to execute stimuli file
	
    while(1) 
    {
		// Visual debugger doesn't really like empty while loops.
		// It shows that the loop loops at the previous line.
		// Volatile so compiler doesn't optimize it away.
		volatile int i = 0;
    }
}


/*-------------------------------------------------------------------------
* Initialize ports and timer that are used by the 7 segment driver.
*------------------------------------------------------------------------*/
void init_7_seg_driver_IO()
{
	// Set Ports A and C as outputs and initialize
	DDRA = 0xFF;
	DDRC = 0xFF;
	PORTA = 0xFF; // all segments off
	PORTC = 0b10000000; // in order to start from rightmost 7 segment (AN0)
	
	// Set Timer0 at ~2ms
	TCCR0 = ( 1 << CS02 ) | ( 1 << WGM01 );
	TIMSK = 1 << OCIE0;
	OCR0 = OCR_value;
}


/*-------------------------------------------------------------------------
* Initialize the memory that is used by the 7 segment driver.
*------------------------------------------------------------------------*/
void init_7_seg_driver_mem()
{
	// Set data
	for( unsigned char i = 0 ; i < num_of_data ; i++ )
	data[i] = 0x0A;
	
	// Set encodings
	segments_encoding[0] = 0b00000011;
	segments_encoding[1] = 0b10011111;
	segments_encoding[2] = 0b00100101;
	segments_encoding[3] = 0b00001101;
	segments_encoding[4] = 0b10011001;
	segments_encoding[5] = 0b01001001;
	segments_encoding[6] = 0b01000001;
	segments_encoding[7] = 0b00011111;
	segments_encoding[8] = 0b00000001;
	segments_encoding[9] = 0b00001001;
	segments_encoding[10] = 0b11111111;
}

/*-------------------------------------------------------------------------
* Initialize ports and timer that are used by the USART driver.
*------------------------------------------------------------------------*/
void init_USART_driver_IO()
{
	// Set RXD as input and TXD as output.
	DDRD |= ( 0 << 0 ); // RXD
	DDRD |= ( 1 << 1 ); // TXD
	
	// Set UBRR for 9600 baud rate when clock is 10MHz. Let compiler calculate the correct value
	#include <util/setbaud.h>
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	
	// UCSRA doesn't need to change, initial values are okay

	// Enable receiver, receive completed interrupt the transmitter
	// 8 bit word: UCSZ2 = 0
	UCSRB = ( 1 << RXEN ) | ( 1 << RXCIE ) | ( 1 << TXEN );
	
	// Write in UCSRC: URSEL = 1. Asynchronous operation: UMSEL = 0.
	// Parity Disabled: UPM1:0 = 00. 8 bit word: UCSZ1:0 = 11
	UCSRC = ( 1 << URSEL ) | ( 1 << UCSZ1 ) | ( 1 << UCSZ0 );
}


/*-------------------------------------------------------------------------
* Initialize the memory that is used by the USART driver.
*------------------------------------------------------------------------*/
void init_USART_driver_mem()
{
	#define none 0xFF
	// Initialize transmitter's FSM to neutral state and number of remaining transmits to 0.
	transmitter_status = none;
	OK_transmits_left = 0;
	R_transmits_left = 0;
}