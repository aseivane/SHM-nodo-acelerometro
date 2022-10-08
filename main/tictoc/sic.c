#include "sic.h"
#include "halfSampleMode.h"
#include <stdio.h>
#include <inttypes.h>	

//#define TICTOC_SIC_DEBUG

void sicReset(SicData* sic);
void updateSamples(SicData* sic, int64_t t1, int64_t t2, int64_t t3, int64_t t4);
void calculateLinearFit(SicData* sic);

/***********************************
* 			WmNode Definitions
*
* This datatype represents the data corresponding to a single estimation of phi.
***********************************/
// TODO extract this to its own module

struct WmNode { 
	int64_t cmp; // This value will be used to order the array
	int64_t phi; 
    int64_t time;
};

typedef struct WmNode WmNode;


// function to copy a WmNode from source to target.
void cpyWmNode(void * source, void * target){
	WmNode* sourceWmNode = (WmNode*) source;
	WmNode* targetWmNode = (WmNode*) target;

	targetWmNode->cmp = sourceWmNode->cmp;
	targetWmNode->phi = sourceWmNode->phi;
	targetWmNode->time = sourceWmNode->time;
}

// function to order two WmNode
double cmpWmNode(void * first, void * second){
	WmNode* firstWmNode = (WmNode*) first;
	WmNode* secondWmNode = (WmNode*) second;
	return (double)firstWmNode->cmp - (double)secondWmNode->cmp;
}

//function to retrieve the value to use for the comparison in the position pos of the array.
int64_t getCmp(void * array, int pos){
	return ((WmNode*)((CircularOrderedArray*) array)->data[pos])->cmp;
}

int64_t getPhi(void * array, int pos){
	return ((WmNode*)((CircularOrderedArray*) array)->data[pos])->phi;
}

double getPhiDouble(void * array, int pos){
	return getPhi(array, pos);
}

int64_t getTime(void * array, int pos){
	return ((WmNode*)((CircularOrderedArray*) array)->data[pos])->time;
}

double getTimeDouble(void * array, int pos){
	return getTime(array, pos);
}

//----------------------------------


void sicInit(SicData* sic) {
	sic->Wm = initCircularOrderedArray(SAMPLES_SIZE, sizeof(WmNode), cpyWmNode, cmpWmNode);
	sic->Wmode = initCircularOrderedArray(MODE_WINDOW * MODE_SAMPLES, sizeof(WmNode), cpyWmNode, cmpWmNode);
	sicReset(sic);
	sic->state = NO_SYNC;
    sic->actual_m = 0;
    sic->actual_c = 0;	
}

// reset internal information without discarding current estimation paramenters.
void sicReset(SicData* sic){
	sic->syncSteps = 0;
	resetCircularOrderedArray(sic->Wm);
	resetCircularOrderedArray(sic->Wmode);
}

void sicEnd(SicData* sic) {
	freeCircularOrderedArray(sic->Wm);
	freeCircularOrderedArray(sic->Wmode);
}

void sicStepTimeout(SicData* sic){
	sic->to++;
	//If the amount of timeouted requests reached the threshold we restart the algorithm.
	if(sic->to == MAX_to){
		if(sic->state == NO_SYNC) { 
			sicReset(sic);
			sic->state = NO_SYNC;
			#ifdef TICTOC_SIC_DEBUG
			printf("SIC - Restarting NO_SYNC state.\n");
			#endif
		} else { 
			//If we have already an actual_m and actual_c we want to keep them
			sicReset(sic);
			sic->state = RE_SYNC;
			#ifdef TICTOC_SIC_DEBUG
			printf("SIC - Entered RE_SYNC state.\n");
			#endif
		}
	}
}

void sicStep(SicData* sic, int64_t t1, int64_t t2, int64_t t3, int64_t t4) {
	sic->to=0; // Successfully received data -> reset timeout counter
	sic->syncSteps++; // Increase loop counter


	updateSamples(sic, t1, t2, t3, t4);

	// If we filled the sample array for the first time or one minute has passed since the last estimation we get a new estimation of phi
	if(((sic->state == NO_SYNC || sic->state == RE_SYNC) && sic->syncSteps == STARTUP_CYCLES) ||
		(sic->state == SYNC && sic->syncSteps == P)){
		
		calculateLinearFit(sic);
		sic->state = SYNC;
		sic->syncSteps = 0;
		#ifdef TICTOC_SIC_DEBUG
		printf("SIC - SYNC state: new m: %f c: %f.\n", sic->actual_m, sic->actual_c);
		#endif
	} 
}


// Calculates phi and inserts it in the corresponding arrays
void updateSamples(SicData* sic, int64_t t1, int64_t t2, int64_t t3, int64_t t4){	
	WmNode node;
	
	node.phi = (t1 - t2 - N * t3 + N * t4)/(N+1);
	node.cmp = node.phi;
	node.time = (t1+t4)/2;
		
	// Insert the latest sample
	insertOrdered(sic->Wm, &node);

	// Each minute we get the mode values from the samples array and insert them mode array
	HalfSampleModeResult hsmResult;
	if(sic->syncSteps % MODE_CYCLES == 0){
		halfSampleModeWindow(sic->Wm, 0, sic->Wm->size, getCmp, MODE_WINDOW, &hsmResult);

		for(int i=hsmResult.position1; i<hsmResult.position2; i++){
			WmNode node;	
			node.phi = getPhi(sic->Wm, i);
			node.cmp = getCmp(sic->Wm, i);
			node.time = getTime(sic->Wm, i);
			insertOrdered(sic->Wmode, &node);
		}
	}	
}

// Combines the old and the new value using the given alpha factor.
double ponderate(double previousValue, double newValue) {
	return ALPHA * previousValue + (1-ALPHA) * newValue;
}

// Uses the current data to estimate the phi parameters.
void calculateLinearFit(SicData* sic){
	LinearFitResult result;
	linearFit(sic->Wmode, 0, sic->Wmode->size, getTimeDouble, getPhiDouble, &result); 

	if(sic->state == SYNC){
		sic->actual_m = ponderate(sic->actual_m, result.m);
		sic->actual_c = ponderate(sic->actual_c, result.c);	
	} else {
		sic->actual_m = result.m;
		sic->actual_c = result.c;	
	}
	
}

int sicTimeAvailable(SicData* sic){
	return sic->state > NO_SYNC;
}

int64_t computePhi(SicData* sic, int64_t systemClock){
	return (int64_t)(systemClock*sic->actual_m + sic->actual_c);
}

int64_t sicTime(SicData* sic, int64_t systemClock){
	return systemClock - computePhi(sic, systemClock);
}
