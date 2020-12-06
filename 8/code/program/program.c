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

#include "program.h"
#include <avr/io.h> // Required for the I/O registers macros
#include <avr/interrupt.h> // Required for the intrinsic function sei()


// __attribute__ ((section (".noinit"))) because there is no need to be 
// initialized to 0 by the compiler.
// "volatile" to show to compiler that they can change outside the program.
volatile unsigned char data[8] __attribute__ ((section (".noinit")));
volatile unsigned char segments_encoding[11] __attribute__ ((section (".noinit")));

volatile unsigned char receiver_status __attribute__ ((section (".noinit")));

volatile unsigned char transmitter_status __attribute__ ((section (".noinit")));
volatile unsigned char OK_transmits_left __attribute__ ((section (".noinit")));

volatile unsigned char scheduler_control __attribute__ ((section (".noinit")));

void init_7_seg_driver_IO();
void init_7_seg_driver_mem();
void init_USART_driver_IO();
void init_USART_driver_mem();
void init_scheduler();


/*-------------------------------------------------------------------------
* Main function. Checks reset source, calls appropriate initialization functions, 
* enables interrupt and stay in a infinite loop. Functionality is serviced through interrupts.
*------------------------------------------------------------------------*/
int main()
{	
	// Compiler sets stack pointer. No need for the program to do anything
	
	// Initialize drivers
	init_7_seg_driver_IO();
	init_USART_driver_IO();
	init_7_seg_driver_mem();
	init_USART_driver_mem();
	// Initialize scheduler
	init_scheduler();
	// Initialize processes
	init_processes();
	
	// Enable global interrupts
	sei(); // Breakpoint here to execute stimuli file
	
    while(1) 
    {
		// for now all enabled processes run one after another.
		if( scheduler_control & ( 1 << SCPE1 ) )
			bcd_counter_1ms();
		if( scheduler_control & ( 1 << SCPE2 ) )
			ring_counter_5ms();
		if( scheduler_control & ( 1 << SCPE3 ) )
			LED_blinking_7ms();
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
	TCCR0 = ( 1 << CS02 ) | ( 1 << WGM01 ); // Set Timer/Counter0 prescaler to 64 and Compare Mode to clear counter on match
	TIMSK = 1 << OCIE0; // Enable Timer/Counter0 Output Compare Match Interrupt
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
	
	// Set UBRR for 9600 baud rate when clock is 10MHz. Let compiler calculate the correct values
	#include <util/setbaud.h>
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	#if USE_2X
		UCSRA |= ( 1 << U2X );
	#else
		UCSRA &= ~( 1 << U2X );
	#endif
	
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
	// Initialize transmitter's FSM to neutral state and number of remaining transmits to 0.
	transmitter_status = none;
	receiver_status = none;
	OK_transmits_left = 0;
}


/*-------------------------------------------------------------------------
* Initialize scheduler memory.
* TO-DO: lab8
*------------------------------------------------------------------------*/
void init_scheduler()
{
	// Enable all processes
	scheduler_control = 0x00;
	
	// Set Timer2 at ~100ms
	TCCR1B = ( 1 << WGM12 ) | ( 1 << CS12 ) | ( 1 << CS10 ); // Set Timer/Counter2 prescaler to 1024 and Compare Mode to clear counter on match
	TIMSK = 1 << OCIE1A; // Enable Timer/Counter2 Output Compare Match Interrupt
	
	
	OCR1AH = 1;
	OCR1AL = 10;
}

ISR( TIMER1_COMPA_vect )
{
	int i = 0;
}