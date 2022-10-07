/*
 * circularOrderedArray.h
 *
 *  Created on: May 24, 2020
 *      Author: jaatadia@gmail.com
 */
#ifndef CIRCULAR_ORDERED_ARRAY_H
#define CIRCULAR_ORDERED_ARRAY_H

#ifndef CIRCULAR_ORDERED_ARRAY_MAX_SIZE
#define CIRCULAR_ORDERED_ARRAY_MAX_SIZE 600
#endif

struct CircularOrderedArray { 
	int next;
	int size;
	double fifo[CIRCULAR_ORDERED_ARRAY_MAX_SIZE];
	double array[CIRCULAR_ORDERED_ARRAY_MAX_SIZE];
};

typedef struct CircularOrderedArray CircularOrderedArray;

void initCircularOrderedArray(CircularOrderedArray *);
void insertOrdered(CircularOrderedArray *, double );
double median(CircularOrderedArray *);
#endif
