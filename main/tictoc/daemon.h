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

#include <sys/socket.h>
#include <netinet/in.h>
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"

#define TIC_TOC_DAEMON_PRIORITY 1
#define TIC_TOC_PERIOD 1000

struct BufferData {
	struct sockaddr_in cliaddr;
	int32_t timestamps[6];
};

typedef struct BufferData BufferData;

struct TicTocData {
	int sock;
	const char * serverIp;
	int serverPort;
	SicData sicdata;
	RingbufHandle_t buf_handle;
	int64_t* timeRequest;
	int socketfd;
};

typedef struct TicTocData TicTocData;

void setupTicToc(TicTocData *, const char * serverIp, int serverPort);

int ticTocReady(TicTocData *);

int64_t ticTocTime(TicTocData *);

#endif /* MAIN_TIC_TOC_H_ */
