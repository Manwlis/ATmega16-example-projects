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
	
	// Get data based on ring counter
	// This block replaces assembly's ring_to_bcd and read from memory.
	// Tried to calculate offset like the assembly implementation with a for loop,
	// but ISR lasted x3 more cycles. 
	unsigned char bcd_number;

	if( ring_counter == 0b00000001 )
		bcd_number = data[0];
	else if( ring_counter == 0b00000010 )
		bcd_number = data[1];
	else if( ring_counter == 0b00000100 )
		bcd_number = data[2];
	else if( ring_counter == 0b00001000 )
		bcd_number = data[3];
	else if( ring_counter == 0b00010000 )
		bcd_number = data[4];
	else if( ring_counter == 0b00100000 )
		bcd_number = data[5];
	else if( ring_counter == 0b01000000 )
		bcd_number = data[6];
	else if( ring_counter == 0b10000000 )
		bcd_number = data[7];
		
	// illegal data. PORTA = 0xFF
	if( bcd_number > 0x0A )
		return;

	// Out to port. Replaces assembly's BCD_to_7_segment_Decoder and out
	PORTA = segments_encoding[bcd_number];
} // breakpoint here to check 7 segment ports