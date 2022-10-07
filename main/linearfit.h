/*
 * linearfit.h
 *
 *  Created on: May 24, 2020
 *      Author: jaatadia@gmail.com
 */
#ifndef LINEAR_FIT_H
#define LINEAR_FIT_H

#ifndef CICRULAR_LINEAR_FIT_ARRAY_MAX_SIZE
#define CICRULAR_LINEAR_FIT_ARRAY_MAX_SIZE 600
#endif

struct Point {
	double x;
	double y;
};

typedef struct Point Point;

struct CircularLinearFitArray { 
	int size;
	int nextPos;
	Point array[CICRULAR_LINEAR_FIT_ARRAY_MAX_SIZE];

	double Sx; 
	double Sy;
	double Sxx;
	double Sxy;

	double m;
	double c;
};

typedef struct CircularLinearFitArray CircularLinearFitArray;

void initCircularLinearFitArray(CircularLinearFitArray* response);

void insertPoint(CircularLinearFitArray* response, double median, double t1);

void linearFit(CircularLinearFitArray* response);

#endif
