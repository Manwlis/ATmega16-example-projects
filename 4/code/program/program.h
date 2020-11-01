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
	
	// Compare register value
	#define OCR_value 77
	// 1 data for each 7 segment
	#define num_of_data 8

	// Transmitter States
	#define none 0xFF
	#define O 0x4F
	#define K 0x4B
	#define CR 0x0D
	#define LF 0x0A

	// ( F_CPU / ( baudrate * 16 ) ) - 1 = ~64,1
	#define UBBR_value 64

	// LOW, HIGH byte functionality from assembly to C
	#define low(x)	( (x) & 0xFF )
	#define high(x)	( ( (x) >> 8 ) & 0xFF )

#endif /* PROGRAM_H_ */