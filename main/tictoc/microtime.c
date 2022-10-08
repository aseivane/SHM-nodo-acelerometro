#include "microtime.h"
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>

int64_t epochInMicros(){
	struct timeval currentTime;
	gettimeofday(&currentTime, 0);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;	
}

void encodeEpochInMicros(int64_t timestamp, int32_t* response, int pos){
	int32_t seconds = timestamp / 1000000;
	int32_t micros = timestamp % 1000000;

	response[pos] = htonl(seconds);
	response[pos+1] = htonl(micros);
}

int64_t decodeEpochInMicros(int32_t* response, int pos){
	return ((int64_t)ntohl(response[pos])) * 1000000 + ntohl(response[pos+1]);
}

void microsToTimestamp(int64_t micros, char * timestamp){
		time_t now;
	    struct tm ts;
	    now=micros/1000000L;
	    ts = *localtime(&now);
	    strftime(timestamp, 64, "%Y-%m-%d %H:%M:%S %Z", &ts);
}