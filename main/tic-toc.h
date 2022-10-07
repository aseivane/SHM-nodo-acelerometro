/*
 * tic-toc-sender.h
 *
 *  Created on: May 9, 2020
 *      Author: jaatadia@gmail.com
 */

#ifndef MAIN_TIC_TOC_H_
#define MAIN_TIC_TOC_H_
#include <stdint.h>
#include "sic.h"

#define TIC_TOC_PERIOD 1000

struct TicTocData {
	int sock;
	const char * serverIp;
	int serverPort;
	SicData sicdata;
};

typedef struct TicTocData TicTocData;

void setupTicToc(TicTocData *, const char * serverIp, int serverPort);

int ticTocReady(TicTocData *);

int64_t ticTocTime(TicTocData *);

#endif /* MAIN_TIC_TOC_H_ */
