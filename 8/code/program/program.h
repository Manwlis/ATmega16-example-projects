/*
 * program.h
 *
 * Macros and enumerations.
 *
 * Created: 30/10/2020
 * Author : Emmanouil Petrakos
 * Developed with AtmelStudio 7.0.129
 */ 


#ifndef PROGRAM_H_
#define PROGRAM_H_
	
	#define	F_CPU	10000000UL
	#define	BAUD		9600
	
	#define ascii_to_bcd_mask 0x0F
	
	// Compare register value for timer0
	#define OCR_value 77
	
	// 1 data for each 7 segment
	#define num_of_data 8
	
	// Used to show no transmitting happened or no USART instruction started
	#define none 0x00
	
	// Receiver states
	#define display_message 'N'
	#define proc_enable_message 'S'
	#define proc_disable_message 'Q'
	
	// SCheduler Process Enable bits
	#define	SCPE1	0
	#define	SCPE2	1
	#define	SCPE3	2
	// SCheduler Process Running bits
	#define	SCPR1	3
	#define	SCPR2	4
	#define	SCPR3	5
	
	// Compare registers A value for timer1
	#define OCR1A_value 15624
	

#endif /* PROGRAM_H_ */