/*
 * processes.c
 *
 * Simple processes used to test the scheduler. PORTB is their shared output.
 *
 * Created: 27/11/2020
 * Author: Emmanouil Petrakos
 * Developed with AtmelStudio 7.0.129
 */

#include "program.h"
#include <avr/io.h> // Required for the I/O registers macros
#include <util/delay.h>


volatile unsigned char bcd_counter_1ms_data __attribute__ ((section (".noinit")));
volatile unsigned char ring_counter_5ms_data __attribute__ ((section (".noinit")));
volatile unsigned char LED_blinking_7ms_data __attribute__ ((section (".noinit")));


/*-------------------------------------------------------------------------
* Initialize processes memory and shared port.
*------------------------------------------------------------------------*/
void init_processes()
{
	// Set Port B as output
	DDRB = 0xFF;
	
	// Set starting data
	bcd_counter_1ms_data = 0;
	ring_counter_5ms_data = 0b10000000;
	LED_blinking_7ms_data = 0b00000000;
}


/*-------------------------------------------------------------------------
* Bcd counter with 1ms delay. PORTB is output.
*------------------------------------------------------------------------*/
void bcd_counter_1ms()
{
	_delay_ms(1);
	bcd_counter_1ms_data++;
	PORTB = bcd_counter_1ms_data;
}


/*-------------------------------------------------------------------------
* Ring counter with 5ms delay. PORTB is output.
*------------------------------------------------------------------------*/
void ring_counter_5ms()
{
	_delay_ms(5);
	ring_counter_5ms_data = ( ring_counter_5ms_data >> 7 ) | ( ring_counter_5ms_data << 1 );
	PORTB = ring_counter_5ms_data;
}


/*-------------------------------------------------------------------------
* Constant inversion of data with 7ms delay. PORTB is output.
*------------------------------------------------------------------------*/
void LED_blinking_7ms()
{
	_delay_ms(7);
	LED_blinking_7ms_data = LED_blinking_7ms_data ^ 0xFF;
	PORTB = LED_blinking_7ms_data;
}