#include "halfSampleMode.h"

#include <stdio.h>
#include <inttypes.h>

//#define HSM_DEBUG
//#define HSM_WINDOW_DEBUG

int halfSampleStep(int N, void* array, int start, int end, int64_t(*fx)(void*, int)){
	#ifdef HSM_DEBUG
	printf("HalfSampleMode - start: %d, end: %d, N: %d.\n", start, end, N);
	#endif	
	
	int j = -1;
	int64_t wmin = (*fx)(array, end-1) - (*fx)(array, start);

	for(int i = start; i < end - (N-1); i++) {

		int64_t w = (*fx)(array, i + (N-1)) - (*fx)(array, i);
		if(w <= wmin) {
			wmin = w;
			j = i;
		}

		#ifdef HSM_DEBUG
		int64_t xi = (*fx)(array, i);
		int64_t xin = (*fx)(array, i + (N-1));
		printf("HalfSampleMode - i: %d, Xi: %ld, XiN-1: %ld, w: %ld.\n", i, xi, xin, w);
		#endif	
	}
	return j;
}


void halfSampleMode(void* array, int start, int end, int64_t(*fx)(void*, int), HalfSampleModeResult* result){
	int size = end - start;
	if(size == 1) {
		result->mode = (*fx)(array, start);
		result->position1=start;
		result->position2=-1;
	} else if(size == 2) {
		result->mode = ((*fx)(array, start) + (*fx)(array, start+1))/2;
		result->position1=start;
		result->position2=start+1;
	} else if(size == 3) {
		int64_t x1 = (*fx)(array, start);
		int64_t x2 = (*fx)(array, start + 1);
		int64_t x3 = (*fx)(array, start + 2);

		if(x2 - x1 == x3 - x2) {
			result->mode = x2;
			result->position1=start+1;
			result->position2=-1;
		} else if (x2 - x1 < x3 - x2) {
			result->mode = (x1 + x2)/2;
			result->position1=start;
			result->position2=start+1;
		} else {
			result->mode = (x2 + x3)/2;	
			result->position1=start+1;
			result->position2=start+2;
		} 
	} else {
		int N = (size % 2 == 0) ? (size/2) : (size/2 + 1);
		int j = halfSampleStep(N, array, start, end, fx);

		#ifdef HSM_DEBUG
		printf("HalfSampleMode - j: %d N: %d.\n", j, N);
		#endif	

		halfSampleMode(array, j, j + N, fx, result);
	}
}

void halfSampleModeWindow(void* array, int start, int end, int64_t(*fx)(void*, int), int windowSize, HalfSampleModeResult* result){
	int size = end - start;
	if(size <= windowSize) {
		result->position1=start;
		result->position2=end;
	} else {
		int N = (size % 2 == 0) ? (size/2) : (size/2 + 1);
		int j = halfSampleStep(N, array, start, end, fx);

		#ifdef HSM_DEBUG
		printf("HalfSampleMode - j: %d N: %d.\n", j, N);
		#endif	

		halfSampleModeWindow(array, j, j + N, fx, windowSize, result);
	}
}


