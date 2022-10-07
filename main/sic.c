/*
 * sic.c
 *
 *  Created on: May 24, 2020
 *      Author: jaatadia@gmail.com
 */
#include "sic.h"

void sicReset(SicData* sic){
	sic->syncSteps = 0;
	
	initCircularOrderedArray(&sic->Wm);
	initCircularLinearFitArray(&sic->Wmedian);
}


void sicInit(SicData* sic) {
	sicReset(sic);
	sic->state = NO_SYNC;
    sic->actual_m = 0;
    sic->actual_c = 0;	
}

void sicStepTimeout(SicData* sic){
	sic->to++;
	if(sic->to == MAX_to){
		if(sic->state == NO_SYNC) { 
			sicInit(sic);
		} else { //if we have already an actual_m and actual_c we want to keep them
			sicReset(sic);
			sic->state = RE_SYNC;
		}
	}
}

void sicStep(SicData* sic, long long t1, long long t2, long long t3, long long t4) {
	sic->to=0;
	insertOrdered(&sic->Wm, t1 - t2 + (t2 - t1 + t4 - t3) / 2.0); // Wm <- t1 - t2 + (t2 - t1 + t4 - t3) / 2.0 
	insertPoint(&sic->Wmedian, t1, median(&sic->Wm)); // Wmedian <- (t1, median(Wm))
	sic->syncSteps++;

	if ((sic->state == PRE_SYNC || sic->state == SYNC) && sic->syncSteps == P) {
		linearFit(&sic->Wmedian);
		sic->state = SYNC;
		sic->syncSteps = 0;
		sic->actual_m = (1 - ALPHA) * sic->Wmedian.m + ALPHA * sic->actual_m;
		sic->actual_c = (1 - ALPHA) * sic->Wmedian.c + ALPHA * sic->actual_c;	
	} else if((sic->state == NO_SYNC || sic->state == RE_SYNC) && sic->syncSteps == P + MEDIAN_MAX_SIZE) {
		linearFit(&sic->Wmedian);
		sic->state = PRE_SYNC;
		sic->syncSteps = 0;
		sic->actual_m = sic->Wmedian.m;
		sic->actual_c = sic->Wmedian.c;
	} 
}

int sicTimeAvailable(SicData* sic){
	return sic->state > NO_SYNC;
}

long long sicTime(SicData* sic, long long systemClock){
	return systemClock - (systemClock*sic->actual_m + sic->actual_c);
}
