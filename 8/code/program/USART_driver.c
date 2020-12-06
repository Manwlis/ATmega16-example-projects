/*
 * USART_driver.c
 *
 * Driver for the USART. Controls the data in SRAM for the 7 segment display.
 * Transmits a response after every incoming message.
 *
 * Created: 10/11/2020
 * Author: Emmanouil Petrakos
 * Developed with AtmelStudio 7.0.129
 */ 

#include "program.h"
#include <avr/io.h> // Required for the I/O registers macros
#include <avr/interrupt.h> // Required for the ISR macro

extern unsigned char data[8];
extern unsigned char receiver_status;
extern unsigned char transmitter_status;
extern unsigned char OK_transmits_left;
extern unsigned char scheduler_control;


//--------------------------------------------------------------------
// Interrupt service routine for USART receive completed.
// Communication is errorless and previous/next inputs/states are irrelevant so a state-machine isn't needed.
// Only the current input is needed to take the appropriate action.
//--------------------------------------------------------------------
ISR( USART_RXC_vect )
{
	// Receive frame
	unsigned char received_frame = UDR;
	// for debug
	received_frame = UDR;
	asm( "mov %0 , r20" : "=r" ( received_frame ) );
	
	if( received_frame == 'S' )
		receiver_status = proc_enable_message; // Set type of message
		
	else if( received_frame == 'Q' )
		receiver_status = proc_disable_message; // Set type of message
		
	else if( received_frame == 'C' )
		for( unsigned char i = 0 ; i < num_of_data ; i++ )
			data[i] = 0x0A; // Clear data
			
	else if( received_frame == 'N' )
	{
		receiver_status = display_message; // Set type of message
		for( unsigned char i = 0 ; i < num_of_data ; i++ )
			data[i] = 0x0A; // Clear data
	}
	else if( received_frame == 'A' )
		return; // Do nothing
		
	else if( received_frame == 'T' )
		return; // Do nothing
		
	else if( received_frame == '\r' ) // <CR>
		return; // Do nothing
		
	else if( received_frame == '\n' ) // <LF>
	{
		// Message ended
		receiver_status = none;
		 // Increase pending responses counter
		OK_transmits_left++;
		// Enable transmitter interrupts to start the response. If it is already enabled, nothing changes.
		UCSRB |= ( 1 << UDRIE ); // breakpoint here to check memory after message
	}
	else // Frame is a number
	{
		unsigned char number = received_frame & ascii_to_bcd_mask;
		// Treatment of a number depends on the type of its message
		if ( receiver_status == display_message )
		{	
			// Moves all data one position forward. Top element gets discarded.
			for( unsigned char i = num_of_data - 2 ; i != 0xFF ; i-- ) // last element is in position 0x00
			data[ i + 1 ] = data[i];
			// Save new data. Top half byte cleared, ascii -> bcd
			data[0] = number;
		}
		else if( receiver_status == proc_enable_message )
			// Enable process
			scheduler_control |= ( 1 << ( number - 1 ) ); // process 1 enable is in bit 0 etc. -1 on number to get the correct bit.

		else if( receiver_status == proc_disable_message )
			// Disable process
			scheduler_control &= ~( 1 << ( number - 1 ) );
	}
}


//--------------------------------------------------------------------
// Interrupt service routine for USART transmitter completed.
// FSM is one way (see report), input is irrelevant. Only current state is needed to compute next state.
//--------------------------------------------------------------------
ISR( USART_UDRE_vect )
{
	if( transmitter_status == none )
	{
		// Decrease pending responses
		OK_transmits_left--;
		// Change State
		transmitter_status = 'O';
		// Send character
		UDR = 'O';
		// Transmitter's UDR is write-only and can't be read by the simulator.
		// TCNT2 is used for logging.
		TCNT2 = 'O';
	}
	else if( transmitter_status == 'O' )
	{
		// Change State
		transmitter_status = 'K';
		// Send character
		UDR = 'K';
		TCNT2 = 'K';
	}
	else if( transmitter_status == 'K' )
	{
		// Change State
		transmitter_status = 0x0D; // CR
		// Send character
		UDR = 0x0D;
		TCNT2 = 0x0D;
	}
	else if( transmitter_status == 0x0D )
	{
		// Change State
		transmitter_status = 0x0A; // LF
		// Send character
		UDR = 0x0A;
		TCNT2 = 0x0A;
	}
	else if( transmitter_status == 0x0A ) // LF
	{
		// Change State
		transmitter_status = none;
		// If no more transmits required, disable transmitter interrupts.
		if( OK_transmits_left == 0 )
			UCSRB &= ~( 1 << UDRIE );
	}
}