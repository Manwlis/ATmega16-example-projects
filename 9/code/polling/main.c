/*
 * polling.c
 *
 * Debounce an SPDT switch using polling.
 *
 * Created: 13/12/2020
 * Author : Emmanouil Petrakos
 */ 

#include <avr/io.h> // Required for the I/O registers macros
#include <avr/interrupt.h> // Required for the intrinsic function sei()

// Compare register value for 10 ms delay with /1024 prescaler
#define OCR0_value 97

void init_polling_driver();

volatile unsigned char old_A0_state __attribute__ ((section (".noinit")));
volatile unsigned char old_A1_state __attribute__ ((section (".noinit")));

/*-------------------------------------------------------------------------
* Main function. Calls appropriate initialization functions, enables interrupts
* and stays in a infinite loop.
* Functionality is serviced through interrupts.
*------------------------------------------------------------------------*/
int main(void)
{
	init_polling_driver();
	
	// Enable global interrupts
	sei(); // breakpoint here to see outptut after Initializations
	
    while (1) 
    {
    }
}

/*-------------------------------------------------------------------------
* Initialize ports, timer and memory that are used by the polling driver.
*------------------------------------------------------------------------*/
void init_polling_driver()
{
	// Set Port A bits 0-1 as inputs.
	DDRA |= ( 0 << PA1 ) | ( 0 << PA0 );
	// Set Port B bits 0 as output.
	DDRB |= ( 1 << PB0 );
	
	// Set Timer0 at ~10ms
	// Set Timer/Counter0 prescaler to 1024 and Compare Mode to clear counter on match
	TCCR0 = ( 1 << CS02 ) | ( 1 << CS00 ) | ( 1 << WGM01 );
	// Enable Timer/Counter0 Output Compare Match Interrupt
	TIMSK = ( 1 << OCIE0 );
	OCR0 = OCR0_value;
	
	// Init mem with the initial state of the SPDT
	old_A0_state = PINA & 0b00000001;
	old_A1_state = ( PINA & 0b00000010 ) >> 1 ;
	// And init the output
	if( old_A0_state == 1 )
		PORTB |= ( 1 << PB0 );
	else
		PORTB &= ~( 1 << PB0 );
}


/*-------------------------------------------------------------------------
* Interrupt service routine for timer/counter0 compare match mode.
* Used for debouncing the SPDT switch.
*------------------------------------------------------------------------*/
ISR( TIMER0_COMP_vect )
{
	unsigned char current_A0_state = PINA & 0b00000001;
	unsigned char current_A1_state = ( PINA & 0b00000010 ) >> 1;
		
	if( old_A0_state == 0 && current_A0_state == 1 )
	{
		// Change output
		PORTB |= ( 1 << PB0 );
		// Save the change to mem
		// Next poll will happen after the rebounce has quelled. No need to do anything clever.
		old_A0_state = 1;
		old_A1_state = 0;
	}
	else if( old_A1_state == 0 && current_A1_state == 1 )
	{
		// Change output
		PORTB &= ~( 1 << PB0 );
		// Save the change to mem
		old_A0_state = 0;
		old_A1_state = 1;
	}
}// breakpoint here to see state after polling