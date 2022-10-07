/*
 * sic.h
 *
 *  Created on: May 24, 2020
 *      Author: jaatadia@gmail.com
 */
#ifndef SIC_H
#define SIC_H

#define MEDIAN_MAX_SIZE 600
#define P 60
#define ALPHA 0.05
#define TIMEOUT 0.8
#define errRTT 0.8
#define MAX_to 6

#define NO_SYNC 0
#define RE_SYNC 1
#define PRE_SYNC 2
#define SYNC 3

#include "linearfit.h"
#include "circularOrderedArray.h"

struct SicData { 
	int state;
	int syncSteps;
    int to;

    CircularOrderedArray Wm;
    CircularLinearFitArray Wmedian;

    double actual_m;
    double actual_c;
};

typedef struct SicData SicData;

void sicInit(SicData* sic);
void sicStepTimeout(SicData* sic);
void sicStep(SicData* sic, long long t1, long long t2, long long t3, long long t4);

int sicTimeAvailable(SicData* sic);
long long sicTime(SicData* sic, long long systemClock);

#endif
