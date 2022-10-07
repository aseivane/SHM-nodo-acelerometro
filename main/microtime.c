/*
 * microtime.c
 *
 *  Created on: May 9, 2020
 *      Author: jaatadia@gmail.com
 */
#include "microtime.h"
#include <time.h>

#ifndef NULL
#define NULL 0
#endif

void epoch(struct timeval * time){
	gettimeofday(time, NULL);
}

int64_t evalToMicros(struct timeval * time){
	return toMicros((int32_t)time->tv_sec, (int32_t)time->tv_usec);
}

int64_t toMicros(int32_t sec, int32_t usec){
	return (int64_t)sec * 1000000L + (int64_t)usec;
}

int64_t epochInMicros(){
	struct timeval currentTime;
	epoch(&currentTime);
	return evalToMicros(&currentTime);
}

void microsToTimestamp(int64_t micros, char * timestamp){
		time_t now;
	    struct tm ts;
	    now=micros/1000000L;
	    ts = *localtime(&now);
	    strftime(timestamp, 64, "%Y-%m-%d %H:%M:%S %Z", &ts);
}
