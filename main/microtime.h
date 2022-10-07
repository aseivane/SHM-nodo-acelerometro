/*
 * microtime.h
 *
 *  Created on: May 9, 2020
 *      Author: jaatadia@gmail.com
 */

#ifndef MAIN_MICROTIME_H_
#define MAIN_MICROTIME_H_

#include <stdint.h>
#include <sys/time.h>

void epoch(struct timeval * time);

int64_t toMicros(int32_t sec, int32_t usec);
int64_t evalToMicros(struct timeval * time);

int64_t epochInMicros();

void microsToTimestamp(int64_t micros, char* stringBuffer);


#endif /* MAIN_MICROTIME_H_ */
