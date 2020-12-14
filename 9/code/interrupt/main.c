/*
 * interrupt.c
 *
 * Debounce an SPDT switch using external interrupts.
 *
 * Created: 14/12/2020
 * Author : Manolis Petrakos
 */ 

#include <avr/io.h> // Required for the I/O registers macros
#include <avr/interrupt.h> // Required for the intrinsic function sei()

void init_external_interrupt_driver();

/*-------------------------------------------------------------------------
* Main function. Calls appropriate initialization functions, enables interrupts
* and stays in a infinite loop.
* Functionality is serviced through interrupts.
*------------------------------------------------------------------------*/
int main(void)
{
	init_external_interrupt_driver();
	
	sei();
	
    while (1) 
    {
    }
}


/*-------------------------------------------------------------------------
* Initialize I/O registers in order to enable external interrupts 0 & 1.
*------------------------------------------------------------------------*/
void init_external_interrupt_driver()
{
	// Set port B bit 0 as output.
	DDRB |= ( 1 << PB0 );
	
	// falling edge Int0 & Int1
	MCUCR |= ( 1 << ISC11 ) | ( 1 << ISC01 );
	
	// Enable Int0 & Int1
	GICR |= ( 1 << INT1 ) | ( 1 << INT0 );
	
	// And init the output
	if( PIND & 0b00000100 )
		PORTB |= ( 1 << PB0 );
	else
		PORTB &= ~( 1 << PB0 );
}


/*-------------------------------------------------------------------------
* Interrupt service routine for external interrupt 0.
* Used for debouncing the SPDT switch.
*------------------------------------------------------------------------*/
ISR( INT0_vect )
{
	// Interrupt is enabled in falling edge mode, i.e. when PD2 becomes 0.
	// 0 appears only when touching/bouncing, so this is the side were the moving electrode is.
	PORTB &= ~( 1 << PB0 );
}


/*-------------------------------------------------------------------------
* Interrupt service routine for external interrupt 1.
* Used for debouncing the SPDT switch.
*------------------------------------------------------------------------*/
ISR( INT1_vect )
{
	PORTB |= ( 1 << PB0 );
}