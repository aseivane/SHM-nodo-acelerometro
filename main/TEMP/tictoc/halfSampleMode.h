#ifndef HSM_H
#define HSM_H

#include <stdint.h>

/*
* Library that calculates the HSM mode of the given array
*/

struct HalfSampleModeResult {
	int64_t mode; // the value of the hsm mode
	
	/* 
	* The value of the mode is located between position1 and position2
	* If the position2 == -1 the mode is the value present in position1
	*/
	int position1; 
	int position2; 
};

typedef struct HalfSampleModeResult HalfSampleModeResult;

/*
* Calculates the HSM mode of the given array
*	array -> the array whose HSM mode will be calculated
*	start -> first position that will be taken into account for the calculation
*	end -> last position that will be taken into account for the calculation (non-inclusive)
*	fx -> function 'fx(array, pos)' capable of returning the value of position from the given array
* 	result -> struct where the result of the operation is stored.
*/
void halfSampleMode(void* array, int start, int end, int64_t(*fx)(void*, int), HalfSampleModeResult* result);


/*
* Calculates the HSM mode of the given array, but stops when it narrows the mode to a window of an specific size.
* Note: the mode value won't be set, just the positions of the window in the array.
*	array -> the array whose HSM mode will be calculated
*	start -> first position that will be taken into account for the calculation
*	end -> last position that will be taken into account for the calculation (non-inclusive)
*	fx -> function 'fx(array, pos)' capable of returning the value of position from the given array
*   windowSize -> the algorithm will stop iterations when it get to a window of this size or a smaller one.
* 	result -> struct where the result of the operation is stored.
*/
void halfSampleModeWindow(void* array, int start, int end, int64_t(*fx)(void*, int), int windowSize, HalfSampleModeResult* result);



#endif