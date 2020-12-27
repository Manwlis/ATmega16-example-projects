/*
 * long.c
 *
 * Author : Emmanouil Petrakos
 * Created: 27/12/2020
 * Developed with AtmelStudio 7.0.129
 */ 

#include <avr/io.h>


__attribute__((noinline)) void matrix_3x3_multiplication( long input1[3][3] , long input2[3][3] , long output[3][3] );


// Declare and initialize arrays.
// Globals so they don't get optimized away.
long matrix1[3][3] =
{
	{ 1 , 2 , 3 } ,
	{ 4 , 5 , 6 } ,
	{ 7 , 8 , 9 }
};
long matrix2[3][3] =
{
	{ 1 , 2 , 3 } ,
	{ 4 , 5 , 6 } ,
	{ 7 , 8 , 9 }
};
long matrix3[3][3] =
{
	{ 0 , 0 , 0 } ,
	{ 0 , 0 , 0 } ,
	{ 0 , 0 , 0 }
};


/**
 * \brief 
 * eafesaf
 */
int main( void )
{	
	// Call multiplication function.
	matrix_3x3_multiplication( matrix1 , matrix2 , matrix3 );
	
	// Don't let the program hang.
    while (1) 
    {
    }
}


/**
 * \brief 
 * drgsg
 * 
 * \param input1 pointer to the 3x3 array of longs used as the multiplier.
 * \param input2 pointer to the 3x3 array of longs used as the multiplicand.
 * \param output pointer to the 3x3 array of longs where the product is saved.
 * 
 * \return void
 */
__attribute__((noinline)) void matrix_3x3_multiplication( long input1[3][3] , long input2[3][3] , long output[3][3] )
{
	output[0][0] = input1[0][0] + input2[0][0];
}