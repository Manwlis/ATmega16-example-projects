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

#include <avr/io.h>
#include <avr/interrupt.h>
#include "program.h"


// __attribute__ ((section (".noinit"))) because there is no need to be 
// initialized to 0 by the compiler.
// "volatile" to show to compiler that they can change outside the program.
volatile unsigned char data[8] __attribute__ ((section (".noinit")));
volatile unsigned char segments_encoding[11] __attribute__ ((section (".noinit")));

volatile unsigned char transmitter_status __attribute__ ((section (".noinit")));
volatile unsigned char num_transmits_left __attribute__ ((section (".noinit")));


void init_7_seg_driver();
void init_USART_driver();


/*-------------------------------------------------------------------------
* Main function. Calls initialization functions, enables interrupt and 
* stay in a infinite loop. Functionality is serviced through interrupts.
*------------------------------------------------------------------------*/
int main()
{
	// Compiler sets stack pointer. No need for the program to do anything.
	
	// Initialize drivers
	init_7_seg_driver();
	init_USART_driver();
	
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
* Initialize memory, ports and timer that are used by the 7 segment driver.
*------------------------------------------------------------------------*/
void init_7_seg_driver()
{
	// Set Memory
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
* Initialize memory, ports and timer that are used by the USART driver.
*------------------------------------------------------------------------*/
void init_USART_driver()
{
	// Set RXD as input and TXD as output.
	DDRD |= ( 0 << 0 ); // RXD
	DDRD |= ( 1 << 1 ); // TXD
	
	// Set UBRR for 9600 baud rate when clock is 10MHz
	UBRRH = high( UBBR_value );
	UBRRL = low( UBBR_value );
	
	// UCSRA doesn't need to change, initial values are okay

	// Enable receiver, receive completed interrupt the transmitter
	// 8 bit word: UCSZ2 = 0
	UCSRB = ( 1 << RXEN ) | ( 1 << RXCIE ) | ( 1 << TXEN );
	
	// Write in UCSRC: URSEL = 1. Asynchronous operation: UMSEL = 0.
	// Parity Disabled: UPM1:0 = 00. 8 bit word: UCSZ1:0 = 11
	UCSRC = ( 1 << URSEL ) | ( 1 << UCSZ1 ) | ( 1 << UCSZ0 );
	
	// Initialize transmitter's FSM to neutral state and number of remaining transmits to 0.
	transmitter_status = none;
	num_transmits_left = 0;
}