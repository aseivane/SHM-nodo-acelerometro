#ifndef MICRO_TIME_H
#define MICRO_TIME_H

#include <stdint.h> 

// Returns the current microseconds since epoch
int64_t epochInMicros();

//Writes the timestamp in a networksafeway using two positions of the response array starting in pos.
void encodeEpochInMicros(int64_t timestamp, int32_t* response, int pos);

//Obtains the timestampe from the position pos of the response array encoded using encodeEpochInMicros.
int64_t decodeEpochInMicros(int32_t* response, int pos);

//Writes the timestamp in human readable way to the buffer
void microsToTimestamp(int64_t micros, char* stringBuffer);

#endif