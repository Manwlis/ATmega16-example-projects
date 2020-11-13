/*
 * 7_segment_driver.c
 *
 * Driver for an 8 digits 7 segment display. Shows data from SRAM.
 *
 * Created: 10/11/2020
 * Author: Emmanouil Petrakos
 * Developed with AtmelStudio 7.0.129
 */ 

#include <avr/io.h> // Required for the I/O registers macros
#include <avr/interrupt.h> // Required for the ISR macro

extern unsigned char data[8];
extern unsigned char segments_encoding[11];


//--------------------------------------------------------------------
// Interrupt service routine for timer/counter0 compare match mode.
// Controls 7 segments outputs.
//--------------------------------------------------------------------
ISR( TIMER0_COMP_vect )
{
	// Show nothing
	PORTA = 0xFF;
	
	// Ring counter
	// local variable in order to minimize access to I/O ports
	unsigned char ring_counter = PORTC;
	ring_counter = ( ring_counter >> 7 ) | ( ring_counter << 1 );
	PORTC = ring_counter;
	
	// Get data based on ring counter. This block replaces assembly's ring_to_bcd
	unsigned char shift_counter = 0;
	while(1)
	{
		ring_counter >>= 1;
		if( ring_counter != 0 )
			shift_counter++;
		else
			break;
	}
	
	//  Read from memory.
	unsigned char bcd_number = data[shift_counter];

	// Out to port. Replaces assembly's BCD_to_7_segment_Decoder and out
	PORTA = segments_encoding[bcd_number];
} // breakpoint here to check 7 segment ports