#ifndef LINEAR_FIT_H
#define LINEAR_FIT_H

#include <stdint.h>

/*
* Library to estimate the linear regresion of an array of points.
*/

// y = m*x + c
struct LinearFitResult {
	double m; 
	double c; 
};

typedef struct LinearFitResult LinearFitResult;


/*
* Calculates the linear interpolation of the given array
*	array -> the array whose HSM mode will be calculated
*	start -> first position that will be taken into account for the calculation
*	end -> last position that will be taken into account for the calculation (non-inclusive)
*	fx -> function 'fx(array, pos)' capable of returning the x coordinate of position pos from the given array
*	fy -> function 'fy(array, pos)' capable of returning the y coordinate of position pos from the given array
* 	result -> struct where the result of the operation is stored.
*/
// end: last position of the array to take into account (not included)
void linearFit(void* array, int start, int end, double(*fx)(void*, int), double(*fy)(void*, int), LinearFitResult* result);

#endif