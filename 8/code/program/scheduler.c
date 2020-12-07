/*
 * scheduler.c
 *
 * Simple ring Scheduler for three processes. Processes can be disabled.
 * Time-slice is static at 100 ms. Uses 16 bit Timer1 Compare interrupts.
 *
 * Created: 7/12/2020
 * Author: Emmanouil Petrakos
 * Developed with AtmelStudio 7.0.129
 */


#include "program.h"
#include <avr/io.h> // Required for the I/O registers macros
#include <avr/interrupt.h> // Required for the intrinsic function sei()


// Save running & enabled processes
volatile unsigned char scheduler_control __attribute__ ((section (".noinit")));


/*-------------------------------------------------------------------------
* Initialize memory and Timer1 used by scheduler.
*------------------------------------------------------------------------*/
void init_scheduler()
{
	// Enable all processes
	scheduler_control = 0x00;
	
	// Set Timer1 at ~100ms
	TCCR1B = ( 1 << WGM12 ) | ( 1 << CS11 ) | ( 1 << CS10 ); // Set Timer/Counter2 prescaler to 64 and Compare Mode to clear counter on match
	TIMSK = 1 << OCIE1A; // Enable Timer/Counter2 Output Compare Match Interrupt
	
	OCR1AH = OCR1A_value >> 8; // High byte
	OCR1AL = OCR1A_value & 0x00FF; // Lower byte
}


/*-------------------------------------------------------------------------
* Interrupt service routine for timer/counter0 compare A match mode.
* Used to implement scheduler. Gives new time-slice to the next enabled process.
*------------------------------------------------------------------------*/
ISR( TIMER1_COMPA_vect )
{
	// local variable to minimize memory accesses.
	unsigned char temp = scheduler_control;
	
	// No process enabled, stop whatever is running and return.
	if( !( temp & ( 1 << SCPE3 ) ) && !( temp & ( 1 << SCPE2 ) ) && !( temp & ( 1 << SCPE1 ) ) )
	{
		scheduler_control = 0; // Enable bits are already cleared. Clear running bits to stop any running process.
		return;
	}
	do
	{
		// SCPR is used like a shift counter.
		if( temp & ( 1 << SCPR3 ) )
		{	
			temp &= ~( 1 << SCPR3 ); // Clear current bit
			temp |= ( 1 << SCPR2 ); // and set the next
		}
		else if( temp & ( 1 << SCPR2 ) )
		{
			temp &= ~( 1 << SCPR2 ); // Clear current bit
			temp |= ( 1 << SCPR1 ); // and set the next
		}
		else if( temp & ( 1 << SCPR1 ) )
		{
			temp &= ~( 1 << SCPR1 ); // Clear current bit
			temp |= ( 1 << SCPR3 ); // and set the next
		}
		else // No process is running, start with process 3
			temp |= ( 1 << SCPR3 );
		
	} while( // Do this as long as no enabled process is running
		( !( temp & ( 1 << SCPR3 ) ) || !( temp & ( 1 << SCPE3 ) ) ) && // process 3 is not running or is not enabled AND
		( !( temp & ( 1 << SCPR2 ) ) || !( temp & ( 1 << SCPE2 ) ) ) && // process 2 is not running or is not enabled AND
		( !( temp & ( 1 << SCPR1 ) ) || !( temp & ( 1 << SCPE1 ) ) ) ); // process 1 is not running or is not enabled
	// Loop can run 3 times maximum. At least one process is enabled
	// and the shift register needs 3 loops to come around.
	// This happens when the currently running process is the only enabled one.
	
	scheduler_control = temp;
}