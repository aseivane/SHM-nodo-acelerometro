#include "linearfit.h"
#include <stdio.h>
#include <inttypes.h>

//#define TICTOC_LINEAR_FIT_DEBUG


#include <stdio.h>
void linearFit(void* array, int start, int end, double(*fx)(void*, int), double(*fy)(void*, int), LinearFitResult* result) {
	double n = end - start;
	double Sx = 0;
	double Sy = 0;
	double stt = 0;
	double sts = 0;

	for (int i = start; i < end; ++i)
	{
		Sx += (*fx)(array, i);
		Sy += (*fy)(array, i);
	}
	for (int i = start; i < end; ++i)
	{
		double t = (*fx)(array, i) - Sx/n;
		stt += t*t;
		sts += t*(*fy)(array, i);
	}
	double slope = sts/stt;
	double intercept = (Sy - Sx*slope)/n;


	result->m = slope;
	result->c = intercept;

	#ifdef TICTOC_LINEAR_FIT_DEBUG
	printf("Linearfit - New linear fit m: %f c: %f.\n", result->m, result->c);
	#endif	
}	


/*
	https://www.johndcook.com/blog/2008/10/20/comparing-two-ways-to-fit-a-line-to-data/
	
	double sx = 0.0, sy = 0.0, stt = 0.0, sts = 0.0;
	int n = x.size();
	for (int i = 0; i < n; ++i)
	{
		sx += x[i];
		sy += y[i];
	}
	for (int i = 0; i < n; ++i)
	{
		double t = x[i] - sx/n;
		stt += t*t;
		sts += t*y[i];
	}
	double slope = sts/stt;
	double intercept = (sy - sx*slope)/n;
*/